set(SKNANO_INPUT_SCHEMA
    ${CMAKE_SOURCE_DIR}/data/Run3_v15_Run2_v15/NanoAODv15.json)
set(SKNANO_SCHEMA_GENERATOR
    ${CMAKE_SOURCE_DIR}/python/generate_branch_code.py)
set(SKNANO_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(SKNANO_SCHEMA_DEPFILE ${SKNANO_GENERATED_INCLUDE_DIR}/schema_codegen.d)
set(SKNANO_GENERATED_BRANCH_FILES
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_decls.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_register.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_reset.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_input_columns.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_loader_api.inc)

add_custom_command(
    OUTPUT ${SKNANO_GENERATED_BRANCH_FILES}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SKNANO_GENERATED_INCLUDE_DIR}
    COMMAND ${Python3_EXECUTABLE} ${SKNANO_SCHEMA_GENERATOR}
            --json ${SKNANO_INPUT_SCHEMA}
            --out-dir ${SKNANO_GENERATED_INCLUDE_DIR}
            --depfile ${SKNANO_SCHEMA_DEPFILE}
            --no-auto-addons
    DEPENDS ${SKNANO_SCHEMA_GENERATOR} ${SKNANO_INPUT_SCHEMA}
    DEPFILE ${SKNANO_SCHEMA_DEPFILE}
    COMMENT "Generating canonical NanoAOD branch bindings"
    VERBATIM)
add_custom_target(SKNanoBranchCodegen DEPENDS ${SKNANO_GENERATED_BRANCH_FILES})

set(ANALYZER_FRAMEWORK_HEADERS
    include/AnalyzerFramework/AnalyzerCore.h
    include/AnalyzerFramework/BranchManager.h
    include/AnalyzerFramework/GeneratorBase.h
    include/AnalyzerFramework/RNTupleSource.h
    include/AnalyzerFramework/SKNanoLoader.h
    include/AnalyzerFramework/Triggerinfo.h)
set(ANALYZER_FRAMEWORK_SOURCES
    src/AnalyzerCore.cc
    src/AnalyzerCoreHistograms.cc
    src/AnalyzerCoreJets.cc
    src/AnalyzerCoreObjects.cc
    src/AnalyzerCoreOutput.cc
    src/AnalyzerCoreWeights.cc
    src/GeneratorBase.cc
    src/RNTupleSource.cc
    src/SKNanoLoader.cc)

add_library(AnalyzerFramework SHARED ${ANALYZER_FRAMEWORK_SOURCES})
add_dependencies(AnalyzerFramework SKNanoBranchCodegen)
target_include_directories(AnalyzerFramework PUBLIC
    $<BUILD_INTERFACE:${SKNANO_GENERATED_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/DataFormats/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/SKNanoCore/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    $<BUILD_INTERFACE:$ENV{LHAPDF_INCLUDE_DIR}>
    $<BUILD_INTERFACE:$ENV{CORRECTION_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/AnalyzerTools/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/KinematicFitter/include>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>)

ROOT_GENERATE_DICTIONARY(G__AnalyzerFramework
    ${ANALYZER_FRAMEWORK_HEADERS}
    MODULE AnalyzerFramework
    LINKDEF include/AnalyzerFramework/AnalyzerFrameworkLinkDef.hpp
    EXTRA_DEPENDENCIES ${SKNANO_GENERATED_BRANCH_FILES})

target_link_libraries(AnalyzerFramework PUBLIC
    ${ROOT_LIBRARIES} ROOT::ROOTNTuple DataFormats AnalyzerTools SKNanoCore
    KinFitter absl::flat_hash_map absl::strings absl::hash
    nlohmann_json::nlohmann_json)

install(TARGETS AnalyzerFramework DESTINATION lib)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libAnalyzerFramework.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libAnalyzerFramework_rdict.pcm
    DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${SKNANO_GENERATED_BRANCH_FILES} DESTINATION include/AnalyzerFramework)

sknano_register_analyzers(GeneratorBase)
