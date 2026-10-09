@echo off
setlocal

:: Check if two commit hashes are provided as arguments
if "%~2"=="" goto :usage

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

set "COMMIT_BASE=%~1"
set "COMMIT_NEW=%~2"

:: --- CONFIGURATION ---
set "BINARY_NAME=slychess.exe"
set "BUILD_BASE_DIR=../sprt_slychess/build_base"
set "BUILD_NEW_DIR=../sprt_slychess/build_new"

:: Save the current git state (branch or detached HEAD) to restore later
set "CURRENT_STATE="
for /f "delims=" %%i in ('git symbolic-ref --short HEAD 2^>nul') do set "CURRENT_STATE=%%i"
if not defined CURRENT_STATE (
    for /f "delims=" %%i in ('git rev-parse HEAD') do set "CURRENT_STATE=%%i"
)

:: Check if there are uncommitted changes to stash
set "STASHED=0"
for /f "delims=" %%i in ('git status --porcelain') do (
    set "STASHED=1"
    goto :check_stash_done
)
:check_stash_done

if "%STASHED%"=="1" (
    echo === Stashing any uncommitted local changes...
    git stash
) else (
    echo === No local changes to stash.
)

:: --- 1. BUILD BASE ENGINE ---
echo === Checking out base commit: %COMMIT_BASE%
git checkout "%COMMIT_BASE%"
if errorlevel 1 goto :error

echo === Configuring and compiling base engine...
if not exist "%BUILD_BASE_DIR%" mkdir "%BUILD_BASE_DIR%"
cmake -S . -B "%BUILD_BASE_DIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error
cmake --build "%BUILD_BASE_DIR%" --config Release -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 goto :error

:: --- 2. BUILD NEW ENGINE ---
echo === Checking out new commit: %COMMIT_NEW%
git checkout "%COMMIT_NEW%"
if errorlevel 1 goto :error

echo === Configuring and compiling new engine...
if not exist "%BUILD_NEW_DIR%" mkdir "%BUILD_NEW_DIR%"
cmake -S . -B "%BUILD_NEW_DIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error
cmake --build "%BUILD_NEW_DIR%" --config Release -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 goto :error

:: --- 3. RESTORE REPO STATE ---
echo === Restoring original git state (%CURRENT_STATE%)...
git checkout "%CURRENT_STATE%"
if "%STASHED%"=="1" (
    echo === Restoring stashed changes...
    git stash pop >nul 2>&1
    if errorlevel 1 (
        echo Note: Conflict encountered while popping stash.
    )
)

:: --- 4. RUN SPRT TEST ---
echo === Starting fastchess SPRT test...
fastchess ^
    -engine cmd="./%BUILD_NEW_DIR%/%BINARY_NAME%" name="New-%COMMIT_NEW%" ^
    -engine cmd="./%BUILD_BASE_DIR%/%BINARY_NAME%" name="Base-%COMMIT_BASE%" ^
    -pgnout file="../sprt_slychess/sprt_results.pgn" ^
    -openings file="..\sprt_slychess\books\8moves_v3.pgn" format=pgn order=random ^
    -each tc=5.0+0.1 -repeat -concurrency 6 -recover ^
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -rounds 10000 ^
    -autosaveinterval 0

echo === SPRT test completed! Results saved to sprt_results.pgn
endlocal
exit /b 0

:usage
echo Usage: %~nx0 ^<commit_hash_base^> ^<commit_hash_new^>
endlocal
exit /b 1

:error
echo Error encountered. Aborting script.
:: Attempt to restore original state on failure if possible
if defined CURRENT_STATE git checkout "%CURRENT_STATE%" >nul 2>&1
if "%STASHED%"=="1" git stash pop >nul 2>&1
endlocal
exit /b 1