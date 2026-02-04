cmake_minimum_required(VERSION 3.15)
project(HarmonyMicrokernel VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Microkernel library
add_library(harmony_kernel STATIC
    microkernel/kernel.cpp
    microkernel/ipc.cpp
)

# Test executable
add_executable(test_kernel
    tests/test_kernel.cpp
)

target_link_libraries(test_kernel harmony_kernel pthread)

# Brain executable (existing)
add_executable(brain
    brain.cpp
)

# Install targets
install(TARGETS harmony_kernel brain test_kernel
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Print build info
message(STATUS "Building Harmony Microkernel v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
