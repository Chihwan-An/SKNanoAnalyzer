find_package(GTest CONFIG REQUIRED)

add_executable(sknano_dataformats_tests TauViewTest.cc)

target_include_directories(sknano_dataformats_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sknano_dataformats_tests PRIVATE
    DataFormats
    GTest::gtest
    GTest::gtest_main)

add_test(NAME dataformats_unit COMMAND sknano_dataformats_tests)
