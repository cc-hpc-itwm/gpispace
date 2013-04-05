// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_FILE_HPP
#define FUSE_FILE_HPP

#include <string>

#include <boost/unordered_set.hpp>

#include <gpifs/util.hpp>

#include <sstream>

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

    typedef boost::unordered_set<std::string> valid_set_t;

    namespace detail
    {
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

        const valid_set_t & set () const
        {
          return _set;
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

      static inline const handle & get_handle ()
      {
        static handle h;

        return h;
      }
      static inline const proc & get_proc ()
      {
        static proc p;

        return p;
      }
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

    namespace set
    {
      static inline const valid_set_t & handle ()
      {
        return detail::get_handle().set();
      }
      static inline const valid_set_t & proc ()
      {
        return detail::get_proc().set();
      }

      static inline std::string string (const valid_set_t & s)
      {
        std::ostringstream str;

        str << "{";

        for (valid_set_t::const_iterator v (s.begin()); v != s.end(); ++v)
          {
            if (v != s.begin())
              {
                str << ", ";
              }

            str << "'" << *v << "'";
          }

        str << "}";

        return str.str();
      }
    } // namespace set
  }
}

#endif
