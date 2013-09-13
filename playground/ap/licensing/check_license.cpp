#include <crypto_box.h>
#include <crypto_hash.h>

#include <fstream>
#include <cstring>
#include <iostream>
#include <sstream>

#include <fhg/util/program_info.h>
#include <fhg/util/hex.hpp>
#include <fhg/util/read_file.hpp>

struct license_t
{
  char hash [128];
  int valid_until;
};

enum license_errors
  {
    LICENSE_VALID = 0
  , LICENSE_NOT_VERIFYABLE
  , LICENSE_CORRUPT
  , LICENSE_EXPIRED
  , LICENSE_INVALID
  };

static int check_license (std::string const &cypher)
{
  static std::string license_pub =
    fhg::util::from_hex
    ("e1b99947a1fdb3011216a3b8d0a912df54ca7a81a86698d259108a03e2d37042");
  static std::string application =
    fhg::util::from_hex
    ("b552fb75d5c35dabc33f50dd24eb7b024cbc0f485eb762d66db4df391af70d2d");
  static std::string nonce =
    "8a6057094e67d3fff98cd393";

  char buf [4096];

  if (fhg_get_executable_path (buf, sizeof(buf)) != 0)
  {
    return LICENSE_NOT_VERIFYABLE;
  }

  std::string hash =
    fhg::util::to_hex (crypto_hash (fhg::util::read_file (buf)));

  // decode license
  license_t lic;
  memset (&lic, 0, sizeof(lic));

  {
    try
    {
      std::string data = crypto_box_open ( cypher
                                         , nonce
                                         , license_pub
                                         , application
                                         );
      memcpy (&lic, &data[0], data.size ());
    }
    catch (const char *e)
    {
      return LICENSE_CORRUPT;
    }
  }

  std::cerr << "hash-1: " << std::string (lic.hash, sizeof(lic.hash)) << std::endl;
  std::cerr << "hash-2: " << hash << std::endl;
  std::cerr << "valid: " << lic.valid_until << std::endl;


  if (hash != std::string (lic.hash, sizeof(lic.hash)))
  {
    return LICENSE_INVALID;
  }

  if (lic.valid_until <= time (NULL))
  {
    return LICENSE_EXPIRED;
  }

  return LICENSE_VALID;
}

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "usage: check-license <file>" << std::endl;
    return LICENSE_NOT_VERIFYABLE;
  }

  return check_license (fhg::util::read_file (argv [1]));
}
