#include <cstdlib> /* EXIT_XXXX constants. */
#include <iostream>

#include <jpn/common/Foreach.h>

#include "Parsing.h"
#include "PetriNet.h"

int main(int argc, char *argv[]) {
    if (!argc) {
        std::cout << "Usage: pnetv FILE..." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        std::vector<pnetv::PetriNet> petriNets;

        try {
            pnetv::parse(argv[i], petriNets);
            foreach (const pnetv::PetriNet &petriNet, petriNets) {
                std::cout << petriNet.name() << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << argv[i] << ":" << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* vim:set et sts=4 sw=4: */
