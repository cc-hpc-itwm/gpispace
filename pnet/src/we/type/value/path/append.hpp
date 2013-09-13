// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_PATH_APPEND_HPP
#define PNET_SRC_WE_TYPE_VALUE_PATH_APPEND_HPP

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        class append
        {
        public:
          append (std::list<std::string>&, const std::string&);
          ~append();
          operator std::list<std::string>&() const;
        private:
          std::list<std::string>& _path;
        };
      }
    }
  }
}

#endif
