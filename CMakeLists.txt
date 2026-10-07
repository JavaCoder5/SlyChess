# CMakeList.txt : CMake project for SlyChess, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

# Enforce C++20 globally and make it mandatory for all compilers (MSVC & GCC)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SlyChess")

# custom target to copy the weights folder to the build directory
add_custom_target(CopyWeights ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/nnue/weights/2026-10-06"
    $<TARGET_FILE_DIR:SlyChess>
    COMMENT "Copying NNUE weights to build directory..."
)

# Add source to this project's executable.
add_executable (SlyChess "src/SlyChess/SlyChess.cpp"  "src/Constants/Constants.h" "src/Movegen/initKnightAttacks.h" "src/Movegen/initKnightAttacks.cpp" "src/Movegen/initKingAttacks.h" "src/Movegen/initKingAttacks.cpp" "src/Movegen/initPawnAttacks.h" "src/Movegen/initPawnAttacks.cpp" "src/Constants/Macros.h" "src/Movegen/MagicBoards/lookupAttacks.h" "src/Movegen/MagicBoards/initSliders.h" "src/Movegen/MagicBoards/initSliders.cpp" "src/Movegen/initPawnCaptures.h" "src/Movegen/initPawnCaptures.cpp" "src/Movegen/generatePseudoLegalMoves.h" "src/Movegen/generatePseudoLegalMoves.cpp" "src/Movegen/Board/makeMove.h" "src/Movegen/Board/makeMove.cpp" "src/Movegen/generateLegalMoves.h" "src/Movegen/generateLegalMoves.cpp" "src/Movegen/Board/unmakeMove.h" "src/Movegen/Board/unmakeMove.cpp" "src/Movegen/Perft/perft.h" "src/Movegen/Perft/perft.cpp" "src/uci/uciToMove.h" "src/uci/uciToMove.cpp" "src/uci/moveToUci.h" "src/uci/moveToUci.cpp" "src/Search/search.h" "src/Search/search.cpp" "src/Search/alphaBeta.h" "src/Search/alphaBeta.cpp" "src/Evaluation/evaluate.h" "src/Evaluation/evaluate.cpp" "src/Search/sortLegalMoves.h" "src/Search/sortLegalMoves.cpp" "src/Search/quiescence.h" "src/Search/quiescence.cpp" "src/Movegen/generatePseudoLegalCaptures.h" "src/Movegen/generatePseudoLegalCaptures.cpp" "src/Movegen/generateLegalCaptures.h" "src/Movegen/generateLegalCaptures.cpp" "src/Search/movePVToFront.h" "src/Search/movePVToFront.cpp" "src/Search/stopAndJoinSearch.h" "src/Search/stopAndJoinSearch.cpp" "src/timeManagement/searchTime.h" "src/timeManagement/searchTime.cpp" "src/Helpers/wasCapture.h" "src/Helpers/wasPromotion.h" "src/Movegen/isSquareAttacked.h" "src/Movegen/isSquareAttacked.cpp" "src/misc/printMutex.h" "src/misc/printMutex.cpp" "src/Xorshift64/xorshift64.h" "src/zobrist/zobrist.h" "src/zobrist/zobrist.cpp" "src/zobrist/computeBaseHash.h" "src/zobrist/computeBaseHash.cpp" "src/transpositionTable/write_tt.h" "src/transpositionTable/write_tt.cpp" "src/transpositionTable/probe_tt.h" "src/transpositionTable/probe_tt.cpp" "src/repetitionTable/isRepetition.h" "src/repetitionTable/isRepetition.cpp" "src/repetitionTable/writeRepetitionTable.h" "src/repetitionTable/writeRepetitionTable.cpp" "src/NNUE/nnue.h" "src/NNUE/nnue.cpp")

target_include_directories(SlyChess PRIVATE ${PROJECT_SOURCE_DIR})

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR};${CMAKE_GENERATOR_PLATFORM}" SLYCHESS_TARGET_ARCH)
if (SLYCHESS_TARGET_ARCH MATCHES "x86_64|amd64|i[3-6]86|win32")
  if (MSVC)
    target_sources(SlyChess PRIVATE "src/NNUE/nnue_avx2.cpp")
    set_source_files_properties("src/NNUE/nnue_avx2.cpp" PROPERTIES COMPILE_OPTIONS "/arch:AVX2")
    target_compile_definitions(SlyChess PRIVATE SLYCHESS_HAS_AVX2=1)
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_sources(SlyChess PRIVATE "src/NNUE/nnue_avx2.cpp")
    set_source_files_properties("src/NNUE/nnue_avx2.cpp" PROPERTIES COMPILE_OPTIONS "-mavx2")
    target_compile_definitions(SlyChess PRIVATE SLYCHESS_HAS_AVX2=1)
  endif()
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SlyChess PROPERTY CXX_STANDARD 20)
endif()

add_custom_command(TARGET SlyChess POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/nnue/weights/2026-10-06/nnue_weights.bin"
    $<TARGET_FILE_DIR:SlyChess>
)

# TODO: Add tests and install targets if needed.
