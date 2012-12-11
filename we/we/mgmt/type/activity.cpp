// {petry,rahn}@itwm.fhg.de

#include <we/mgmt/type/activity.hpp>

#include <iostream>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      namespace detail
      {
        printer::printer (const activity_t& act, std::ostream& os)
          : _act (act)
          , _os (os)
        {}

        printer& printer::operator<< (const top_list_t& top_list)
        {
          bool first (true);

          _os << "[";

          BOOST_FOREACH(const activity_t::token_on_port_t& top, top_list)
            {
              if (first)
                {
                  first = false;
                }
              else
                {
                  _os << ", ";
                }

              _os << _act.transition().name_of_port (top.second)
                  << "=(" << top.first << ", " << top.second << ")";
            }

          _os << "]";

          return *this;
        }

        printer& printer::operator<< (std::ostream& (*fn)(std::ostream&))
        {
          fn (_os);
          return *this;
        }
      }

      void activity_t::writeTo (std::ostream& os) const
      {
        unique_lock_t lock (_mutex);

        os << "{";
        os << "act, "
           << flags()
           << ", "
           << transition()
           << ", "
          ;

        detail::printer printer (*this, os);
        os << "{input, ";
        printer << input();
        os << "}";
        os << "{pending, ";
        printer << pending_input();
        os << "}";
        os << ", ";
        os << "{output, ";
        printer << output();
        os << "}";

        os << "}";
      }

      bool operator== (const activity_t& a, const activity_t& b)
      {
        return a.id() == b.id();
      }

      std::ostream& operator<< (std::ostream &os, const activity_t & act)
      {
        act.writeTo (os);
        return os;
      }
    }
  }
}
