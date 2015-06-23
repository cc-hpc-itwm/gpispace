// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/show.hpp>
#include <we/type/signature/traverse.hpp>

#include <util-generic/print_container.hpp>

#include <functional>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class printer
        {
        public:
          printer (std::ostream& os)
            : _os (os)
          {}
          void _struct (const std::pair<std::string, structure_type>& s) const
          {
            _os << fhg::util::print_container<decltype (s.second)>
              ( s.first + " :: [", ", ", "]", s.second
              , [this] (std::ostream& os, const field_type& f) -> std::ostream&
                {
                  traverse (*this, f);
                  return os;
                }
              );
          }
          void _field (const std::pair<std::string, std::string>& f) const
          {
            _os << f.first << " :: " << f.second;
          }
          void _field_struct
            (const std::pair<std::string, structure_type>& s) const
          {
            traverse (*this, s);
          }
        private:
          std::ostream& _os;
        };

        class show_sig : public boost::static_visitor<std::ostream&>
        {
        public:
          show_sig (std::ostream& os)
            : _os (os)
          {}
          std::ostream& operator() (const std::string& tname) const
          {
            return _os << tname;
          }
          std::ostream& operator() (const structured_type& s) const
          {
            traverse (printer (_os), s);
            return _os;
          }
        private:
          std::ostream& _os;
        };
      }

      show::show (const signature_type& signature)
        : _signature (signature)
      {}
      std::ostream& show::operator() (std::ostream& os) const
      {
        return boost::apply_visitor (show_sig (os), _signature);
      }
    }
  }
}
