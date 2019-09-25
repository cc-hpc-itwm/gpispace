#pragma once

#include <certificates.hpp>

#include <boost/test/data/monomorphic.hpp>

#define certificates_data                                                \
  boost::unit_test::data::make                                           \
    ( { gspc::Certificates{}                                             \
      , gspc::Certificates {GSPC_SSL_CERTIFICATES_FOR_TESTS}             \
      }                                                                  \
    )
