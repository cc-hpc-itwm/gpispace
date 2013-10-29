#include "license.hpp"

#ifdef FHG_ENABLE_LICENSE_CHECK

#include <crypto_box.h>
#include <crypto_hash.h>

#include <cstring>

#include <fhg/util/program_info.h>
#include <fhg/util/hex.hpp>
#include <fhg/util/read_file.hpp>

namespace fhg
{
  namespace plugin
  {
    struct license_t
    {
      enum { HASH_SIZE = 2 * crypto_hash_BYTES }; // hexadecimal representation

      char hash [HASH_SIZE];
      int  expiry;
    };

    int check_license (std::string const &license)
    {
      static std::string license_pub =
        fhg::util::from_hex
        ("e1b99947a1fdb3011216a3b8d0a912df54ca7a81a86698d259108a03e2d37042");
      static std::string appkey =
        fhg::util::from_hex
        ("b552fb75d5c35dabc33f50dd24eb7b024cbc0f485eb762d66db4df391af70d2d");
      static std::string nonce =
        "8a6057094e67d3fff98cd393";

      char buf [4096];

      if (fhg_get_executable_path (buf, sizeof(buf)) != 0)
      {
        return LICENSE_NOT_VERIFYABLE;
      }

      std::string hash;

      try
      {
        hash = fhg::util::to_hex (crypto_hash (fhg::util::read_file (buf)));
      }
      catch (std::exception const &)
      {
        return LICENSE_NOT_VERIFYABLE;
      }

      if (hash.size () != license_t::HASH_SIZE)
      {
        return LICENSE_NOT_VERIFYABLE;
      }

      // decode license
      license_t lic;
      memset (&lic, 0, sizeof(lic));

      try
      {
        std::string data = crypto_box_open ( license
                                           , nonce
                                           , license_pub
                                           , appkey
                                           );
        if (data.size () != sizeof(license_t))
        {
          return LICENSE_VERSION_MISMATCH;
        }

        memcpy (&lic, &data[0], sizeof(license_t));
      }
      catch (const char *e)
      {
        return LICENSE_CORRUPT;
      }

      if (0 != memcmp (hash.data (), lic.hash, license_t::HASH_SIZE))
      {
        return LICENSE_INVALID;
      }

      if (lic.expiry <= time (NULL))
      {
        return LICENSE_EXPIRED;
      }

      return LICENSE_VALID;
    }

    int check_license_file (std::string const &path)
    {
      try
      {
        return check_license (fhg::util::read_file (path));
      }
      catch (std::exception const &)
      {
        return LICENSE_NOT_VERIFYABLE;
      }
    }
  }
}

#else

namespace fhg
{
  namespace plugin
  {
    int check_license (std::string const &license)
    {
      return LICENSE_VALID;
    }

    int check_license_file (std::string const &path)
    {
      return LICENSE_VALID;
    }
  }
}

#endif
