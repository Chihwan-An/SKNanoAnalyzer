add_executable(test_view_and_branch_lifetime
    test_view_and_branch_lifetime.cc
)

target_include_directories(test_view_and_branch_lifetime PRIVATE
    ${CMAKE_SOURCE_DIR}/Analyzers/include
    ${CMAKE_SOURCE_DIR}/DataFormats/include
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)

target_compile_features(test_view_and_branch_lifetime PRIVATE cxx_std_20)

target_link_libraries(test_view_and_branch_lifetime PRIVATE
    ${ROOT_LIBRARIES}
    SKNanoCore
)

add_test(NAME view_and_branch_lifetime
    COMMAND test_view_and_branch_lifetime
)

add_executable(test_lazy_trigger_provider
    test_lazy_trigger_provider.cc
)

target_compile_features(test_lazy_trigger_provider PRIVATE cxx_std_17)

target_include_directories(test_lazy_trigger_provider PRIVATE
    ${CMAKE_SOURCE_DIR}/Analyzers/include
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)

target_link_libraries(test_lazy_trigger_provider PRIVATE
    Analyzers
)

add_test(NAME lazy_trigger_provider
    COMMAND test_lazy_trigger_provider
)

add_executable(test_generated_input_columns
    test_generated_input_columns.cc
)

target_compile_features(test_generated_input_columns PRIVATE cxx_std_17)
target_link_libraries(test_generated_input_columns PRIVATE Analyzers)

add_test(NAME generated_input_columns
    COMMAND test_generated_input_columns
)

# Manual production-schema smoke test.  It is intentionally not registered
# with CTest because the CustomNanoAOD storage path is site-local.
add_executable(custom_bph_file_smoke
    custom_bph_file_smoke.cc
)
target_compile_features(custom_bph_file_smoke PRIVATE cxx_std_17)
target_link_libraries(custom_bph_file_smoke PRIVATE Analyzers)

add_executable(test_histogram_handles
    test_histogram_handles.cc
)

target_compile_features(test_histogram_handles PRIVATE cxx_std_17)
target_link_libraries(test_histogram_handles PRIVATE Analyzers)

add_test(NAME histogram_handles
    COMMAND test_histogram_handles
)

add_executable(test_output_handles
    test_output_handles.cc
)

target_compile_features(test_output_handles PRIVATE cxx_std_17)
target_link_libraries(test_output_handles PRIVATE Analyzers)

add_test(NAME output_handles
    COMMAND test_output_handles
)

add_executable(test_performance_telemetry
    test_performance_telemetry.cc
)

target_compile_features(test_performance_telemetry PRIVATE cxx_std_17)
target_include_directories(test_performance_telemetry PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)

add_test(NAME performance_telemetry
    COMMAND test_performance_telemetry
)

add_executable(test_execution_plan
    test_execution_plan.cc
)

target_compile_features(test_execution_plan PRIVATE cxx_std_17)
target_include_directories(test_execution_plan PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)

add_test(NAME execution_plan
    COMMAND test_execution_plan
)

add_executable(test_event_block
    test_event_block.cc
)
target_compile_features(test_event_block PRIVATE cxx_std_17)
target_include_directories(test_event_block PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME event_block COMMAND test_event_block)

add_executable(test_correction_batch test_correction_batch.cc)
target_compile_features(test_correction_batch PRIVATE cxx_std_17)
target_include_directories(test_correction_batch PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME correction_batch COMMAND test_correction_batch)

add_executable(test_variation_kernel test_variation_kernel.cc)
target_compile_features(test_variation_kernel PRIVATE cxx_std_17)
target_include_directories(test_variation_kernel PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME variation_kernel COMMAND test_variation_kernel)

add_executable(test_event_arena_kernels test_event_arena_kernels.cc)
target_compile_features(test_event_arena_kernels PRIVATE cxx_std_17)
target_include_directories(test_event_arena_kernels PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
target_link_libraries(test_event_arena_kernels PRIVATE SKNanoCore)
add_test(NAME event_arena_kernels COMMAND test_event_arena_kernels)

add_executable(test_cluster_task_planner test_cluster_task_planner.cc)
target_compile_features(test_cluster_task_planner PRIVATE cxx_std_17)
target_include_directories(test_cluster_task_planner PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME cluster_task_planner COMMAND test_cluster_task_planner)

add_executable(test_counter_rng test_counter_rng.cc)
target_compile_features(test_counter_rng PRIVATE cxx_std_17)
target_include_directories(test_counter_rng PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
find_package(Threads REQUIRED)
target_link_libraries(test_counter_rng PRIVATE Threads::Threads)
add_test(NAME counter_rng COMMAND test_counter_rng)

# Compile MLHelper against a small API-compatible ORT test double so the
# prepared binding contract remains testable when the full conda stack is not
# active. Production MLHelper still links the real ONNX Runtime target.
add_executable(test_mlhelper_prepared
    test_mlhelper_prepared.cc
    ${CMAKE_SOURCE_DIR}/AnalyzerTools/src/MLHelper.cc
)
target_compile_features(test_mlhelper_prepared PRIVATE cxx_std_17)
target_include_directories(test_mlhelper_prepared PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${CMAKE_SOURCE_DIR}/AnalyzerTools/include
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME mlhelper_prepared COMMAND test_mlhelper_prepared)

add_executable(test_tree_cache_tuner test_tree_cache_tuner.cc)
target_compile_features(test_tree_cache_tuner PRIVATE cxx_std_17)
target_include_directories(test_tree_cache_tuner PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
add_test(NAME tree_cache_tuner COMMAND test_tree_cache_tuner)

add_executable(test_double_buffer_pipeline test_double_buffer_pipeline.cc)
target_compile_features(test_double_buffer_pipeline PRIVATE cxx_std_17)
target_include_directories(test_double_buffer_pipeline PRIVATE
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)
target_link_libraries(test_double_buffer_pipeline PRIVATE Threads::Threads)
add_test(NAME double_buffer_pipeline COMMAND test_double_buffer_pipeline)

add_executable(test_fatjet_contract
    test_fatjet_contract.cc
)

target_compile_features(test_fatjet_contract PRIVATE cxx_std_17)

target_include_directories(test_fatjet_contract PRIVATE
    ${CMAKE_SOURCE_DIR}/DataFormats/include
    ${CMAKE_SOURCE_DIR}/SKNanoCore/include
)

target_link_libraries(test_fatjet_contract PRIVATE
    DataFormats
)

add_test(NAME fatjet_contract
    COMMAND test_fatjet_contract
)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME branch_schema_pipeline
        COMMAND ${Python3_EXECUTABLE} -m unittest -v
                tests/test_dump_branch_schema.py
    )
    set_tests_properties(branch_schema_pipeline PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(NAME sample_path_resolution
        COMMAND ${Python3_EXECUTABLE} -m unittest -v
                tests/test_sample_paths.py
    )
    set_tests_properties(sample_path_resolution PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(NAME performance_baseline_pipeline
        COMMAND ${Python3_EXECUTABLE} -m unittest -v
                tests/test_performance_baseline.py
    )
    set_tests_properties(performance_baseline_pipeline PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
