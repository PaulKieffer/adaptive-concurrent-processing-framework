cmake_minimum_required(VERSION 3.20)

project(
    AdaptiveConcurrentProcessingFramework
    VERSION 0.1.0
    LANGUAGES CXX
)

option(ACPF_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ACPF_ENABLE_TSAN "Enable ThreadSanitizer" OFF)

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
add_executable(acpf
    src/main.cpp
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
