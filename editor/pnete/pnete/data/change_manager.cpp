// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/expression.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>
#include <pnete/data/handle/transition.hpp>

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
#include <boost/smart_ptr.hpp>

#include <QPointF>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      change_manager_t::change_manager_t (QObject* parent)
        : QUndoStack (parent)
      { }

      namespace
      {
        namespace ids
        {
          enum
          {
            move_place_item,
            move_port_item,
            move_transition_item,
          };
        }

        //! \note Uses inheritance, then 'using base::fun' to expose
        //! protected member functions to non-friends.
        //! See http://stackoverflow.com/questions/75538#1065606
        struct signal : change_manager_t
        {
#define EXPOSE(NAME) using change_manager_t::NAME

        // - net -----------------------------------------------------
        // -- connection ---------------------------------------------
        EXPOSE (property_changed);

        // -- transition ---------------------------------------------
        EXPOSE (transition_added);
        EXPOSE (transition_deleted);

        // -- place --------------------------------------------------
        EXPOSE (place_added);
        EXPOSE (place_deleted);

        // - port ----------------------------------------------------

        // - function ------------------------------------------------
        EXPOSE (function_name_changed);

        // - expression ---------------------------------------------
        EXPOSE (signal_set_expression);
        EXPOSE (signal_set_expression_parse_result);

#undef EXPOSE
        };

        std::string inc (const std::string& s)
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
#define ACTION_ARG_LIST                             \
              change_manager_t& change_manager      \
            , const QObject* origin

#define ACTION_INIT(NAME)                           \
                QUndoCommand (QObject::tr (NAME))   \
              , _change_manager (change_manager)    \
              , _origin (origin)

#define ACTION_MEMBERS                              \
          change_manager_t& _change_manager;        \
          const QObject* _origin

        template<typename T>
          ::we::type::property::value_type to_property_type (const T& t)
        {
          return
            boost::lexical_cast< ::we::type::property::value_type> (t);
        }

        template<typename T>
          T from_property_type (const ::we::type::property::value_type& v)
        {
          return
            boost::lexical_cast<T> (v);
        }

        template<typename HANDLE_TYPE>
          void set_property ( const HANDLE_TYPE& handle
                            , const ::we::type::property::key_type& key
                            , const ::we::type::property::value_type& val
                            , change_manager_t& change_manager
                            , const QObject* origin
                            )
        {
          typedef HANDLE_TYPE handle_type;
          typedef void (change_manager_t::* signal_type)
            ( const QObject*
            , const handle_type&
            , const ::we::type::property::key_type&
            , const ::we::type::property::value_type&
            );

          handle.get_ref().properties().set (key, val);
          change_manager.emit_signal<signal_type>
            ( &signal::property_changed, origin
            , handle, key, val
            );
        }

        // ## editing actions ########################################
        template<typename HANDLE_TYPE>
          class meta_set_property : public QUndoCommand
        {
        private:
          typedef HANDLE_TYPE handle_type;

        public:
          meta_set_property
            ( const char* name
            , change_manager_t& change_manager
            , const QObject* origin
            , const handle_type& handle
            , const ::we::type::property::key_type& key
            , const ::we::type::property::value_type& val
            )
              : QUndoCommand (QObject::tr (name))
              , _change_manager (change_manager)
              , _origin (origin)
              , _handle (handle)
              , _key (key)
              , _new_value (val)
              , _old_value (handle.get().properties().get_val (key))
          { }

          virtual void undo()
          {
            set_property (_handle, _key, _old_value, _change_manager, NULL);
          }

          virtual void redo()
          {
            set_property
              (_handle, _key, _new_value, _change_manager, _origin);
            _origin = NULL;
          }

          const ::we::type::property::value_type& new_value
            (const ::we::type::property::value_type& new_value_)
          {
            return _new_value = new_value_;
          }
          const ::we::type::property::value_type& new_value() const
          {
            return _new_value;
          }

        private:
          change_manager_t& _change_manager;
          const QObject* _origin;
          const handle_type _handle;
          const ::we::type::property::key_type _key;
          ::we::type::property::value_type _new_value;
          const ::we::type::property::value_type _old_value;
        };

        // - net -----------------------------------------------------
        template<typename HANDLE_TYPE>
          class meta_move_item : public QUndoCommand
        {
        private:
          typedef HANDLE_TYPE handle_type;

        public:
          meta_move_item
            ( const char* name
            , int id
            , change_manager_t& change_manager
            , const QObject* origin
            , const handle_type& handle
            , const QPointF& position
            )
              : QUndoCommand (QObject::tr (name))
              , _id (id)
              , _change_manager (change_manager)
              , _origin (origin)
              , _handle (handle)
              , _set_x_action ( new action::meta_set_property<handle_type>
                                ( "set_transition_property_action"
                                , change_manager, origin, handle
                                , "fhg.pnete.position.x"
                                , to_property_type (position.x())
                                )
                              )
              , _set_y_action ( new action::meta_set_property<handle_type>
                                ( "set_transition_property_action"
                                , change_manager, origin, handle
                                , "fhg.pnete.position.y"
                                , to_property_type (position.y())
                                )
                              )
          { }

          virtual void undo()
          {
            _set_x_action->undo();
            _set_y_action->undo();
          }

          virtual void redo()
          {
            _set_x_action->redo();
            _set_y_action->redo();
          }

          virtual int id() const
          {
            return _id;
          }

          virtual bool mergeWith (const QUndoCommand* other_)
          {
            if (id() == other_->id())
            {
              const meta_move_item<handle_type>* other
                (static_cast<const meta_move_item<handle_type>*> (other_));
              if (_handle == other->_handle)
              {
                _set_x_action->new_value (other->_set_x_action->new_value());
                _set_y_action->new_value (other->_set_y_action->new_value());
                return true;
              }
            }

            return false;
          }

        private:
          int _id;
          change_manager_t& _change_manager;
          const QObject* _origin;
          const handle_type _handle;
          boost::scoped_ptr<meta_set_property<handle_type> > _set_x_action;
          boost::scoped_ptr<meta_set_property<handle_type> > _set_y_action;
        };

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
              ( &signal::transition_deleted
              , NULL
              , handle::transition (_transition, _change_manager)
              );

            _net.get_ref().erase_transition (_transition);
          }

          virtual void redo()
          {
            _net.get_ref().push_transition (_transition);

            _change_manager.emit_signal
              ( &signal::transition_added
              , _origin
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
              ( &signal::transition_added
              , NULL
              , handle::transition (_transition, _change_manager)
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &signal::transition_deleted
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
              ( &signal::place_deleted
              , NULL
              , handle::place (_place, _change_manager)
              );

            _net.get_ref().erase_place (_place);
          }

          virtual void redo()
          {
            _net.get_ref().push_place (_place);

            _change_manager.emit_signal
              ( &signal::place_added
              , _origin
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
              ( &signal::place_added
              , NULL
              , handle::place (_place, _change_manager)
              );
          }

          virtual void redo()
          {
            _change_manager.emit_signal
              ( &signal::place_deleted
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
        void set_function_name_impl
          ( ACTION_ARG_LIST
          , const data::handle::function& function
          , const boost::optional<std::string>& name
          )
        {
          function.get_ref().name (name);

          change_manager.emit_signal
            ( &signal::function_name_changed
            , origin
            , function
            , QString::fromStdString (name.get_value_or (""))
            );
        }

        class set_function_name : public QUndoCommand
        {
        public:
          set_function_name
            ( ACTION_ARG_LIST
            , const handle::function& function
            , const QString& name
            )
              : ACTION_INIT ("set_function_name")
              , _function (function)
              , _old_name (function.get().name())
              , _new_name
                (boost::make_optional (!name.isEmpty(), name.toStdString()))
          { }

          virtual void undo()
          {
            set_function_name_impl
              (_change_manager, NULL, _function, _old_name);
          }

          virtual void redo()
          {
            set_function_name_impl
              (_change_manager, _origin, _function, _new_name);
            _origin = NULL;
          }

        private:
          ACTION_MEMBERS;
          const data::handle::function _function;
          const boost::optional<std::string> _old_name;
          const boost::optional<std::string> _new_name;
        };

        // - expression ----------------------------------------------
        void set_expression_content_impl
          ( ACTION_ARG_LIST
          , const data::handle::expression& expression
          , const std::string& content
          )
        {
          expression.get_ref().set (content);

          change_manager.emit_signal
            ( &signal::signal_set_expression
            , origin
            , expression
            , QString::fromStdString (content)
            );

          change_manager.emit_signal
            ( &signal::signal_set_expression_parse_result
            , origin
            , expression
            , QString::fromStdString (expr::parse::parse_result (content))
            );
        }

        class set_expression_content : public QUndoCommand
        {
        public:
          set_expression_content
            ( ACTION_ARG_LIST
            , const handle::expression& expression
            , const QString& new_content
            )
              : ACTION_INIT ("set_expression_content")
              , _expression (expression)
              , _old_content (expression.get().expression())
              , _new_content (new_content.toStdString())
          { }

          virtual void undo()
          {
            set_expression_content_impl
              (_change_manager, NULL, _expression, _old_content);
          }

          virtual void redo()
          {
            set_expression_content_impl
              (_change_manager, _origin, _expression, _new_content);
            _origin = NULL;
          }

        private:
          ACTION_MEMBERS;
          const data::handle::expression _expression;
          const std::string _old_content;
          const std::string _new_content;
        };

#undef ACTION_ARG_LIST
#undef ACTION_INIT
#undef ACTION_MEMBERS

      }

      // ## editing methods ##########################################
      // - net -------------------------------------------------------
      // -- connection -----------------------------------------------
      void change_manager_t::set_property
        ( const QObject* origin
        , const data::handle::connect& connect
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::connect>
               ( "set_connect_property_action"
               , *this, origin, connect, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const QObject* origin
        , const data::handle::connect& connect
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (connect, key, val, *this, origin);
      }

      // -- transition -----------------------------------------------
      void change_manager_t::add_transition
        ( const QObject* origin
        , const ::xml::parse::id::ref::function& fun
        , const handle::net& net
        )
      {
        const ::xml::parse::id::ref::transition transition
          ( ::xml::parse::type::transition_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , net.id().id()
            , fun
            ).make_reference_id()
          );

        std::string name (fun.get().name() ? *fun.get().name() : "transition");
        //! \todo Don't check for duplicate names when fun.name is set?
        while (net.get().has_transition (name))
        {
          name = inc (name);
        }
        transition.get_ref().name (name);

        push (new action::add_transition (*this, origin, net.id(), transition));
      }

      void change_manager_t::add_transition
        ( const QObject* origin
        , const handle::net& net
        )
      {
        const ::xml::parse::id::function function_id
          (net.id().id_mapper()->next_id());
        const ::xml::parse::id::transition transition_id
          (net.id().id_mapper()->next_id());

        const ::xml::parse::id::ref::transition transition
          ( ::xml::parse::type::transition_type
            ( transition_id
            , net.id().id_mapper()
            , net.id().id()
            , ::xml::parse::id::ref::function
              ( ::xml::parse::type::function_type
                ( function_id
                , net.id().id_mapper()
                , ::xml::parse::type::function_type::make_parent (transition_id)
                , ::xml::parse::type::expression_type
                  ( net.id().id_mapper()->next_id()
                  , net.id().id_mapper()
                  , function_id
                  ).make_reference_id()
                ).make_reference_id()
              )
            ).make_reference_id()
          );

        std::string name ("transition");
        //! \todo Don't check for duplicate names when fun.name is set?
        while (net.get().has_transition (name))
        {
          name = inc (name);
        }
        transition.get_ref().name (name);

        push (new action::add_transition (*this, origin, net.id(), transition));
      }

      void change_manager_t::delete_transition
        ( const QObject* origin
        , const handle::transition& transition
        )
      {
        push (new action::remove_transition (*this, origin, transition.id()));
      }

      void change_manager_t::set_property
        ( const QObject* origin
        , const data::handle::transition& transition
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::transition>
               ( "set_transition_property_action"
               , *this, origin, transition, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const QObject* origin
        , const data::handle::transition& transition
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (transition, key, val, *this, origin);
      }

      void change_manager_t::move_item ( const QObject* origin
                                       , const handle::transition& transition
                                       , const QPointF& position
                                       )
      {
        push ( new action::meta_move_item<handle::transition>
               ( "move_transition_item_action", ids::move_transition_item
               , *this, origin, transition, position
               )
             );
      }

      void change_manager_t::no_undo_move_item
        ( const QObject* origin
        , const handle::transition& transition
        , const QPointF& position
        )
      {
         action::set_property ( transition
                              , "fhg.pnete.position.x"
                              , action::to_property_type (position.x())
                              , *this
                              , origin
                              );
         action::set_property ( transition
                              , "fhg.pnete.position.y"
                              , action::to_property_type (position.y())
                              , *this
                              , origin
                              );
      }


      // -- place ----------------------------------------------------
      void change_manager_t::add_place
        ( const QObject* origin
        , const handle::net& net
        )
      {
        const ::xml::parse::id::ref::place place
          ( ::xml::parse::type::place_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , net.id().id()
            ).make_reference_id()
          );

        std::string name ("place");
        while (net.get().has_place (name))
        {
          name = inc (name);
        }
        place.get_ref().name (name);

        push (new action::add_place (*this, origin, net.id(), place));
      }

      void change_manager_t::delete_place
        ( const QObject* origin
        , const handle::place& place
        )
      {
        push (new action::remove_place (*this, origin, place.id()));
      }

      void change_manager_t::set_property
        ( const QObject* origin
        , const data::handle::place& place
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::place>
               ( "set_place_property_action"
               , *this, origin, place, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const QObject* origin
        , const data::handle::place& place
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (place, key, val, *this, origin);
      }

      void change_manager_t::move_item ( const QObject* origin
                                       , const handle::place& place
                                       , const QPointF& position
                                       )
      {
        push ( new action::meta_move_item<handle::place>
               ( "move_place_item_action", ids::move_place_item
               , *this, origin, place, position
               )
             );
      }

      void change_manager_t::no_undo_move_item
        ( const QObject* origin
        , const handle::place& place
        , const QPointF& position
        )
      {
         action::set_property ( place
                              , "fhg.pnete.position.x"
                              , action::to_property_type (position.x())
                              , *this
                              , origin
                              );
         action::set_property ( place
                              , "fhg.pnete.position.y"
                              , action::to_property_type (position.y())
                              , *this
                              , origin
                              );
      }

      // - port ------------------------------------------------------
      void change_manager_t::set_property
        ( const QObject* origin
        , const data::handle::port& port
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::port>
               ( "set_port_property_action"
               , *this, origin, port, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const QObject* origin
        , const data::handle::port& port
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (port, key, val, *this, origin);
      }

      void change_manager_t::move_item ( const QObject* origin
                                       , const handle::port& port
                                       , const QPointF& position
                                       )
      {
        push ( new action::meta_move_item<handle::port>
               ( "move_port_item_action", ids::move_port_item
               , *this, origin, port, position
               )
             );
      }

      void change_manager_t::no_undo_move_item
        ( const QObject* origin
        , const handle::port& port
        , const QPointF& position
        )
      {
         action::set_property ( port
                              , "fhg.pnete.position.x"
                              , action::to_property_type (position.x())
                              , *this
                              , origin
                              );
         action::set_property ( port
                              , "fhg.pnete.position.y"
                              , action::to_property_type (position.y())
                              , *this
                              , origin
                              );
      }

      // - function --------------------------------------------------
      void change_manager_t::set_function_name
        ( const QObject* origin
        , const data::handle::function& fun
        , const QString& name
        )
      {
        push (new action::set_function_name (*this, origin, fun, name));
      }

      void change_manager_t::set_property
        ( const QObject* origin
        , const data::handle::function& function
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::function>
               ( "set_function_property_action"
               , *this, origin, function, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const QObject* origin
        , const data::handle::function& function
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (function, key, val, *this, origin);
      }

      // - expression ------------------------------------------------
      void change_manager_t::set_expression
        ( const QObject* origin
        , data::handle::expression& expression
        , const QString& text
        )
      {
        push ( new action::set_expression_content
               (*this, origin, expression, text)
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
