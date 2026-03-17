#pragma once

#include <future>



    namespace gspc::util::cxx17
    {
      //! std::future_error in c++11 has a exposition only constructor
      //! from std::error_code. In c++17, this changed to
      //! std::future_errc. This means that in c++11 and c++14 there
      //! is no standard-compliant way to actually construct a
      //! future_error. Most implementations (at least libstdcxx)
      //! provide that exposition only constructor though, so that
      //! there is a way to actually construct it.
      std::future_error make_future_error (std::error_code const&);
    }



#include <gspc/util/cxx17/future_error.ipp>
