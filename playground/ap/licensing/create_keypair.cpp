#include <crypto_box.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <fhg/util/hex.hpp>

int main (int ac, char *argv[]) try
 {
   if (ac < 2)
   {
     std::cerr << "usage: create-keypair file-prefix" << std::endl;
     return EXIT_FAILURE;
   }

   std::string pk;
   std::string sk;

   pk = crypto_box_keypair (&sk);

   if (argv [1][0] != '-')
   {
     {
       std::string name (argv [1]);
       std::ofstream ofs (name.c_str ());
       ofs << fhg::util::to_hex (sk) << std::endl;
     }

     {
       std::string name (argv [1]);
       name += ".pub";
       std::ofstream ofs (name.c_str ());
       ofs << fhg::util::to_hex (pk) << std::endl;
     }
   }
   else
   {
     std::cout << "sk: " << fhg::util::to_hex (sk) << std::endl;
     std::cout << "pk: " << fhg::util::to_hex (pk) << std::endl;
   }

   return 0;
 }
 catch (std::exception const &ex)
 {
   std::cerr << "failed: " << ex.what () << std::endl;
   return EXIT_FAILURE;
 }
