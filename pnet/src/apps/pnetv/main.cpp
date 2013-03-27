#include <cstdlib> /* EXIT_XXXX constants. */
#include <iostream>

#include <boost/program_options.hpp>

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include <jpna/Parsing.h>
#include <jpna/PetriNet.h>
#include <jpna/Verification.h>

#include <fhg/revision.hpp>

enum {
    EXIT_INVALID_ARGUMENTS = EXIT_FAILURE,
    EXIT_IO_ERROR,
    EXIT_MAYBE_LOOPS,
    EXIT_LOOPS
};

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;

    std::vector<std::string> inputFiles;

    po::options_description options("options");
    options.add_options()
        ( "help,h", "this message")
        ( "version,V", "print version information")
        ( "input,i"
          , po::value(&inputFiles)
          , "input file name, - for stdin"
        );

    po::positional_options_description positional;
    positional.add("input", -1);

    po::variables_map variables;
    po::store(po::command_line_parser(argc, argv)
        . options(options).positional(positional).run()
        , variables
    );
    po::notify(variables);

    if (variables.count("version"))
      {
        std::cout << fhg::project_info ("Pnet Verifier");

        return EXIT_SUCCESS;
      }

    if (variables.count("help") || inputFiles.empty()) {
        std::cout << "usage: pnetv [options] FILE..." << std::endl
                  << std::endl
                  << options
                  << std::endl
                  << "possible exit codes:" << std::endl
                  << " " << EXIT_SUCCESS << " - termination is guaranteed" << std::endl
                  << " " << EXIT_INVALID_ARGUMENTS << " - invalid arguments" << std::endl
                  << " " << EXIT_IO_ERROR << " - I/O error" << std::endl
                  << " " << EXIT_MAYBE_LOOPS << " - infinite loops are possible" << std::endl
                  << " " << EXIT_LOOPS << " - infinite loops are guaranteed" << std::endl
                  << std::endl
                  << "  For every file, for each subnet in this file, pnetv checks whether this" << std::endl
                  << "subnet terminates and prints the result on a separate line. This line" << std::endl
                  << "includes the name of the file and the path to the subnet separated by" << std::endl
                  << "double colon (::) as well as the verification result. The latter is either" << std::endl
                  << "TERMINATES, MAYBE_LOOPS (there are only loops involving transitions having" << std::endl
                  << "conditions), or LOOPS (there are loops involving only transitions without" << std::endl
                  << "conditions). In the latter two cases, the names of transitions leading to" << std::endl
                  << "the loop (init:) and constituting the loop (loop:) are printed." << std::endl
                  << "  The `fhg.pnetv.firings_limit' transition property sets the maximum number" << std::endl
                  << "of times a transition can fire. False positives can be eliminated by setting" << std::endl
                  << "this property to (typically) 1 for a transition involved into discovered loop." << std::endl;

        return inputFiles.empty() ? EXIT_INVALID_ARGUMENTS : EXIT_SUCCESS;
    }

    int exitCode = EXIT_SUCCESS;

    FOREACH(const std::string &filename, inputFiles) {
        boost::ptr_vector<jpna::PetriNet> petriNets;

        try {
            if (filename == "-") {
                jpna::parse("stdin", std::cin, petriNets);
            } else {
                jpna::parse(filename.c_str(), petriNets);
            }

            FOREACH (const jpna::PetriNet &petriNet, petriNets) {
                std::cout << petriNet.name() << ": ";
                std::cout.flush();
                jpna::VerificationResult result = jpna::verify(petriNet);
                std::cout << result << std::endl;

                switch (result.result()) {
                    case jpna::VerificationResult::TERMINATES: {
                        break;
                    }
                    case jpna::VerificationResult::MAYBE_LOOPS: {
                        if (exitCode == EXIT_SUCCESS) {
                            exitCode = EXIT_MAYBE_LOOPS;
                        }
                        break;
                    }
                    case jpna::VerificationResult::LOOPS: {
                        exitCode = EXIT_LOOPS;
                        break;
                    }
                    default: {
                        jpn::unreachable();
                    }
                }
            }
        } catch (const std::exception &e) {
            std::cerr << filename << ":" << e.what() << std::endl;
            return EXIT_IO_ERROR;
        }
    }

    return exitCode;
}

/* vim:set et sts=4 sw=4: */
