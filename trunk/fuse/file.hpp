// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_FILE_HPP
#define FUSE_FILE_HPP

#include <string>
#include <list>

#include <boost/unordered_set.hpp>

#include <util.hpp>

namespace gpifs
{
  namespace file
  {
    namespace name
    {
      namespace handle
      {
        CONST(data)
        CONST(type)
        CONST(name)
      } // namespace handle

      namespace proc
      {
        CONST(alloc)
        CONST(free)
        CONST(refresh)
        CONST(error)
      } // namespace proc
    } // namespace name

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
          _set.insert (name::handle::data());
          _set.insert (name::handle::type());
          _set.insert (name::handle::name());
        }
      };
      class proc : public valid
      {
      public:
        proc () : valid ()
        {
          _set.insert (name::proc::alloc());
          _set.insert (name::proc::free());
          _set.insert (name::proc::refresh());
          _set.insert (name::proc::error());
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
      static inline std::size_t handle ()
      {
        return detail::get_handle().num();
      }
      static inline std::size_t proc ()
      {
        return detail::get_proc().num();
      }
    } // namespace num
  }
}

#endif
