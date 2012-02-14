#ifndef WE_LOADER_MODULE_TRAITS_HPP
#define WE_LOADER_MODULE_TRAITS_HPP 1

namespace we
{
  namespace loader
  {
    template <typename T>
    struct module_traits
    {
      static const std::string & extension()
      {
        static std::string s("so");
        return s;
      }

      static const std::string & prefix()
      {
        static std::string s("lib");
        return s;
      }

      static std::string file_name (const std::string & mod_name)
      {
        return prefix() + mod_name + "." + extension();
      }
    };
  }
}

#endif
