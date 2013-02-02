// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/get.hpp>

#include <we/type/value/show.hpp>
#include <we/type/literal/show.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp>

namespace value
{
  namespace visitor
  {
    namespace exception
    {
      missing_field::missing_field (const std::string& name)
        : std::runtime_error ("get_field: missing field " + name)
      {}


      cannot_get_field_from_literal::cannot_get_field_from_literal
        ( const signature::field_name_t& name
        , const literal::type& l
        )
          : std::runtime_error
            ( ( boost::format ("cannot get field %1% from the literal %2%")
              % name % literal::show (l)
              ).str()
            )
      {}

      empty_path::empty_path()
        : std::runtime_error ("get: empty path")
      {}
    }

    get_field::get_field (const path_type& path)
      : _path (path)
      , _pos (_path.begin())
    {}

    get_field::get_field (const signature::field_name_t& name)
      : _path (fhg::util::split<signature::field_name_t, path_type> (name, '.'))
      , _pos (_path.begin())
    {}

    const type& get_field::operator() (const structured_t& s)
    {
      if (_pos == _path.end())
      {
        throw exception::empty_path();
      }

      structured_t::const_iterator field (s.find (*_pos));

      if (field == s.end())
      {
        throw exception::missing_field (name());
      }

      ++_pos;

      if (_pos == _path.end())
      {
        return field->second;
      }
      else
      {
        return boost::apply_visitor (*this, field->second);
      }
    }

    const type& get_field::operator() (const literal::type& l)
    {
      throw exception::cannot_get_field_from_literal (name(), l);
    }

    std::string get_field::name() const
    {
      return fhg::util::join (_path.begin(), _path.end(), ".");
    }
  }

  const type& get_field (const signature::field_name_t& field, const type& v)
  {
    visitor::get_field get (field);

    return boost::apply_visitor (get, v);
  }

  const type& get_field (const path_type& path, const type& v)
  {
    visitor::get_field get (path);

    return boost::apply_visitor (get, v);
  }

  template<>
  const type& get<type> (const signature::field_name_t& field, const type& v)
  {
    return get_field (field, v);
  }

  template<>
  const type& get<type> (const path_type& path, const type& v)
  {
    return get_field (path, v);
  }

}
