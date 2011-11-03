#ifndef FHG_PLUGIN_STORAGE_HPP
#define FHG_PLUGIN_STORAGE_HPP 1

#include <string>

namespace fhg
{
  namespace plugin
  {
    class Storage
    {
    public:
      virtual ~Storage() {}

      virtual int save (std::string const &key, std::string const &value) = 0;
      virtual int load (std::string const &key, std::string &value) = 0;
      virtual int remove (std::string const &key) = 0;
      virtual int commit () = 0;
      virtual int flush () = 0;
    };
  }
}

#endif
