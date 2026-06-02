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
add_executable (SlyChess "src/SlyChess/SlyChess.cpp"  "src/Constants/Constants.h" "src/Movegen/initKnightAttacks.h" "src/Movegen/initKnightAttacks.cpp" "src/Movegen/initKingAttacks.h" "src/Movegen/initKingAttacks.cpp")

target_include_directories(SlyChess PRIVATE ${PROJECT_SOURCE_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SlyChess PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
