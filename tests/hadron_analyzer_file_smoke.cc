#include "HadronAnalyzer.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    if (argc < 3 || argc > 5) {
        std::cerr << "usage: hadron_analyzer_file_smoke INPUT.root OUTPUT.root "
                     "[MAX_EVENTS] [COMMA_SEPARATED_FLAGS]\n";
        return 2;
    }

    try {
        HadronAnalyzer analyzer;
        analyzer.SetInputFormat("auto");
        analyzer.SetTreeName("Events");
        analyzer.SetAnalyzerName("HadronAnalyzerFileSmoke");
        analyzer.LogEvery = 5000;
        analyzer.IsDATA = false;
        analyzer.MCSample = "TTLJ_powheg";
        analyzer.xsec = 405.685352016;
        analyzer.sumW = 161967471676.6117;
        analyzer.sumSign = 480548627.;
        analyzer.SetEra("2024");
        analyzer.SetPeriod("");
        const std::string flags = argc >= 5 ? argv[4] : "Mu";
        std::size_t begin = 0;
        while (begin <= flags.size()) {
            const std::size_t end = flags.find(',', begin);
            const std::string flag = flags.substr(begin, end - begin);
            if (!flag.empty())
                analyzer.Userflags.push_back(flag);
            if (end == std::string::npos)
                break;
            begin = end + 1;
        }
        if (analyzer.AddFile(argv[1]) != 1)
            throw std::runtime_error("failed to add input file");
        analyzer.MaxEvent = argc >= 4 ? std::stol(argv[3]) : 100;
        analyzer.SetOutfilePath(argv[2]);
        analyzer.Init();
        analyzer.initializeAnalyzer();
        analyzer.Loop();
        analyzer.WriteHist();
        std::cout << "HADRON_ANALYZER_FILE_SMOKE_OK input_format="
                  << (analyzer.IsRNTupleInput() ? "rntuple" : "ttree")
                  << " input_entries=" << analyzer.GetInputEntries()
                  << " max_events=" << analyzer.MaxEvent << '\n';
    } catch (const std::exception &error) {
        std::cerr << "hadron_analyzer_file_smoke: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
