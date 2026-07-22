set(COMMON_ANALYZER_HEADERS
    include/CommonAnalyzers/ExampleRun.h
    include/CommonAnalyzers/GetEffLumi.h
    include/CommonAnalyzers/ModellingPatch.h)
set(COMMON_ANALYZER_SOURCES
    src/ExampleRun.cc
    src/GetEffLumi.cc
    src/ModellingPatch.cc)

add_library(CommonAnalyzers SHARED ${COMMON_ANALYZER_SOURCES})
target_include_directories(CommonAnalyzers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
ROOT_GENERATE_DICTIONARY(G__CommonAnalyzers
    ${COMMON_ANALYZER_HEADERS}
    MODULE CommonAnalyzers
    LINKDEF include/CommonAnalyzers/CommonAnalyzersLinkDef.hpp)
target_link_libraries(CommonAnalyzers PUBLIC AnalyzerFramework)

install(TARGETS CommonAnalyzers DESTINATION lib)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libCommonAnalyzers.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libCommonAnalyzers_rdict.pcm
    DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

sknano_register_analyzers(ExampleRun GetEffLumi ModellingPatch)
