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
#             Core
# ================================

add_library(core STATIC
        src/Engine/OrderBook.cpp
        src/Replayer/NasdaqReplayer.cpp

        include/Engine/EngineStructures.hpp
        include/Engine/OrderBook.hpp
        include/Parser/MessageType.hpp
        include/Parser/NasdaqStructures.hpp
        include/Parser/TimeParser.hpp
        include/Replayer/NasdaqReplayer.hpp
        include/Utils/MemoryPool.hpp
)

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ================================
#           Executables
# ================================

# 1. Run replay (an ITCH file needed)
add_executable(nasdaq_replayer
        apps/day_replay.cpp
)

target_link_libraries(nasdaq_replayer PRIVATE
        core
)

# 2. Run engine benchmark (random data)

add_executable(engine_benchmark
        Benchmarks/engine_benchmark.cpp
)

target_link_libraries(engine_benchmark PRIVATE
        core
        benchmark::benchmark
)