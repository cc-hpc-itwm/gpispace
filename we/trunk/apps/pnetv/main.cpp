#include <cstdlib> /* EXIT_XXXX constants. */
#include <iostream>

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include <jpna/Parsing.h>
#include <jpna/PetriNet.h>
#include <jpna/Verification.h>

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "Usage: pnetv FILE..." << std::endl
                  << "Load compiled Petri nets from given files and check them for termination." << std::endl
                  << "Possible exit codes:" << std::endl
                  << " 0 - termination is guaranteed" << std::endl
                  << " 1 - wrong arguments" << std::endl
                  << " 2 - I/O error" << std::endl
                  << " 3 - loops are possible" << std::endl
                  << " 4 - loops are guaranteed" << std::endl;
        return 1;
    }

    int exitCode = 0;

    for (int i = 1; i < argc; ++i) {
        boost::ptr_vector<jpna::PetriNet> petriNets;

        const char *filename = argv[i];

        try {
            jpna::parse(filename, petriNets);

            foreach (const jpna::PetriNet &petriNet, petriNets) {
                std::cout << petriNet.name() << ": ";
                std::cout.flush();
                jpna::VerificationResult result = jpna::verify(petriNet);
                std::cout << result << std::endl;

                switch (result.result()) {
                    case jpna::VerificationResult::TERMINATES:
                        break;
                    case jpna::VerificationResult::MAYBE_LOOPS:
                        if (exitCode == 0) {
                            exitCode = 3;
                        }
                        break;
                    case jpna::VerificationResult::LOOPS:
                        exitCode = 4;
                        break;
                    default:
                        jpn::unreachable();
                }
            }
        } catch (const std::exception &e) {
            std::cerr << filename << ":" << e.what() << std::endl;
            return 2;
        }
    }

    return exitCode;
}

/* vim:set et sts=4 sw=4: */
