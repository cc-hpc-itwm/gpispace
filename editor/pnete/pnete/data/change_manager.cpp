// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>

#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/transition.hpp>
#include <pnete/data/handle/place.hpp>

#include <xml/parse/id/types.hpp>
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
            , const ::xml::parse::id::ref::net& net
            , const ::xml::parse::id::ref::transition& transition
            )
            : QUndoCommand (QObject::tr ("add_transition_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _transition (transition)
            , _net (net)
          { }

          virtual void undo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::transition_deleted
              , _origin
              , handle::transition (_transition, _change_manager)
              );

            _net.get_ref().erase_transition (_transition);
          }

          virtual void redo()
          {
            _net.get_ref().push_transition (_transition);

            _change_manager.emit_signal
              ( &change_manager_t::transition_added
              , NULL
              , handle::transition (_transition, _change_manager)
              );

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::id::ref::transition _transition;
          ::xml::parse::id::ref::net _net;
        };

        class remove_transition : public QUndoCommand
        {
        public:
          remove_transition
            ( change_manager_t& change_manager
            , const QObject* origin
            , const ::xml::parse::id::ref::transition& transition
            )
            : QUndoCommand (QObject::tr ("remove_transition_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _transition (transition)
            , _net (_transition.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            _net.get_ref().push_transition (_transition);

            _change_manager.emit_signal
              ( &change_manager_t::transition_added
              , NULL
              , handle::transition (_transition, _change_manager)
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::transition_deleted
              , _origin
              , handle::transition (_transition, _change_manager)
              );

            _net.get_ref().erase_transition (_transition);

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::id::ref::transition _transition;
          ::xml::parse::id::ref::net _net;
        };

        // -- place --------------------------------------------------
        class add_place : public QUndoCommand
        {
        public:
          add_place
            ( change_manager_t& change_manager
            , const QObject* origin
            , const ::xml::parse::id::ref::net& net
            , const ::xml::parse::id::ref::place& place
            )
            : QUndoCommand (QObject::tr ("add_place_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _place (place)
            , _net (net)
          { }

          virtual void undo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::place_deleted
              , _origin
              , handle::place (_place, _change_manager)
              );

            _net.get_ref().erase_place (_place);
          }

          virtual void redo()
          {
            _net.get_ref().push_place (_place);

            _change_manager.emit_signal
              ( &change_manager_t::place_added
              , NULL
              , handle::place (_place, _change_manager)
              );

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::id::ref::place _place;
          ::xml::parse::id::ref::net _net;
        };

        class remove_place : public QUndoCommand
        {
        public:
          remove_place
            ( change_manager_t& change_manager
            , const QObject* origin
            , const ::xml::parse::id::ref::place& place
            )
            : QUndoCommand (QObject::tr ("remove_place_action"))
            , _change_manager (change_manager)
            , _origin (origin)
            , _place (place)
            , _net (_place.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            _net.get_ref().push_place (_place);

            _change_manager.emit_signal
              ( &change_manager_t::place_added
              , NULL
              , handle::place (_place, _change_manager)
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &change_manager_t::place_deleted
              , _origin
              , handle::place (_place, _change_manager)
              );

            _net.get_ref().erase_place (_place);

            _origin = NULL;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          ::xml::parse::id::ref::place _place;
          ::xml::parse::id::ref::net _net;
        };

        // - function ------------------------------------------------
        // - expression ----------------------------------------------
      }

      // ## editing methods ##########################################
      // - net -------------------------------------------------------

      // -- transition -----------------------------------------------
      void change_manager_t::add_transition
        ( const QObject* origin
        , const ::xml::parse::id::ref::function& fun
        , const handle::net& net
        )
      {
        const ::xml::parse::id::transition transition_id (_state.id_mapper()->next_id());

        {
          const ::xml::parse::type::transition_type transition
            (transition_id, _state.id_mapper(), net.id().id(), fun);
        }

        const ::xml::parse::id::ref::transition transition
          (transition_id, _state.id_mapper());

        transition.get_ref().function_or_use (fun);
        fun.get_ref().parent (transition_id);
        transition.get_ref().name (fun.get().name() ? *fun.get().name() : "transition");

        //! \todo Don't check for duplicate names when fun.name is set?
        while (net.get().has_transition (transition.get().name()))
        {
          transition.get_ref().name (inc (transition.get().name()));
        }

        push (new action::add_transition (*this, origin, net.id(), transition));
      }

      void change_manager_t::add_transition
        ( const QObject* origin
        , const handle::net& net
        )
      {
        const ::xml::parse::id::function function_id (_state.id_mapper()->next_id());
        const ::xml::parse::id::transition transition_id (_state.id_mapper()->next_id());

        const ::xml::parse::id::ref::transition transition
          ( ::xml::parse::type::transition_type
            ( transition_id
            , _state.id_mapper()
            , net.id().id()
            , ::xml::parse::id::ref::function
              ( ::xml::parse::type::function_type
                ( function_id
                , _state.id_mapper()
                , ::xml::parse::type::function_type::make_parent (transition_id)
                , ::xml::parse::type::expression_type ( _state.id_mapper()->next_id()
                                                      , _state.id_mapper()
                                                      , function_id
                                                      ).make_reference_id()
                ).make_reference_id()
              )
            ).make_reference_id()
          );

        transition.get_ref().name ("transition");

        //! \todo Don't check for duplicate names when fun.name is set?
        while (net.get().has_transition (transition.get().name()))
        {
          transition.get_ref().name (inc (transition.get().name()));
        }

        push (new action::add_transition (*this, origin, net.id(), transition));
      }

      void change_manager_t::delete_transition
        ( const QObject* origin
        , const handle::transition& transition
        )
      {
        push (new action::remove_transition (*this, origin, transition.id()));
      }

      // -- place ----------------------------------------------------
      void change_manager_t::add_place
        ( const QObject* origin
        , const handle::net& net
        )
      {
        const ::xml::parse::id::place id (_state.id_mapper()->next_id());
        {
          ::xml::parse::type::place_type place
            (id, _state.id_mapper(), net.id().id());
        }

        const ::xml::parse::id::ref::place place (id, _state.id_mapper());
        place.get_ref().name ("place");

        while (net.get().has_place (place.get().name()))
        {
          place.get_ref().name (inc (place.get().name()));
        }

        push (new action::add_place (*this, origin, net.id(), place));
      }

      void change_manager_t::delete_place
        ( const QObject* origin
        , const handle::place& place
        )
      {
        push (new action::remove_place (*this, origin, place.id()));
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
          fun.name (boost::none);
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
