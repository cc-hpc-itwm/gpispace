#include <cstdlib> /* EXIT_XXXX constants. */
#include <iostream>

#include <jpn/common/Foreach.h>

#include <pneta/Parsing.h>
#include <pneta/PetriNet.h>
#include <pneta/Verification.h>

int main(int argc, char *argv[]) {
    if (!argc) {
        std::cout << "Usage: pnetv FILE..." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        boost::ptr_vector<pneta::PetriNet> petriNets;

        const char *filename = argv[i];

        try {
            pneta::parse(filename, petriNets);

            foreach (const pneta::PetriNet &petriNet, petriNets) {
                std::cout << petriNet.name() << ": ";
                std::cout.flush();
                std::cout << pneta::verify(petriNet) << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << filename << ":" << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* vim:set et sts=4 sw=4: */
