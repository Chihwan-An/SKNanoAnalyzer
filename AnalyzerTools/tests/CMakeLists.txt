find_package(GTest CONFIG REQUIRED)

add_executable(sknano_tools_tests TauCorrectionTest.cc)

target_link_libraries(sknano_tools_tests PRIVATE
    AnalyzerTools
    GTest::gtest
    GTest::gtest_main)

add_test(NAME tools_unit COMMAND sknano_tools_tests)

set_tests_properties(tools_unit PROPERTIES
    ENVIRONMENT "JSONPOG_REPO_PATH=$ENV{JSONPOG_REPO_PATH};SKNANO_DATA=$ENV{SKNANO_DATA};ROCCOR_PATH=$ENV{ROCCOR_PATH}")
