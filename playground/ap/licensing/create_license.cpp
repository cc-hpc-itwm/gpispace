#include <crypto_box.h>
#include <crypto_hash.h>

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

#include <fhg/util/read.hpp>
#include <fhg/util/hex.hpp>
#include <fhg/util/read_file.hpp>

struct license_t
{
  char hash [128];
  int valid_until;
};

static std::string license =
 fhg::util::from_hex
 ("ac203dcf5e1728a9100010f62a344da0ffc791664e27e4a57b6644d11b6d689d");
static std::string license_pub =
 fhg::util::from_hex
 ("e1b99947a1fdb3011216a3b8d0a912df54ca7a81a86698d259108a03e2d37042");

static std::string application =
 fhg::util::from_hex
 ("b552fb75d5c35dabc33f50dd24eb7b024cbc0f485eb762d66db4df391af70d2d");
static std::string application_pub =
 fhg::util::from_hex
 ("59e26d0030873c04584717273300f5be8e80fe98b9f8eaeecf4485ccb5d9cb4a");

static std::string nonce =
 "8a6057094e67d3fff98cd393";

int main (int ac, char *argv[])
{
  if (ac < 3)
  {
    std::cerr << "usage: create-license <path-to-binary> <valid-until>" << std::endl
              << std::endl
              << "   path-to-binary: the binary that performs the license check" << std::endl
              << "   valid-until: time in unix seconds" << std::endl
      ;
    return EXIT_FAILURE;
  }

  license_t lic;
  memset (&lic, 0, sizeof(lic));

  lic.valid_until = fhg::util::read<int> (argv [2]);

  {
    std::string hash =
      fhg::util::to_hex (crypto_hash (fhg::util::read_file (argv [1])));

    if (hash.size () != sizeof(lic.hash))
    {
      std::cerr << "failed: hash has invalid size:"
                << " expected: " << sizeof(lic.hash)
                << " got: " << hash.size ()
                << std::endl;
      return EXIT_FAILURE;
    }

    memcpy (lic.hash, hash.c_str (), hash.size ());
  }

  std::cerr << "hash: " << std::string (lic.hash, sizeof(lic.hash)) << std::endl;
  std::cerr << "valid: " << lic.valid_until << std::endl;

  std::string cypher;

  try
  {
    cypher = crypto_box ( std::string ((char*)&lic, sizeof(lic))
                        , nonce
                        , application_pub
                        , license
                        );
  }
  catch (const char *e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  std::cout.write (cypher.data (), cypher.size ());

  return EXIT_SUCCESS;
}
