INCLUDE_DIRECTORIES("${GTEST_INCLUDES}")

INCLUDE(GoogleTest)

ADD_EXECUTABLE(dpf_tests 
    "dpf_tests.cpp"
)

TARGET_COMPILE_DEFINITIONS(dpf_tests PRIVATE "BASE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
TARGET_LINK_LIBRARIES(dpf_tests PRIVATE libdpf)
TARGET_LINK_LIBRARIES(dpf_tests PRIVATE GTest::gtest_main)

gtest_discover_tests(dpf_tests)
