sknano_add_analysis_module(
    NAME MyAnalysis
    HEADERS include/MyAnalysis/MyAnalyzer.h
    SOURCES src/MyAnalyzer.cc
    LINKDEF include/MyAnalysis/LinkDef.hpp
    ANALYZERS MyAnalyzer)
