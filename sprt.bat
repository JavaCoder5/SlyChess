@echo off
setlocal

:: Check if two commit hashes are provided as arguments
if "%~2"=="" goto :usage

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

set "COMMIT_BASE=%~1"
set "COMMIT_NEW=%~2"

:: --- CONFIGURATION ---
set "BINARY_NAME=slychess.exe"
set "WORKTREE_BASE_DIR=../sprt_slychess/worktree_base"
set "WORKTREE_NEW_DIR=../sprt_slychess/worktree_new"
set "BUILD_BASE_DIR=%WORKTREE_BASE_DIR%/build"
set "BUILD_NEW_DIR=%WORKTREE_NEW_DIR%/build"

:: Resolve the commits here, so e.g. HEAD means this checkout's HEAD, not the worktree's
set "SHA_BASE="
set "SHA_NEW="
for /f "delims=" %%i in ('git rev-parse --verify --short "%COMMIT_BASE%"') do set "SHA_BASE=%%i"
for /f "delims=" %%i in ('git rev-parse --verify --short "%COMMIT_NEW%"') do set "SHA_NEW=%%i"
if not defined SHA_BASE goto :error
if not defined SHA_NEW goto :error

:: --- 1. BUILD BASE ENGINE ---
echo === Checking out base commit %COMMIT_BASE% into %WORKTREE_BASE_DIR%
call :checkout_worktree "%WORKTREE_BASE_DIR%" "%SHA_BASE%"
if errorlevel 1 goto :error

echo === Configuring and compiling base engine...
if not exist "%BUILD_BASE_DIR%" mkdir "%BUILD_BASE_DIR%"
cmake -S "%WORKTREE_BASE_DIR%" -B "%BUILD_BASE_DIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error
cmake --build "%BUILD_BASE_DIR%" --config Release -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 goto :error

:: --- 2. BUILD NEW ENGINE ---
echo === Checking out new commit %COMMIT_NEW% into %WORKTREE_NEW_DIR%
call :checkout_worktree "%WORKTREE_NEW_DIR%" "%SHA_NEW%"
if errorlevel 1 goto :error

echo === Configuring and compiling new engine...
if not exist "%BUILD_NEW_DIR%" mkdir "%BUILD_NEW_DIR%"
cmake -S "%WORKTREE_NEW_DIR%" -B "%BUILD_NEW_DIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error
cmake --build "%BUILD_NEW_DIR%" --config Release -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 goto :error

:: --- 3. RUN SPRT TEST ---
echo === Starting fastchess SPRT test...
fastchess ^
    -engine cmd="./%BUILD_NEW_DIR%/%BINARY_NAME%" name="New-%COMMIT_NEW%" ^
    -engine cmd="./%BUILD_BASE_DIR%/%BINARY_NAME%" name="Base-%COMMIT_BASE%" ^
    -pgnout file="../sprt_slychess/sprt_%SHA_NEW%_vs_%SHA_BASE%.pgn" ^
    -openings file="..\sprt_slychess\books\8moves_v3.pgn" format=pgn order=random ^
    -each tc=5.0+0.1 -repeat -concurrency 6 -recover ^
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -rounds 10000 ^
    -autosaveinterval 0

echo === SPRT test completed! Results saved to sprt_%SHA_NEW%_vs_%SHA_BASE%.pgn
endlocal
exit /b 0

:: Check out a commit in its own worktree, so the main working copy is never touched.
:: The worktrees are reused between runs, which keeps rebuilds incremental.
:checkout_worktree
if exist "%~1/.git" (
    git -C "%~1" checkout --quiet --force --detach "%~2"
) else (
    git worktree prune
    git worktree add --detach "%~1" "%~2"
)
exit /b %errorlevel%

:usage
echo Usage: %~nx0 ^<commit_hash_base^> ^<commit_hash_new^>
endlocal
exit /b 1

:error
echo Error encountered. Aborting script.
endlocal
exit /b 1