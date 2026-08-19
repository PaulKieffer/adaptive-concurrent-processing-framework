cmake_minimum_required(VERSION 3.20)

project(
    AdaptiveConcurrentProcessingFramework
    VERSION 0.1.0
    LANGUAGES CXX
)

# Options
option(ACPF_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ACPF_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ACPF_BUILD_TESTS "Build unit tests" ON)
option(ACPF_BUILD_EXAMPLES "Build examples" ON)
option(ACPF_BUILD_BENCHMARKS "Build benchmarks" OFF)

# Prevent incompatible sanitizer combinations
if(ACPF_ENABLE_ASAN AND ACPF_ENABLE_TSAN)
    message(FATAL_ERROR
        "AddressSanitizer and ThreadSanitizer cannot be enabled together."
    )
endif()

# Sanitizer-Function
function(enable_sanitizer target sanitizer)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -fsanitize=${sanitizer}
            -fno-omit-frame-pointer
        )

        target_link_options(${target} PRIVATE
            -fsanitize=${sanitizer}
            -no-pie
        )
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Target
add_library(acpf
    src/task_queue.cpp
    src/thread_pool.cpp
)

target_include_directories(acpf
    PUBLIC
        include
)

target_compile_options(acpf PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Activate Sanitizer
if(ACPF_ENABLE_ASAN)
    enable_sanitizer(acpf address)
endif()

if(ACPF_ENABLE_TSAN)
    enable_sanitizer(acpf thread)
endif()

if(ACPF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ACPF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(ACPF_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
