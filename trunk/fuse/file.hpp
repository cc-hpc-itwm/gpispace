// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_FILE_HPP
#define FUSE_FILE_HPP

#include <string>
#include <list>

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
    static inline std::string refresh ()
    {
      static const std::string f ("refresh");

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

        std::size_t num () const
        {
          return _set.size();
        }

      protected:
        valid_set_t _set;
      };

      class handle : public valid
      {
      public:
        handle () : valid ()
        {
          _set.insert (data());
          _set.insert (type());
          _set.insert (name());
        }
      };
      class proc : public valid
      {
      public:
        proc () : valid ()
        {
          _set.insert (alloc());
          _set.insert (free());
          _set.insert (refresh());
        }
      };

      static inline handle get_handle () { static handle h; return h; }
      static inline proc get_proc () { static proc p; return p; }
    } // namespace detail

    // ********************************************************************* //

    namespace is_valid
    {
      static inline bool handle (const std::string & name)
      {
        return detail::get_handle().is_valid (name);
      }
      static inline bool proc (const std::string & name)
      {
        return detail::get_proc().is_valid (name);
      }
    } // namespace is_valid

    namespace num
    {
      static inline std::size_t handle () { return detail::get_handle().num(); }
      static inline std::size_t proc () { return detail::get_proc().num(); }
    } // namespace num
  }
}

#endif
