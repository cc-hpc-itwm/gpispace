// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>

#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/transition.hpp>
#include <pnete/data/handle/place.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/transition.hpp>

#include <xml/parse/state.hpp>
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

      namespace
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
            transition.name (inc (transition.name()));

            return push_transition (transition, net);
          }
        }
      }

      namespace action
      {
        // ## editing actions ########################################
        // - net -----------------------------------------------------
        // -- transition ---------------------------------------------
        class add_transition : public QUndoCommand
        {
        public:
          add_transition
            ( change_manager_t& change_manager
            , const QObject* origin
            , const handle::net& net
            , const ::xml::parse::type::transition_type& transition
            )
            : QUndoCommand (QObject::tr ("add_transition_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _handle (net)
            , _transition (transition)
          { }

          virtual void undo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::transition_deleted
              , _origin
              , handle::transition (_transition, _handle)
              );

            _handle().erase_transition (_transition);
          }

          virtual void redo()
          {
            _handle().push_transition (_transition);

            _change_manager.emit_signal
              ( &change_manager_t::transition_added
              , NULL
              , handle::transition (_transition, _handle)
              );

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          handle::net _handle;
          ::xml::parse::type::transition_type _transition;
        };

        class remove_transition : public QUndoCommand
        {
        public:
          remove_transition
            ( change_manager_t& change_manager
            , const QObject* origin
            , const handle::transition& transition
            )
            : QUndoCommand (QObject::tr ("remove_transition_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _handle (transition)
            , _transition (_handle())
          { }

          virtual void undo()
          {
            _handle.net()().push_transition (_transition);

            _change_manager.emit_signal
              ( &change_manager_t::transition_added
              , NULL
              , _handle
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::transition_deleted
              , _origin
              , _handle
              );

            _handle.net()().erase_transition (_transition);

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          handle::transition _handle;
          ::xml::parse::type::transition_type _transition;
        };

        // -- place --------------------------------------------------
        class add_place : public QUndoCommand
        {
        public:
          add_place
            ( change_manager_t& change_manager
            , const QObject* origin
            , const handle::net& net
            , const ::xml::parse::type::place_type& place
            )
            : QUndoCommand (QObject::tr ("add_place_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _handle (net)
            , _place (place)
          { }

          virtual void undo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::place_deleted
              , _origin
              , handle::place (_place, _handle)
              );

            _handle().erase_place (_place);
          }

          virtual void redo()
          {
            _handle().push_place (_place);

            _change_manager.emit_signal
              ( &change_manager_t::place_added
              , NULL
              , handle::place (_place, _handle)
              );

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          handle::net _handle;
          ::xml::parse::type::place_type _place;
        };

        class remove_place : public QUndoCommand
        {
        public:
          remove_place
            ( change_manager_t& change_manager
            , const QObject* origin
            , const handle::place& place
            )
            : QUndoCommand (QObject::tr ("remove_place_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _handle (place)
            , _place (_handle())
          { }

          virtual void undo()
          {
            _handle.net()().push_place (_place);

            _change_manager.emit_signal
              ( &change_manager_t::place_added
              , NULL
              , _handle
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::place_deleted
              , _origin
              , _handle
              );

            _handle.net()().erase_place (_handle());

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          handle::place _handle;
          ::xml::parse::type::place_type _place;
        };

        // - function ------------------------------------------------
        // - expression ----------------------------------------------
      }

      // ## editing methods ##########################################
      // - net -------------------------------------------------------

      // -- transition -----------------------------------------------
      void change_manager_t::add_transition
        ( const QObject* origin
        , const ::xml::parse::type::function_type& fun
        , const handle::net& net
        )
      {
        ::xml::parse::type::transition_type transition
          ( xml::parse::id::transition (_state.next_id())
          , net.id()
          , _state.id_mapper()
          );
        transition.function_or_use (fun);
        transition.name (fun.name() ? *fun.name() : "transition");

        //! \todo Don't check for duplicate names when fun.name is set?
        while (net().has_transition (transition.name()))
        {
          transition.name (inc (transition.name()));
        }

        push (new action::add_transition (*this, origin, net, transition));
      }

      void change_manager_t::add_transition
        ( const QObject* origin
        , const handle::net& net
        )
      {
        const ::xml::parse::id::expression expression_id (_state.next_id());
        const ::xml::parse::id::function function_id (_state.next_id());
        const ::xml::parse::id::transition transition_id (_state.next_id());

        ::xml::parse::type::expression_type expression ( expression_id
                                                       , function_id
                                                       , _state.id_mapper()
                                                       );
        ::xml::parse::type::function_type f ( expression
                                            , function_id
                                            , transition_id
                                            , _state.id_mapper()
                                            );

        ::xml::parse::type::transition_type transition ( f
                                                       , transition_id
                                                       , net.id()
                                                       , _state.id_mapper()
                                                       );
        transition.name ("transition");

        while (net().has_transition (transition.name()))
        {
          transition.name (inc (transition.name()));
        }

        push (new action::add_transition (*this, origin, net, transition));
      }

      void change_manager_t::delete_transition
        ( const QObject* origin
        , const handle::transition& transition
        )
      {
        push (new action::remove_transition (*this, origin, transition));
      }

      // -- place ----------------------------------------------------
      void change_manager_t::add_place
        ( const QObject* origin
        , const handle::net& net
        )
      {
        ::xml::parse::type::place_type place
          ( ::xml::parse::id::place (_state.next_id())
          , net.id()
          , _state.id_mapper()
          );
        place.name ("place");

        while (net().has_place (place.name()))
        {
          place.name (inc (place.name()));
        }

        push (new action::add_place (*this, origin, net, place));
      }

      void change_manager_t::delete_place
        ( const QObject* origin
        , const handle::place& place
        )
      {
        push (new action::remove_place (*this, origin, place));
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
          fun.name (name.toStdString());
        }
        else
        {
          fun.name(fhg::util::Nothing<std::string>());
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
