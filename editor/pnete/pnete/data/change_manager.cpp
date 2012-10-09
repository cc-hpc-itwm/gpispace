// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      change_manager_t::change_manager_t (::xml::parse::state::type& state)
        : _state (state)
      {}

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

      namespace action
      {
        // ## editing actions ########################################
        // - net -----------------------------------------------------
        // -- transition ---------------------------------------------
        class remove_transition : public QUndoCommand
        {
        public:
          remove_transition
            ( change_manager_t& change_manager
            , const QObject* origin
            , ::xml::parse::type::transition_type& transition
            , ::xml::parse::type::net_type& net
            )
            : QUndoCommand (QObject::tr ("remove_transition_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _transition (&transition)
            , _transition_copy (transition)
            , _net (net)
          { }

          virtual void undo()
          {
            _transition = &detail::push_transition (_transition_copy, _net);

            _change_manager.emit_signal
              ( &change_manager_t::signal_add_transition
              , _origin
              , *_transition
              , _net
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::signal_delete_transition
              , _origin
              , *_transition
              , _net
              );

            _net.erase_transition (*_transition);
            _transition = NULL;
            _origin = NULL;
          }
        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::type::transition_type* _transition;
          const ::xml::parse::type::transition_type _transition_copy;
          ::xml::parse::type::net_type& _net;
        };

        // -- place --------------------------------------------------
        class remove_place : public QUndoCommand
        {
        public:
          remove_place
            ( change_manager_t& change_manager
            , const QObject* origin
            , ::xml::parse::type::place_type& place
            , ::xml::parse::type::net_type& net
            )
            : QUndoCommand (QObject::tr ("remove_place_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _place (&place)
            , _place_copy (place)
            , _net (net)
          { }

          virtual void undo()
          {
            _place = &detail::push_place (_place_copy, _net);

            _change_manager.emit_signal
              ( &change_manager_t::signal_add_place
              , _origin
              , *_place
              , _net
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::signal_delete_place
              , _origin
              , *_place
              , _net
              );

            _net.erase_place (*_place);
            _place = NULL;
            _origin = NULL;
          }
        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::type::place_type* _place;
          const ::xml::parse::type::place_type _place_copy;
          ::xml::parse::type::net_type& _net;
        };

        // - function ------------------------------------------------
        // - expression ----------------------------------------------
      }

      // ## editing methods ##########################################
      // - net -------------------------------------------------------

      // -- transition -----------------------------------------------
      void change_manager_t::add_transition
      ( const QObject* origin
      , ::xml::parse::type::function_type& fun
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::transition_type transition (_state.next_id());

        transition.function_or_use (fun);
        transition.name = fun.name ? *fun.name : "transition";

        emit_signal ( &change_manager_t::signal_add_transition
                    , origin
                    , detail::push_transition (transition, net)
                    , net
                    );
      }

      void change_manager_t::add_transition
      ( const QObject* origin
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::transition_type transition (_state.next_id());

        transition.name = "transition";

        emit_signal ( &change_manager_t::signal_add_transition
                    , origin
                    , detail::push_transition (transition, net)
                    , net
                    );
      }

      void change_manager_t::delete_transition
        ( const QObject* origin
        , ::xml::parse::type::transition_type& trans
        , ::xml::parse::type::net_type& net
        )
      {
        push (new action::remove_transition (*this, origin, trans, net));
      }

      // -- place ----------------------------------------------------
      void change_manager_t::add_place
      ( const QObject* origin
      , ::xml::parse::type::net_type& net
      )
      {
        ::xml::parse::type::place_type place (_state.next_id());

        place.name = "place";

        emit_signal ( &change_manager_t::signal_add_place
                    , origin
                    , detail::push_place (place, net)
                    , net
                    );
      }

      void change_manager_t::delete_place
        ( const QObject* origin
        , ::xml::parse::type::place_type& place
        , ::xml::parse::type::net_type& net
        )
      {
        push (new action::remove_place (*this, origin, place, net));
      }

      // - function --------------------------------------------------
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

        emit_signal ( &change_manager_t::signal_set_function_name
                    , origin
                    , fun
                    , name
                    );
      }

      // - expression ------------------------------------------------
      void change_manager_t::set_expression
        ( const QObject* origin
        , ::xml::parse::type::expression_type& expression
        , const QString& text
        )
      {
        expression.set (text.toStdString());

        emit_signal ( &change_manager_t::signal_set_expression
                    , origin
                    , expression
                    , text
                    );

        emit_signal ( &change_manager_t::signal_set_expression_parse_result
                    , origin
                    , expression
                    , QString::fromStdString
                      (expr::parse::parse_result (text.toStdString()))
                    );
      }



      //! \todo This surely can be done cleaner with variadic templates.

#define ARG_TYPE(function_type,n)                                       \
  boost::mpl::at_c<boost::function_types::parameter_types<function_type>,n>::type

      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1)
      {
        emit (this->*fun) (arg1);
      }
      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1
                                                 , typename ARG_TYPE(Fun,2) arg2)
      {
        emit (this->*fun) (arg1, arg2);
      }
      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1
                                                 , typename ARG_TYPE(Fun,2) arg2
                                                 , typename ARG_TYPE(Fun,3) arg3)
      {
        emit (this->*fun) (arg1, arg2, arg3);
      }
      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1
                                                 , typename ARG_TYPE(Fun,2) arg2
                                                 , typename ARG_TYPE(Fun,3) arg3
                                                 , typename ARG_TYPE(Fun,4) arg4)
      {
        emit (this->*fun) (arg1, arg2, arg3, arg4);
      }
      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1
                                                 , typename ARG_TYPE(Fun,2) arg2
                                                 , typename ARG_TYPE(Fun,3) arg3
                                                 , typename ARG_TYPE(Fun,4) arg4
                                                 , typename ARG_TYPE(Fun,5) arg5)
      {
        emit (this->*fun) (arg1, arg2, arg3, arg4, arg5);
      }
      template<typename Fun>
      void change_manager_t::emit_signal (Fun fun, typename ARG_TYPE(Fun,1) arg1
                                                 , typename ARG_TYPE(Fun,2) arg2
                                                 , typename ARG_TYPE(Fun,3) arg3
                                                 , typename ARG_TYPE(Fun,4) arg4
                                                 , typename ARG_TYPE(Fun,5) arg5
                                                 , typename ARG_TYPE(Fun,6) arg6)
      {
        emit (this->*fun) (arg1, arg2, arg3, arg4, arg5, arg6);
      }
#undef ARG_TYPE
    }
  }
}
