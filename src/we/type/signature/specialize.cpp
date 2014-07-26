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
        class apply_field_struct : public boost::static_visitor<>
        {
        public:
          apply_field_struct (P p)
            : _p (p)
          {}

          void operator() (std::pair<std::string, structure_type>& s) const
          {
            _p._field_struct (s);
          }
        private:
          P _p;
        };

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
            boost::apply_visitor (apply_field_struct<P> (_p), s);
          }
        private:
          P _p;
        };

        template<typename P>
        class apply_struct : public boost::static_visitor<>
        {
        public:
          apply_struct (P p)
            : _p (p)
          {}
          void operator() (std::pair<std::string, structure_type>& s) const
          {
            _p._struct (s);
          }
        private:
          P _p;
        };

        template<typename P>
        void apply (P p, structured_type& structured)
        {
          boost::apply_visitor (apply_struct<P> (p), structured);
        }
        template<typename P>
        void apply (P p, field_type& field)
        {
          boost::apply_visitor (apply_field<P> (p), field);
        }
        template<typename P>
        void apply (P p, std::pair<std::string, structure_type>& s)
        {
          p._struct (s);
        }

        class mapper
        {
        public:
          mapper (const std::unordered_map<std::string, std::string>& m)
            : _m (m)
          {}
          void _struct (std::pair<std::string, structure_type>& s) const
          {
            s.first = map (s.first);

            for (field_type& f : s.second)
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
        apply (mapper (m), s);
      }
    }
  }
}
