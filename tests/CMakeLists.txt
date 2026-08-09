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
    #sanity_test.cpp
    task_queue_test.cpp
)

target_link_libraries(acpf_tests
    PRIVATE
        acpf
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(acpf_tests)
