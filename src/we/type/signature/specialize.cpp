// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/specialize.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        template<typename P>
        class apply_field : public boost::static_visitor<>
        {
        public:
          apply_field (P p)
            : _p (p)
          {}

          void operator() (std::pair<std::string, std::string>& f) const
          {
            _p._field (f);
          }
          void operator() (structured_type& s) const
          {
            _p._field_struct (s);
          }
        private:
          P _p;
        };

        class mapper
        {
        public:
          mapper (const std::unordered_map<std::string, std::string>& m)
            : _m (m)
          {}
          void _struct (structured_type& s) const
          {
            s.first = map (s.first);

            for (field_type& f : s.second)
            {
              boost::apply_visitor (apply_field<mapper> (*this), f);
            }
          }
          void _field (std::pair<std::string, std::string>& f) const
          {
            f.second = map (f.second);
          }
          void _field_struct (structured_type& s) const
          {
            _struct (s);
          }

        private:
          const std::unordered_map<std::string, std::string>& _m;

          const std::string& map (const std::string& x) const
          {
            const std::unordered_map<std::string, std::string>::const_iterator
              pos (_m.find (x));

            return (pos == _m.end()) ? x : pos->second;
          }
        };
      }

      void specialize
        ( structured_type& s
        , const std::unordered_map<std::string, std::string>& m
        )
      {
        mapper(m)._struct(s);
      }
    }
  }
}
