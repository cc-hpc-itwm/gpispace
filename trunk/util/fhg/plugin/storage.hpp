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

      virtual int add_storage(std::string const &) = 0;
      virtual Storage *get_storage(std::string const&) const = 0;
      virtual int del_storage(std::string const &) = 0;

      virtual int save (std::string const &key, std::string const &value) = 0;
      virtual int load (std::string const &key, std::string &value) = 0;
      virtual bool exists (std::string const &key) const = 0;
      virtual int remove (std::string const &key) = 0;
      virtual int commit () = 0;
    };
  }
}

#endif
