// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_FILE_HPP
#define FUSE_FILE_HPP

#include <string>

#include <boost/unordered_set.hpp>

namespace gpi_fuse
{
  namespace file
  {
    static std::string data ()
    {
      static const std::string d ("data");

      return d;
    }
    static std::string type ()
    {
      static const std::string t ("type");

      return t;
    }
    static std::string alloc  ()
    {
      static const std::string a ("alloc");

      return a;
    }
    static std::string free ()
    {
      static const std::string f ("free");

      return f;
    }

    // ********************************************************************* //

    namespace detail
    {
      typedef boost::unordered_set<std::string> valid_set_t;

      class valid
      {
      public:
        valid () : _set () {}

        bool is_valid (const std::string & name) const
        {
          return _set.find (name) != _set.end();
        }

      protected:
        valid_set_t _set;
      };

      class valid_handle : public valid
      {
      public:
        valid_handle () : valid ()
        {
          _set.insert (data());
          _set.insert (type());
        }
      };

      class valid_proc : public valid
      {
      public:
        valid_proc () : valid ()
        {
          _set.insert (alloc());
          _set.insert (free());
        }
      };
    } // namespace detail

    // ********************************************************************* //

    static bool is_valid_handle_file (const std::string & name)
    {
      static detail::valid_handle v;

      return v.is_valid (name);
    }

    static bool is_valid_proc_file (const std::string & name)
    {
      static detail::valid_proc v;

      return v.is_valid (name);
    }
  }
}

#endif
