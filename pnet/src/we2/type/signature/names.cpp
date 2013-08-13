// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/names.hpp>
#include <we2/type/signature/traverse.hpp>

#include <boost/foreach.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class get_names_rec
        {
        public:
          get_names_rec (boost::unordered_set<std::string>& names)
            : _names (names)
          {}
          void _struct (const std::pair<std::string, structure_type>& s) const
          {
            BOOST_FOREACH (const field_type& f, s.second)
            {
              traverse (*this, f);
            }
          }
          void _field (const std::pair<std::string, std::string>& f) const
          {
            _names.insert (f.second);
          }
          void _field_struct
            (const std::pair<std::string, structure_type>& s) const
          {
            traverse (*this, s);
          }

        private:
          boost::unordered_set<std::string>& _names;
        };

        class get_names : public boost::static_visitor<>
        {
        public:
          get_names (boost::unordered_set<std::string>& names)
            : _names (names)
          {}
          void operator() (const std::string& tname) const
          {
            _names.insert (tname);
          }
          void operator() (const structured_type& s) const
          {
            traverse (get_names_rec (_names), s);
          }

        private:
          boost::unordered_set<std::string>& _names;
        };
      }

      boost::unordered_set<std::string> names (const signature_type& signature)
      {
        boost::unordered_set<std::string> names;

        boost::apply_visitor (get_names (names), signature);

        return names;
      }
    }
  }
}
