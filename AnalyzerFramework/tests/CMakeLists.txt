find_package(GTest CONFIG REQUIRED)

add_executable(sknano_framework_tests
    SelectTauIndicesTest.cc
    OverlapRemovalTest.cc)

target_include_directories(sknano_framework_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/DataFormats/tests)

target_link_libraries(sknano_framework_tests PRIVATE
    AnalyzerFramework
    GTest::gtest
    GTest::gtest_main)

add_test(NAME framework_unit COMMAND sknano_framework_tests)
