// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/specialize.hpp>
#include <we2/type/signature/apply.hpp>

#include <boost/foreach.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class mapper
        {
        public:
          mapper (const boost::unordered_map<std::string, std::string>& m)
            : _m (m)
          {}
          void _struct (std::pair<std::string, structure_type>& s) const
          {
            s.first = map (s.first);

            BOOST_FOREACH (field_type& f, s.second)
            {
              apply (*this, f);
            }
          }
          void _field (std::pair<std::string, std::string>& f) const
          {
            f.second = map (f.second);
          }
          void _field_struct (std::pair<std::string, structure_type>& s) const
          {
            apply (*this, s);
          }

        private:
          const boost::unordered_map<std::string, std::string>& _m;

          const std::string& map (const std::string& x) const
          {
            const boost::unordered_map<std::string, std::string>::const_iterator
              pos (_m.find (x));

            return (pos == _m.end()) ? x : pos->second;
          }
        };
      }

      void specialize
        ( structured_type& s
        , const boost::unordered_map<std::string, std::string>& m
        )
      {
        apply (mapper (m), s);
      }
    }
  }
}
