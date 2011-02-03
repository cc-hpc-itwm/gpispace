// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_FILE_HPP
#define FUSE_FILE_HPP

#include <string>

#include <boost/unordered_set.hpp>

namespace gpi_fuse
{
  namespace file
  {
    static inline std::string data ()
    {
      static const std::string d ("data");

      return d;
    }
    static inline std::string type ()
    {
      static const std::string t ("type");

      return t;
    }
    static inline std::string name ()
    {
      static const std::string n ("name");

      return n;
    }
    static inline std::string alloc  ()
    {
      static const std::string a ("alloc");

      return a;
    }
    static inline std::string free ()
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

        std::size_t num_valid () const
        {
          return _set.size();
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
          _set.insert (name());
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

      static inline valid_handle get_valid_handle ()
      {
        static valid_handle v;

        return v;
      }
      static inline valid_proc get_valid_proc ()
      {
        static valid_proc v;

        return v;
      }
    } // namespace detail

    // ********************************************************************* //

    static inline bool is_valid_handle_file (const std::string & name)
    {
      return detail::get_valid_handle().is_valid (name);
    }
    static inline bool is_valid_proc_file (const std::string & name)
    {
      return detail::get_valid_proc().is_valid (name);
    }

    static inline std::size_t num_valid_handle_file ()
    {
      return detail::get_valid_handle().num_valid();
    }
    static inline std::size_t num_valid_proc_file ()
    {
      return detail::get_valid_proc().num_valid();
    }
  }
}

#endif
