#include <cstdlib> /* EXIT_XXXX constants. */
#include <iostream>

#include <jpn/common/Foreach.h>

#include <jpna/Parsing.h>
#include <jpna/PetriNet.h>
#include <jpna/Verification.h>

int main(int argc, char *argv[]) {
    if (!argc) {
        std::cout << "Usage: pnetv FILE..." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        boost::ptr_vector<jpna::PetriNet> petriNets;

        const char *filename = argv[i];

        try {
            jpna::parse(filename, petriNets);

            foreach (const jpna::PetriNet &petriNet, petriNets) {
                std::cout << petriNet.name() << ": ";
                std::cout.flush();
                std::cout << jpna::verify(petriNet) << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << filename << ":" << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* vim:set et sts=4 sw=4: */
