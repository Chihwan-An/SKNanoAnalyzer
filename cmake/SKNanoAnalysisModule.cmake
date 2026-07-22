include(CMakeParseArguments)

function(sknano_register_analyzers)
    set_property(GLOBAL APPEND PROPERTY SKNANO_REGISTERED_ANALYZERS ${ARGN})
endfunction()

function(sknano_add_analysis_module)
    set(options)
    set(one_value NAME LINKDEF INCLUDE_DIR)
    set(multi_value HEADERS SOURCES ANALYZERS PUBLIC_LIBRARIES PRIVATE_LIBRARIES)
    cmake_parse_arguments(MODULE "${options}" "${one_value}" "${multi_value}" ${ARGN})

    if(NOT MODULE_NAME OR NOT MODULE_LINKDEF OR NOT MODULE_HEADERS OR NOT MODULE_SOURCES)
        message(FATAL_ERROR
            "sknano_add_analysis_module requires NAME, LINKDEF, HEADERS, and SOURCES")
    endif()
    if(NOT MODULE_INCLUDE_DIR)
        set(MODULE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    endif()

    add_library(${MODULE_NAME} SHARED ${MODULE_SOURCES})
    target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${MODULE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(${MODULE_NAME} PUBLIC
        AnalyzerFramework ${MODULE_PUBLIC_LIBRARIES}
        PRIVATE ${MODULE_PRIVATE_LIBRARIES})
    ROOT_GENERATE_DICTIONARY(G__${MODULE_NAME}
        ${MODULE_HEADERS}
        MODULE ${MODULE_NAME}
        LINKDEF ${MODULE_LINKDEF})

    install(TARGETS ${MODULE_NAME} DESTINATION lib)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.rootmap
        ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}_rdict.pcm
        DESTINATION lib)
    install(DIRECTORY ${MODULE_INCLUDE_DIR}/ DESTINATION include)
    sknano_register_analyzers(${MODULE_ANALYZERS})
endfunction()
