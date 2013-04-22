// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/field.hpp>
#include <we/type/value/peek.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        static std::list<std::string> _init_path()
        {
          return std::list<std::string>();
        }
        static std::list<std::string>& _path()
        {
          static std::list<std::string> p (_init_path());

          return p;
        }
      }

      path::path (const std::string& x)
        : _key (_path().insert (_path().end(), x))
        , _end (_path().end())
      {}
      path::~path()
      {
        _path().pop_back();
      }
      const std::list<std::string>::const_iterator& path::key() const
      {
        return _key;
      }
      const std::list<std::string>::const_iterator& path::end() const
      {
        return _end;
      }
      const std::list<std::string>& path::operator() () const
      {
        return _path();
      }

      const value_type& field ( const path& p
                              , const value_type& v
                              , const signature_type& signature
                              )
      {
        boost::optional<const value_type&> field (peek (p.key(), p.end(), v));

        if (!field)
        {
          throw exception::missing_field (signature, p());
        }

        return *field;
      }
    }
  }
}
