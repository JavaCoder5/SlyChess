# CMakeList.txt : CMake project for SlyChess, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SlyChess")

# Add source to this project's executable.
add_executable (SlyChess "src/SlyChess/SlyChess.cpp"  "src/Constants/Constants.h" "src/Movegen/initKnightAttacks.h" "src/Movegen/initKnightAttacks.cpp" "src/Movegen/initKingAttacks.h" "src/Movegen/initKingAttacks.cpp" "src/Movegen/initPawnAttacks.h" "src/Movegen/initPawnAttacks.cpp" "src/Constants/Macros.h" "src/Movegen/MagicBoards/lookupAttacks.h" "src/Movegen/MagicBoards/initSliders.h" "src/Movegen/MagicBoards/initSliders.cpp" "src/Movegen/initPawnCaptures.h" "src/Movegen/initPawnCaptures.cpp" "src/Movegen/generatePseudoLegalMoves.h" "src/Movegen/generatePseudoLegalMoves.cpp" "src/Movegen/Board/makeMove.h" "src/Movegen/Board/makeMove.cpp" "src/Movegen/generateLegalMoves.h" "src/Movegen/generateLegalMoves.cpp" "src/Movegen/Board/unmakeMove.h" "src/Movegen/Board/unmakeMove.cpp" "src/Movegen/Perft/perft.h" "src/Movegen/Perft/perft.cpp" "src/uci/uciToMove.h" "src/uci/uciToMove.cpp" "src/uci/moveToUci.h" "src/uci/moveToUci.cpp" "src/Search/search.h" "src/Search/search.cpp" "src/Search/alphaBeta.h" "src/Search/alphaBeta.cpp" "src/Evaluation/evaluate.h" "src/Evaluation/evaluate.cpp" "src/Search/sortLegalMoves.h" "src/Search/sortLegalMoves.cpp" "src/Search/quiescence.h" "src/Search/quiescence.cpp" "src/Movegen/generatePseudoLegalCaptures.h" "src/Movegen/generatePseudoLegalCaptures.cpp" "src/Movegen/generateLegalCaptures.h" "src/Movegen/generateLegalCaptures.cpp")

target_include_directories(SlyChess PRIVATE ${PROJECT_SOURCE_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SlyChess PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
