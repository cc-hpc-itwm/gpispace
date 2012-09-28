// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace detail
      {
        static std::string inc (const std::string& s)
        {
          unsigned long num (0);

          std::string::const_iterator end (s.end());
          std::string::const_reverse_iterator pos (s.rbegin());

          while (pos != s.rend() && isdigit (*pos))
            {
              num *= 10;
              num += *pos - '0';
              --end; ++pos;
            }

          return std::string (s.begin(), end)
            + ((end == s.end()) ? "_" : "")
            + boost::lexical_cast<std::string> (num + 1)
            ;
        }

        static ::xml::parse::type::place_type&
        push_place ( ::xml::parse::type::place_type place
                   , ::xml::parse::type::net_type& net
                   )
        {
          try
            {
              return net.push_place (place);
            }
          catch (const ::xml::parse::error::duplicate_place&)
            {
              place.name = detail::inc (place.name);

              return push_place (place, net);
            }
        }

        static ::xml::parse::type::transition_type&
        push_transition ( ::xml::parse::type::transition_type transition
                        , ::xml::parse::type::net_type& net
                        )
        {
          try
            {
              return net.push_transition (transition);
            }
          catch (const ::xml::parse::error::duplicate_transition
                       < ::xml::parse::type::transition_type>&
                )
            {
              transition.name = detail::inc (transition.name);

              return push_transition (transition, net);
            }
        }
      }

      change_manager_t::change_manager_t (internal::type & i)
        : _internal (i)
      {}

      internal::type& change_manager_t::internal () const
      {
        return _internal;
      }

      void change_manager_t::set_function_name
      ( const QObject* origin
      , ::xml::parse::type::function_type& fun
      , const QString& name
      )
      {
        if (!name.isEmpty())
        {
          fun.name = name.toStdString();
        }
        else
        {
          fun.name.clear();
        }

        emit signal_set_function_name (origin, fun, name);
      }

      void change_manager_t::set_expression
      ( const QObject* origin
      , ::xml::parse::type::expression_type& expression
      , const QString& text
      )
      {
        expression.set (text.toStdString());

        emit signal_set_expression (origin, expression, text);

        emit signal_set_expression_parse_result
          ( origin
          , expression
          , QString::fromStdString
            (expr::parse::parse_result (text.toStdString()))
          );
      }

      void change_manager_t::delete_transition
      ( const QObject* origin
      , const ::xml::parse::type::transition_type& trans
      , ::xml::parse::type::net_type& net
      )
      {
        emit signal_delete_transition (origin, trans, net);

        net.erase_transition (trans);
      }

      void change_manager_t::add_transition
      ( const QObject* origin
      , ::xml::parse::type::function_type& fun
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::transition_type transition;

        transition.f = fun;
        transition.name = fun.name ? *fun.name : "transition";

        emit signal_add_transition
          (origin, detail::push_transition (transition, net), net);
      }

      void change_manager_t::add_transition
      ( const QObject* origin
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::transition_type transition;

        transition.name = "transition";

        emit signal_add_transition
          (origin, detail::push_transition (transition, net), net);
      }
      void change_manager_t::add_place
      ( const QObject* origin
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::place_type place;

        place.name = "place";

        emit signal_add_place (origin, detail::push_place (place, net), net);
      }
    }
  }
}
