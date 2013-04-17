// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/require_type.hpp>
#include <we/type/value/exception.hpp>

#include <boost/format.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_require_type : public boost::static_visitor<>
        {
        public:
          visitor_require_type ( const std::list<std::string>& path
                               = std::list<std::string>()
                               )
            : _path (path)
          {}

#define OKAY(_type...)                                           \
          void operator() (const _type&, const _type&) const {}

          OKAY (we::type::literal::control);
          OKAY (bool);
          OKAY (int);
          OKAY (long);
          OKAY (unsigned int);
          OKAY (unsigned long);
          OKAY (float);
          OKAY (double);
          OKAY (char);
          OKAY (std::string);
          OKAY (bitsetofint::type);
          OKAY (bytearray::type);
          OKAY (std::list<value_type>);
          OKAY (std::vector<value_type>);
          OKAY (std::set<value_type>);
          OKAY (std::map<value_type, value_type>);

#undef OKAY

          void operator() ( const structured_type& signature
                          , const structured_type& value
                          ) const
          {
            const structured_type::const_iterator sig_end (signature.end());
            const structured_type::const_iterator val_end (value.end());
            structured_type::const_iterator sig_pos (signature.begin());
            structured_type::const_iterator val_pos (value.begin());

            while (sig_pos != sig_end && val_pos != val_end)
            {
              _path.push_back (sig_pos->first);

              if (sig_pos->first != val_pos->first)
              {
                throw exception::missing_field (sig_pos->second, _path);
              }

              boost::apply_visitor (*this, sig_pos->second, val_pos->second);

              _path.pop_back();

              ++sig_pos;
              ++val_pos;
            }

            if (sig_pos != sig_end)
            {
              _path.push_back (sig_pos->first);

              throw exception::missing_field (sig_pos->second, _path);
            }

            if (val_pos != val_end)
            {
              _path.push_back (val_pos->first);

              throw exception::unknown_field (val_pos->second, _path);
            }
          }

          template<typename S, typename V>
          void operator() (const S& signature, const V& value) const
          {
            throw exception::type_mismatch (signature, value, _path);
          }

        private:
          mutable std::list<std::string> _path;
        };
      }

      void require_type ( const signature_type& signature
                        , const value_type& value
                        )
      {
        boost::apply_visitor (visitor_require_type(), signature, value);
      }
    }
  }
}
