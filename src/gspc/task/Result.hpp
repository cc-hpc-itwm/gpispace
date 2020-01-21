#pragma once

#include <gspc/task/ID.hpp>
#include <gspc/value_type.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <exception>
#include <ostream>
#include <string>
#include <unordered_map>

namespace gspc
{
  namespace task
  {
    //! \todo Move to separate files.
    namespace result
    {
      struct Success
      {
        //! \todo why not std::vector<char>
        using Outputs = std::unordered_multimap<std::string, value_type>;
        Outputs outputs;

        friend std::ostream& operator<< (std::ostream&, Success const&);

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };

      struct Failure
      {
        std::exception_ptr exception;

        friend std::ostream& operator<< (std::ostream&, Failure const&);

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };

      struct CancelIgnored
      {
        task::ID after_inject;

        friend std::ostream& operator<< (std::ostream&, CancelIgnored const&);

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };

      struct CancelOptional
      {
        task::ID after_inject;

        friend std::ostream& operator<< (std::ostream&, CancelOptional const&);

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };

      struct Postponed
      {
        std::string reason;

        friend std::ostream& operator<< (std::ostream&, Postponed const&);

        template<typename Archive>
          void serialize (Archive& ar, unsigned int);
      };
    }

    using Result = boost::variant < result::Success
                                  , result::Failure
                                  , result::CancelIgnored
                                  , result::CancelOptional
                                  , result::Postponed
                                  >;
  }
}

#include <gspc/task/Result.ipp>
