#pragma once

#include <boost/filesystem/path.hpp>

#include <ostream>
#include <string>

namespace gspc
{
  namespace task
  {
    struct Implementation
    {
      //! \note used by worker to load and call module implementation
      //! \todo search path + basename only? id? gspc: searchpath + basename - "lib"
      boost::filesystem::path so;
      //! \todo string or index or hash? gspc: string (unmangled)
      using Symbol = std::string;
      Symbol symbol;

      friend std::ostream& operator<< (std::ostream&, Implementation const&);

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);
    };
  }
}

#include <gspc/task/Implementation.ipp>
