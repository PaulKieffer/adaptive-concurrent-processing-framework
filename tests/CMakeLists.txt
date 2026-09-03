include(FetchContent)

# Forces GoogleTest to link against the shared C runtime library on Windows
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(acpf_tests
    task_queue_test.cpp
    thread_pool_test.cpp
    controller_test.cpp
    moving_average_test.cpp
)

target_link_libraries(acpf_tests
    PRIVATE
        acpf
        GTest::gtest_main
)

if(ACPF_ENABLE_ASAN)
    enable_sanitizer(acpf_tests address)
endif()

if(ACPF_ENABLE_TSAN)
    enable_sanitizer(acpf_tests thread)
endif()

include(GoogleTest)

# ThreadSanitizer may fail to start on some Linux systems with:
#
#   FATAL: ThreadSanitizer: unexpected memory mapping
#
# This is caused by an interaction between ThreadSanitizer's fixed shadow
# memory mappings and Linux ASLR (Address Space Layout Randomization). The
# problem is environmental and can occur before any test code is executed;
# it does not indicate a data race or other error in the tested code.
#
# On affected systems, disabling ASLR for the test process via
# `setarch <arch> -R` provides a reliable workaround. We deliberately apply
# this only to the TSAN test executable instead of disabling ASLR globally,
# so normal builds and test configurations remain unaffected.
#
# `CROSSCOMPILING_EMULATOR` is used here because CMake/CTest also executes
# the test binary during gtest_discover_tests(). By assigning setarch as
# the emulator, both test discovery and normal CTest execution use the same
# TSAN-compatible invocation.
#
# This workaround is only relevant on UNIX platforms and only when TSAN is
# enabled.
if(ACPF_ENABLE_TSAN AND UNIX)
    find_program(SETARCH_EXECUTABLE setarch REQUIRED)

    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE HOST_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set_property(TARGET acpf_tests PROPERTY
        CROSSCOMPILING_EMULATOR
        "${SETARCH_EXECUTABLE};${HOST_ARCH};-R"
    )
endif()

gtest_add_tests(
    TARGET acpf_tests
)