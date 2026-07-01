cmake_minimum_required(VERSION 3.20)
project(nasdaq_replayer)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================
#          Compiler flags
# ================================

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra -Wpedantic")

# ================================
#     Include google benchmark
# ================================

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
)
FetchContent_MakeAvailable(googlebenchmark)

# ================================
#              Engine
# ================================

add_library(engine STATIC
        Engine/EngineStructures.hpp
        Engine/OrderBook.hpp
        Engine/OrderBook.cpp
        Utils/MemoryPool.hpp
)

target_include_directories(engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

# ================================
#              Parser
# ================================

add_library(parser STATIC
        Parser/MessageType.hpp
        Parser/NasdaqStructures.hpp
        Parser/TimeParser.cpp
        Parser/TimeParser.hpp
)

target_include_directories(parser PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Parser
)

# ================================
#             Replayer
# ================================

add_library(replayer STATIC
        src/Replayer/NasdaqReplayer.cpp
        src/Replayer/NasdaqReplayer.hpp
        src/Replayer/Snapshot.hpp
)

target_include_directories(replayer PUBLIC
        src/Replayer
)

target_link_libraries(replayer PUBLIC engine parser)

# ================================
#           Executables
# ================================

# 1. Run replay (an ITCH file needed)
add_executable(nasdaq_replayer
        day_replay.cpp
)

target_link_libraries(nasdaq_replayer PRIVATE
        replayer
)

# 2. Run engine benchmark (random data)

add_executable(engine_benchmark
        Benchmarks/engine_benchmark.cpp
)

target_link_libraries(engine_benchmark PRIVATE
        engine
        benchmark::benchmark
)