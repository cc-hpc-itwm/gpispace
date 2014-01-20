// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/expression.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/place_map.hpp>
#include <pnete/data/handle/port.hpp>
#include <pnete/data/handle/transition.hpp>

#include <fhg/util/read_bool.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/transition.hpp>

#include <xml/parse/util/position.hpp>

#include <we/expr/parse/parser.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/smart_ptr.hpp>

#include <QList>
#include <QPointF>

#define VARIADIC_SIZE(...) VARIADIC_SIZE_I(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,)
#define VARIADIC_SIZE_I(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, e34, e35, e36, e37, e38, e39, e40, e41, e42, e43, e44, e45, e46, e47, e48, e49, e50, e51, e52, e53, e54, e55, e56, e57, e58, e59, e60, e61, e62, e63, size, ...) size

#define EMIT_SIGNAL_CALL_NAME___(x) change_manager.emit_signal ## x
#define EMIT_SIGNAL_CALL_NAME__(x) EMIT_SIGNAL_CALL_NAME___ (x)
#define EMIT_SIGNAL_CALL_NAME_(x) EMIT_SIGNAL_CALL_NAME__ (x)
#define EMIT_SIGNAL_CALL_NAME(...)                      \
  EMIT_SIGNAL_CALL_NAME_ (VARIADIC_SIZE (__VA_ARGS__))

#define EMIT_SIGNAL(SIGNAL,...)                             \
  EMIT_SIGNAL_CALL_NAME (__VA_ARGS__) (SIGNAL,__VA_ARGS__)

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
          EXPOSE (connection_added);
          EXPOSE (connection_removed);
          EXPOSE (connection_direction_changed);
          EXPOSE (property_changed);

          // -- place_map ----------------------------------------------
          EXPOSE (place_map_added);
          EXPOSE (place_map_removed);

          // -- transition ---------------------------------------------
          EXPOSE (transition_added);
          EXPOSE (transition_deleted);
          EXPOSE (name_set);

          // -- place --------------------------------------------------
          EXPOSE (place_added);
          EXPOSE (place_deleted);
          EXPOSE (place_is_virtual_changed);
          EXPOSE (type_set);

          // - port ----------------------------------------------------
          EXPOSE (port_added);
          EXPOSE (port_deleted);
          EXPOSE (place_association_set);

          // - function ------------------------------------------------
          EXPOSE (function_name_changed);

          // - expression ---------------------------------------------
          EXPOSE (signal_set_expression);
          EXPOSE (signal_set_expression_parse_result);

#undef EXPOSE
        };

        std::string next (const std::string& s)
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

        template<typename NET_TYPE> std::string unique_name_for_place
          (const NET_TYPE& net, std::string prefix = "place")
        {
          while (net.get().has_place (prefix))
          {
            prefix = next (prefix);
          }
          return prefix;
        }

        template<typename NET_TYPE> std::string unique_name_for_transition
          (const NET_TYPE& net, std::string prefix = "transition")
        {
          while (net.get().has_transition (prefix))
          {
            prefix = next (prefix);
          }
          return prefix;
        }

        template<typename FUNCTION_TYPE> std::string unique_name_for_port
          ( const FUNCTION_TYPE& function
          , const we::type::PortDirection& direction
          , std::string prefix = "port"
          )
        {
          //! \note If we add a in-port with type t and a out-port with
          //! type u, it would find the same name for them, but will
          //! fail on inserting, as in- and out-ports with same name
          //! need same type.
          if (direction == we::type::PORT_IN || direction == we::type::PORT_OUT)
          {
            while (  function.get().ports().has
                      (std::make_pair (prefix, we::type::PORT_IN))
                  || function.get().ports().has
                      (std::make_pair (prefix, we::type::PORT_OUT))
                  )
            {
              prefix = next (prefix);
            }
          }
          else
          {
            while (function.get().ports().has (std::make_pair (prefix, direction)))
            {
              prefix = next (prefix);
            }
          }
          return prefix;
        }

        template<typename FUNCTION_TYPE>
          std::string unique_name_for_tunnel_port_and_place
            (const FUNCTION_TYPE& function, std::string prefix = "tunnel")
        {
          while ( function.get().ports().has
                  (std::make_pair (prefix, we::type::PORT_TUNNEL))
                || function.get().get_net()->get().has_place (prefix)
                )
          {
            prefix = next (prefix);
          }
          return prefix;
        }
      }

      namespace action
      {
#define ACTION_ARG_LIST                             \
              change_manager_t& change_manager      \
            , internal_type* document

#define ACTION_INIT(NAME)                           \
                QUndoCommand (NAME)                 \
              , _change_manager (change_manager)    \
              , _document (document)

#define ACTION_MEMBERS                              \
          change_manager_t& _change_manager;        \
          internal_type* _document;

#define ACTION_IMPL_ARGS                        \
        _change_manager, _document

#define ACTION_CTOR_ARGS                        \
        change_manager, document


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
          void set_property ( ACTION_ARG_LIST
                            , const HANDLE_TYPE& handle
                            , const ::we::type::property::key_type& key
                            , const ::we::type::property::value_type& val
                            )
        {
          typedef HANDLE_TYPE handle_type;
          typedef void (change_manager_t::* signal_type)
            ( const handle_type&
            , const ::we::type::property::key_type&
            , const ::we::type::property::value_type&
            );

          handle.get_ref().properties().set (key, val);
          EMIT_SIGNAL ( static_cast<signal_type> (&signal::property_changed)
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
            ( const QString& name
            , ACTION_ARG_LIST
            , const handle_type& handle
            , const ::we::type::property::key_type& key
            , const ::we::type::property::value_type& val
            )
              : ACTION_INIT (name)
              , _handle (handle)
              , _key (key)
              , _new_value (val)
              , _old_value (*handle.get().properties().get (key))
          { }

          virtual void undo()
          {
            set_property (ACTION_IMPL_ARGS, _handle, _key, _old_value);
          }

          virtual void redo()
          {
            set_property (ACTION_IMPL_ARGS, _handle, _key, _new_value);
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
          ACTION_MEMBERS;
          const handle_type _handle;
          const ::we::type::property::key_type _key;
          ::we::type::property::value_type _new_value;
          const ::we::type::property::value_type _old_value;
        };

        template<typename HANDLE_TYPE>
        void set_type_impl
          (ACTION_ARG_LIST, const HANDLE_TYPE& handle, const QString& type)
        {
          handle.get_ref().type (type.toStdString());

          typedef void (change_manager_t::* signal_type)
            (const HANDLE_TYPE&, const QString&);

          EMIT_SIGNAL
            (static_cast<signal_type> (&signal::type_set), handle, type);
        }

        template<typename HANDLE_TYPE>
        class meta_set_type : public QUndoCommand
        {
        public:
          meta_set_type
            ( const QString& name
            , ACTION_ARG_LIST
            , const HANDLE_TYPE& handle
            , const QString& type
            )
              : ACTION_INIT (name)
              , _handle (handle)
              , _old_type (QString::fromStdString (handle.get().type()))
              , _new_type (type)
          { }

          virtual void undo()
          {
            set_type_impl (ACTION_IMPL_ARGS, _handle, _old_type);
          }

          virtual void redo()
          {
            set_type_impl (ACTION_IMPL_ARGS, _handle, _new_type);
          }

        private:
          ACTION_MEMBERS;
          const HANDLE_TYPE _handle;
          const QString _old_type;
          const QString _new_type;
        };

        template<typename HANDLE_TYPE>
        void set_name_impl
          (ACTION_ARG_LIST, const HANDLE_TYPE& handle, const QString& name)
        {
          handle.get_ref().name (name.toStdString());

          typedef void (change_manager_t::* signal_type)
            (const HANDLE_TYPE&, const QString&);

          EMIT_SIGNAL
            (static_cast<signal_type> (&signal::name_set), handle, name);
        }

        template<typename HANDLE_TYPE>
        class meta_set_name : public QUndoCommand
        {
        public:
          meta_set_name
            ( const QString& action_name
            , ACTION_ARG_LIST
            , const HANDLE_TYPE& handle
            , const QString& name
            )
              : ACTION_INIT (action_name)
              , _handle (handle)
              , _old_name (QString::fromStdString (handle.get().name()))
              , _new_name (name)
          { }

          virtual void undo()
          {
            set_name_impl (ACTION_IMPL_ARGS, _handle, _old_name);
          }

          virtual void redo()
          {
            set_name_impl (ACTION_IMPL_ARGS, _handle, _new_name);
          }

        private:
          ACTION_MEMBERS;
          const HANDLE_TYPE _handle;
          const QString _old_name;
          const QString _new_name;
        };

        // - net -----------------------------------------------------
        template<typename HANDLE_TYPE>
          class meta_move_item : public QUndoCommand
        {
        private:
          typedef HANDLE_TYPE handle_type;

        public:
          meta_move_item
            ( const QString& name
            , int id
            , ACTION_ARG_LIST
            , const handle_type& handle
            , const QPointF& position
            , const bool outer
            )
              : ACTION_INIT (name)
              , _id (id)
              , _handle (handle)
              , _outer (outer)
              , _set_x_action ( new action::meta_set_property<handle_type>
                                ( QObject::tr ("set_transition_property_action")
                                , ACTION_CTOR_ARGS, handle
                                , !_outer
                                ? "fhg.pnete.position.x"
                                : "fhg.pnete.outer_position.x"
                                , to_property_type (position.x())
                                )
                              )
              , _set_y_action ( new action::meta_set_property<handle_type>
                                ( QObject::tr ("set_transition_property_action")
                                , ACTION_CTOR_ARGS, handle
                                , !_outer
                                ? "fhg.pnete.position.y"
                                : "fhg.pnete.outer_position.y"
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
              if (_handle == other->_handle && _outer == other->_outer)
              {
                _set_x_action->new_value (other->_set_x_action->new_value());
                _set_y_action->new_value (other->_set_y_action->new_value());
                return true;
              }
            }

            return false;
          }

        private:
          ACTION_MEMBERS;
          int _id;
          const handle_type _handle;
          const bool _outer;
          boost::scoped_ptr<meta_set_property<handle_type> > _set_x_action;
          boost::scoped_ptr<meta_set_property<handle_type> > _set_y_action;
        };

        // -- connection ---------------------------------------------

        void remove_connection_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::transition& transition
          , const ::xml::parse::id::ref::connect& connect
          )
        {
          EMIT_SIGNAL
            (&signal::connection_removed, handle::connect (connect, document));

          transition.get_ref().remove_connection (connect);
        }

        void add_connection_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::transition& transition
          , const ::xml::parse::id::ref::connect& connection
          )
        {
          transition.get_ref().push_connection (connection);

          EMIT_SIGNAL
            ( &signal::connection_added
            , handle::connect (connection, document)
            , handle::place (*connection.get().resolved_place(), document)
            , handle::port (*connection.get().resolved_port(), document)
            );
        }

        class add_connection : public QUndoCommand
        {
        public:
          add_connection
            ( ACTION_ARG_LIST
            , const ::xml::parse::id::ref::transition& transition
            , const ::xml::parse::id::ref::connect& connect
            )
              : ACTION_INIT (QObject::tr ("add_connection_action"))
              , _transition (transition)
              , _connect (connect)
          { }

          virtual void undo()
          {
            remove_connection_impl (ACTION_IMPL_ARGS, _transition, _connect);
          }

          virtual void redo()
          {
            add_connection_impl (ACTION_IMPL_ARGS, _transition, _connect);
          }

        private:
          ACTION_MEMBERS;
          const ::xml::parse::id::ref::transition _transition;
          const ::xml::parse::id::ref::connect _connect;
        };

        class remove_connection : public QUndoCommand
        {
        public:
          remove_connection
            (ACTION_ARG_LIST, const ::xml::parse::id::ref::connect& connect)
              : ACTION_INIT (QObject::tr ("remove_connection_action"))
              , _connect (connect)
              , _transition (_connect.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            add_connection_impl (ACTION_IMPL_ARGS, _transition, _connect);
          }

          virtual void redo()
          {
            remove_connection_impl (ACTION_IMPL_ARGS, _transition, _connect);
          }

        private:
          ACTION_MEMBERS;
          const ::xml::parse::id::ref::connect _connect;
          const ::xml::parse::id::ref::transition _transition;
        };

        void connection_is_read_impl
          (ACTION_ARG_LIST, const handle::connect& connect, bool read)
        {
          connect.get_ref().direction ( read
                                      ? we::edge::PT_READ
                                      : we::edge::PT
                                      );
          EMIT_SIGNAL (&signal::connection_direction_changed, connect);
        }

        class connection_is_read : public QUndoCommand
        {
        public:
          connection_is_read
            (ACTION_ARG_LIST, const handle::connect& connect, bool read)
              : ACTION_INIT (QObject::tr ("connection_is_read_action"))
              , _connect (connect)
              , _old_value (connect.is_read())
              , _new_value (read)
          { }

          virtual void undo()
          {
            connection_is_read_impl (ACTION_IMPL_ARGS, _connect, _old_value);
          }

          virtual void redo()
          {
            connection_is_read_impl (ACTION_IMPL_ARGS, _connect, _new_value);
          }

        private:
          ACTION_MEMBERS;
          const handle::connect _connect;
          const bool _old_value;
          const bool _new_value;
        };

        // -- place_map ----------------------------------------------
        void remove_place_map_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::transition& transition
          , const ::xml::parse::id::ref::place_map& place_map
          )
        {
          EMIT_SIGNAL ( &signal::place_map_removed
                      , handle::place_map (place_map, document)
                      );

          transition.get_ref().remove_place_map (place_map);
        }

        void add_place_map_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::transition& transition
          , const ::xml::parse::id::ref::place_map& place_map
          )
        {
          transition.get_ref().push_place_map (place_map);

          EMIT_SIGNAL ( &signal::place_map_added
                      , handle::place_map (place_map, document)
                      );
        }

        class add_place_map : public QUndoCommand
        {
        public:
          add_place_map
            ( ACTION_ARG_LIST
            , const ::xml::parse::id::ref::transition& transition
            , const ::xml::parse::id::ref::place_map& place_map
            )
              : ACTION_INIT (QObject::tr ("add_place_map_action"))
              , _transition (transition)
              , _place_map (place_map)
          { }

          virtual void undo()
          {
            remove_place_map_impl (ACTION_IMPL_ARGS, _transition, _place_map);
          }

          virtual void redo()
          {
            add_place_map_impl (ACTION_IMPL_ARGS, _transition, _place_map);
          }

        private:
          ACTION_MEMBERS;
          const ::xml::parse::id::ref::transition _transition;
          const ::xml::parse::id::ref::place_map _place_map;
        };

        class remove_place_map : public QUndoCommand
        {
        public:
          remove_place_map
            (ACTION_ARG_LIST, const ::xml::parse::id::ref::place_map& place_map)
              : ACTION_INIT (QObject::tr ("remove_place_map_action"))
              , _place_map (place_map)
              , _transition (_place_map.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            add_place_map_impl (ACTION_IMPL_ARGS, _transition, _place_map);
          }

          virtual void redo()
          {
            remove_place_map_impl (ACTION_IMPL_ARGS, _transition, _place_map);
          }

        private:
          ACTION_MEMBERS;
          const ::xml::parse::id::ref::place_map _place_map;
          const ::xml::parse::id::ref::transition _transition;
        };

        // -- transition ---------------------------------------------
        void remove_transition_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::net& net
          , const ::xml::parse::id::ref::transition& transition
          )
        {
          EMIT_SIGNAL ( &signal::transition_deleted
                      , handle::transition (transition, document)
                      );

          net.get_ref().erase_transition (transition);
        }

        void add_transition_impl
          ( ACTION_ARG_LIST
          , const ::xml::parse::id::ref::net& net
          , const ::xml::parse::id::ref::transition& transition
          )
        {
          net.get_ref().push_transition (transition);

          EMIT_SIGNAL ( &signal::transition_added
                      , handle::transition (transition, document)
                      );
        }

        class add_transition : public QUndoCommand
        {
        public:
          add_transition
            ( ACTION_ARG_LIST
            , const ::xml::parse::id::ref::net& net
            , const ::xml::parse::id::ref::transition& transition
            )
              : ACTION_INIT (QObject::tr ("add_transition_action"))
              , _transition (transition)
              , _net (net)
          { }

          virtual void undo()
          {
            remove_transition_impl (ACTION_IMPL_ARGS, _net, _transition);
          }

          virtual void redo()
          {
            add_transition_impl (ACTION_IMPL_ARGS, _net, _transition);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::transition _transition;
          ::xml::parse::id::ref::net _net;
        };

        class remove_transition : public QUndoCommand
        {
        public:
          remove_transition ( ACTION_ARG_LIST
                            , const ::xml::parse::id::ref::transition& transition
                            )
            : ACTION_INIT (QObject::tr ("remove_transition_action"))
            , _transition (transition)
            , _net (_transition.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            add_transition_impl (ACTION_IMPL_ARGS, _net, _transition);
          }

          virtual void redo()
          {
            remove_transition_impl (ACTION_IMPL_ARGS, _net, _transition);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::transition _transition;
          ::xml::parse::id::ref::net _net;
        };

        // -- place --------------------------------------------------
        void add_place_impl ( ACTION_ARG_LIST
                            , const ::xml::parse::id::ref::net& net
                            , const ::xml::parse::id::ref::place& place
                            )
        {
          net.get_ref().push_place (place);

          EMIT_SIGNAL (&signal::place_added, handle::place (place, document));
        }

        void remove_place_impl ( ACTION_ARG_LIST
                               , const ::xml::parse::id::ref::net& net
                               , const ::xml::parse::id::ref::place& place
                               )
        {
          EMIT_SIGNAL (&signal::place_deleted, handle::place (place, document));

          net.get_ref().erase_place (place);
        }

        class add_place : public QUndoCommand
        {
        public:
          add_place ( ACTION_ARG_LIST
                    , const ::xml::parse::id::ref::net& net
                    , const ::xml::parse::id::ref::place& place
                    )
            : ACTION_INIT (QObject::tr ("add_place_action"))
            , _place (place)
            , _net (net)
          { }

          virtual void undo()
          {
            remove_place_impl (ACTION_IMPL_ARGS, _net, _place);
          }

          virtual void redo()
          {
            add_place_impl (ACTION_IMPL_ARGS, _net, _place);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::place _place;
          ::xml::parse::id::ref::net _net;
        };

        class remove_place : public QUndoCommand
        {
        public:
          remove_place
            (ACTION_ARG_LIST, const ::xml::parse::id::ref::place& place)
              : ACTION_INIT (QObject::tr ("remove_place_action"))
              , _place (place)
              , _net (_place.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            add_place_impl (ACTION_IMPL_ARGS, _net, _place);
          }

          virtual void redo()
          {
            remove_place_impl (ACTION_IMPL_ARGS, _net, _place);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::place _place;
          ::xml::parse::id::ref::net _net;
        };

        void place_set_virtual_impl
          (ACTION_ARG_LIST, const ::xml::parse::id::ref::place& place, bool val)
        {
          place.get_ref().set_virtual (val);

          EMIT_SIGNAL ( &signal::place_is_virtual_changed
                      , handle::place (place, document)
                      , val
                      );
        }

        class place_set_virtual : public QUndoCommand
        {
        public:
          place_set_virtual ( ACTION_ARG_LIST
                            , const ::xml::parse::id::ref::place& place
                            , bool new_value
                            )
            : ACTION_INIT (QObject::tr ("place_set_virtual_action"))
            , _place (place)
            , _old_value (_place.get().is_virtual())
            , _new_value (new_value)
          { }

          virtual void undo()
          {
            place_set_virtual_impl (ACTION_IMPL_ARGS, _place, _old_value);
          }

          virtual void redo()
          {
            place_set_virtual_impl (ACTION_IMPL_ARGS, _place, _new_value);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::place _place;
          bool _old_value;
          bool _new_value;
        };

        // -- port --------------------------------------------------
        void add_port_impl ( ACTION_ARG_LIST
                           , const ::xml::parse::id::ref::function& function
                           , const ::xml::parse::id::ref::port& port
                           )
        {
          function.get_ref().push_port (port);

          EMIT_SIGNAL (&signal::port_added, handle::port (port, document));
        }

        void remove_port_impl ( ACTION_ARG_LIST
                              , const ::xml::parse::id::ref::function& function
                              , const ::xml::parse::id::ref::port& port
                              )
        {
          EMIT_SIGNAL (&signal::port_deleted, handle::port (port, document));

          if (function.get().ports().has (port))
          {
            function.get_ref().remove_port (port);
          }
          else
          {
            throw std::runtime_error ("No port with that id in that function.");
          }
        }

        class add_port : public QUndoCommand
        {
        public:
          add_port ( ACTION_ARG_LIST
                   , const ::xml::parse::id::ref::function& function
                   , const ::xml::parse::id::ref::port& port
                   )
            : ACTION_INIT (QObject::tr ("add_port_action"))
            , _port (port)
            , _function (function)
          { }

          virtual void undo()
          {
            remove_port_impl (ACTION_IMPL_ARGS, _function, _port);
          }

          virtual void redo()
          {
            add_port_impl (ACTION_IMPL_ARGS, _function, _port);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::port _port;
          ::xml::parse::id::ref::function _function;
        };

        class remove_port : public QUndoCommand
        {
        public:
          remove_port (ACTION_ARG_LIST, const ::xml::parse::id::ref::port& port)
            : ACTION_INIT (QObject::tr ("remove_port_action"))
            , _port (port)
            , _function (_port.get().parent()->make_reference_id())
          { }

          virtual void undo()
          {
            add_port_impl (ACTION_IMPL_ARGS, _function, _port);
          }

          virtual void redo()
          {
            remove_port_impl (ACTION_IMPL_ARGS, _function, _port);
          }

        private:
          ACTION_MEMBERS;
          ::xml::parse::id::ref::port _port;
          ::xml::parse::id::ref::function _function;
        };

        void set_place_association_impl
          ( ACTION_ARG_LIST
          , const handle::port& port
          , const boost::optional<std::string>& place
          )
        {
          port.get_ref().place = place;
          EMIT_SIGNAL (&signal::place_association_set, port, place);
        }

        class set_place_association : public QUndoCommand
        {
        public:
          set_place_association
            ( ACTION_ARG_LIST
            , const handle::port& port
            , const boost::optional<std::string>& place
            )
              : ACTION_INIT (QObject::tr ("set_place_association_action"))
              , _port (port)
              , _old_place (port.get().place)
              , _new_place (place)
          { }

          virtual void undo()
          {
            set_place_association_impl (ACTION_IMPL_ARGS, _port, _old_place);
          }

          virtual void redo()
          {
            set_place_association_impl (ACTION_IMPL_ARGS, _port, _new_place);
          }

        private:
          ACTION_MEMBERS;
          const handle::port _port;
          const boost::optional<std::string> _old_place;
          const boost::optional<std::string> _new_place;
        };

        // - function ------------------------------------------------
        void set_function_name_impl
          ( ACTION_ARG_LIST
          , const data::handle::function& function
          , const boost::optional<std::string>& name
          )
        {
          function.get_ref().name (name);

          EMIT_SIGNAL
            ( &signal::function_name_changed
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
              : ACTION_INIT (QObject::tr ("set_function_name_action"))
              , _function (function)
              , _old_name (function.get().name())
              , _new_name
                (boost::make_optional (!name.isEmpty(), name.toStdString()))
          { }

          virtual void undo()
          {
            set_function_name_impl (ACTION_IMPL_ARGS, _function, _old_name);
          }

          virtual void redo()
          {
            set_function_name_impl (ACTION_IMPL_ARGS, _function, _new_name);
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

          EMIT_SIGNAL
            ( &signal::signal_set_expression
            , expression
            , QString::fromStdString (content)
            );

          EMIT_SIGNAL
            ( &signal::signal_set_expression_parse_result
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
              : ACTION_INIT (QObject::tr ("set_expression_content_action"))
              , _expression (expression)
              , _old_content (expression.get().expression())
              , _new_content (new_content.toStdString())
          { }

          virtual void undo()
          {
            set_expression_content_impl
              (ACTION_IMPL_ARGS, _expression, _old_content);
          }

          virtual void redo()
          {
            set_expression_content_impl
              (ACTION_IMPL_ARGS, _expression, _new_content);
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
#undef ACTION_IMPL_ARGS
#undef ACTION_CTOR_ARGS

      }

#define ACTION_CTOR_ARGS(HANDLE) *this, HANDLE.document()

      namespace
      {
        template<typename ID>
          bool is_hard_hidden (const ID& id)
        {
          return fhg::util::read_bool
            ( id.get().properties().get ("fhg.pnete.is_hard_hidden")
            .get_value_or ("false")
            );
        }

        template<typename A, typename B>
        we::type::property::type maybe_hard_hidden (const A& a, const B& b)
        {
          we::type::property::type prop;
          if (is_hard_hidden (a) || is_hard_hidden (b))
          {
            prop.set ("fhg.pnete.is_hard_hidden", "true");
          }
          return prop;
        }
      }

      // ## editing methods ##########################################
      // - net -------------------------------------------------------
      // -- connection -----------------------------------------------
      void change_manager_t::add_connection ( const data::handle::place& place
                                            , const data::handle::port& port
                                            , bool no_make_explicit
                                            )
      {
        const ::xml::parse::id::ref::net net_of_place
          (place.get().parent()->make_reference_id());

        const ::xml::parse::id::ref::function function_of_net_of_place
          (net_of_place.get().parent()->make_reference_id());

        const ::xml::parse::id::ref::function function_of_port
          (port.get().parent()->make_reference_id());

        if (function_of_net_of_place == function_of_port)
        {
          beginMacro (tr ("set_place_association_action"));

          if (place.is_implicit() && !no_make_explicit)
          {
            make_explicit (place);
          }

          set_place_association (port, place.get().name());

          endMacro();
        }
        else
        {
          const ::xml::parse::id::ref::transition transition_of_fun_of_port
            (*function_of_port.get().parent_transition());

          if (net_of_place == transition_of_fun_of_port.get().parent()->id())
          {
            if (port.is_tunnel())
            {
              beginMacro (tr ("add_place_map_action"));

              if (place.is_implicit() && !no_make_explicit)
              {
                make_explicit (place);
              }

              push ( new action::add_place_map
                     ( ACTION_CTOR_ARGS (place)
                     , transition_of_fun_of_port
                     , ::xml::parse::type::place_map_type
                       ( transition_of_fun_of_port.id_mapper()->next_id()
                       , transition_of_fun_of_port.id_mapper()
                       , boost::none
                       , XML_PARSE_UTIL_POSITION_GENERATED()
                       , port.get().name()
                       , place.get().name()
                       , maybe_hard_hidden (port, place)
                       ).make_reference_id()
                     )
                   );

              endMacro();
            }
            else
            {
              beginMacro (tr ("add_connection_action"));

              if (place.is_implicit() && !no_make_explicit)
              {
                make_explicit (place);
              }

              push ( new action::add_connection
                     ( ACTION_CTOR_ARGS (place)
                     , transition_of_fun_of_port
                     , ::xml::parse::type::connect_type
                       ( transition_of_fun_of_port.id_mapper()->next_id()
                       , transition_of_fun_of_port.id_mapper()
                       , boost::none
                       , XML_PARSE_UTIL_POSITION_GENERATED()
                       , place.get().name()
                       , port.get().name()
                       , port.is_input()
                       ? we::edge::PT
                       : we::edge::TP
                       , maybe_hard_hidden (port, place)
                       ).make_reference_id()
                     )
                   );

              endMacro();
            }
          }
          else
          {
            throw std::runtime_error
              ("unknown constellation for creating connection");
          }
        }
      }

      namespace
      {
        template<typename opt>
          opt any (const opt& a, const opt& b)
        {
          return a ? a : b;
        }
      }

      void change_manager_t::add_connection ( const data::handle::port& port_a
                                            , const data::handle::port& port_b
                                            , const data::handle::net& net
                                            , const boost::optional<std::string>& implicit_place_name
                                            )
      {
        //! \todo Check for ports being in that or in transitions of that net?

        if (port_a.get().signature() != port_b.get().signature())
        {
          throw std::runtime_error ("different types for connected ports");
        }

        beginMacro (tr ("add_connection_with_implicit_place_action"));

        const ::xml::parse::id::ref::place place
          ( ::xml::parse::type::place_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , boost::none
            , implicit_place_name.get_value_or
              (unique_name_for_place (net, "implicit"))
            , port_a.get().type()
            , std::list<xml::parse::type::place_type::token_type>()
            , maybe_hard_hidden (port_a, port_b)
            ).make_reference_id()
          );

        place.get_ref().properties().set
          ("fhg.pnete.is_implicit_place", "true");

        push (new action::add_place (ACTION_CTOR_ARGS (net), net.id(), place));

        handle::place place_handle (place, net.document());

        add_connection (place_handle, port_a, true);
        add_connection (place_handle, port_b, true);

        const boost::optional<std::string> port_a_slot_return
          (port_a.get().properties().get ("fhg.seislib.slot.return"));
        const boost::optional<std::string> port_b_slot_return
          (port_b.get().properties().get ("fhg.seislib.slot.return"));
        const boost::optional<std::string> port_a_slot_uses
          (port_a.get().properties().get ("fhg.seislib.slot.uses"));
        const boost::optional<std::string> port_b_slot_uses
          (port_b.get().properties().get ("fhg.seislib.slot.uses"));

        // one does, other does not
        if (  !port_a_slot_uses && !port_a_slot_return
           && !port_b_slot_uses && !port_b_slot_return
           )
        {
          // fine, do nothing
        }
        else if (  (port_a_slot_uses && port_b_slot_return)
                || (port_a_slot_return && port_b_slot_uses)
                )
        {
          const boost::optional<xml::parse::id::ref::port> a_equiv
            ( port_a_slot_uses
            ? any ( port_a.parent().get().ports().get
                    (std::make_pair (*port_a_slot_uses, we::type::PORT_TUNNEL))
                  , port_a.parent().get().ports().get
                    (std::make_pair (*port_a_slot_uses, we::type::PORT_IN))
                  )
            : port_a.parent().get().ports().get
              (std::make_pair (*port_a_slot_return, we::type::PORT_OUT))
            );
          const boost::optional<xml::parse::id::ref::port> b_equiv
            ( port_b_slot_uses
            ? any ( port_b.parent().get().ports().get
                    (std::make_pair (*port_b_slot_uses, we::type::PORT_TUNNEL))
                  , port_b.parent().get().ports().get
                    (std::make_pair (*port_b_slot_uses, we::type::PORT_IN))
                  )
            : port_b.parent().get().ports().get
              (std::make_pair (*port_b_slot_return, we::type::PORT_OUT))
            );

          if (!a_equiv || !b_equiv)
          {
            throw std::runtime_error ("given slot management ports do not exist");
          }

          const std::string place_name (unique_name_for_place (net, "slot_exc"));

          add_connection ( handle::port (*a_equiv, port_a.document())
                         , handle::port (*b_equiv, port_b.document())
                         , net
                         , place_name
                         );

          port_a.get_ref().properties().set
            ("fhg.seislib.slot.connected_via", place_name);
          port_b.get_ref().properties().set
            ("fhg.seislib.slot.connected_via", place_name);
        }
        else
        {
          throw std::runtime_error ("connecting ports where one does slot management while the other does not or more than one return+use given");
        }

        endMacro();
      }

      void change_manager_t::remove_connection (const handle::connect& connect)
      {
        beginMacro (tr ("remove_connection_action"));

        if (connect.get().resolved_port())
        {
          const boost::optional<std::string> connected_via
            ( connect.get().resolved_port()->get().properties().get
              ("fhg.seislib.slot.connected_via")
            );
          if (connected_via)
          {
            const boost::optional<xml::parse::id::ref::place> pl
              (connect.get().parent()->parent()->places().get (*connected_via));
            //! \note Might be the second connection which gets
            //! deleted from deleting the place, which then no longer
            //! has that place.
            if (pl)
            {
              delete_place (handle::place (*pl, connect.document()));
            }
            connect.get().resolved_port()->get_ref().properties().del
              ("fhg.seislib.slot.connected_via");
          }
        }

        if (connect.get().resolved_place())
        {
          static QList<handle::connect> recurse_check;
          if (!recurse_check.contains (connect))
          {
            recurse_check.append (connect);
            const handle::place place
              (*connect.get().resolved_place(), connect.document());
            delete_place (place);
            recurse_check.removeAll (connect);
          }
        }

        push ( new action::remove_connection ( ACTION_CTOR_ARGS (connect)
                                             , connect.id()
                                             )
             );

        endMacro();
      }

      void change_manager_t::connection_is_read
        (const data::handle::connect& connect, const bool& read)
      {
        if (connect.is_out())
        {
          throw std::runtime_error ("tried setting is_read on out-connection");
        }

        push ( new action::connection_is_read ( ACTION_CTOR_ARGS (connect)
                                              , connect
                                              , read
                                              )
             );
      }


      void change_manager_t::set_property
        ( const data::handle::connect& connect
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::connect>
               ( tr ("set_connect_property_action")
               , ACTION_CTOR_ARGS (connect), connect, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::connect& connect
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (ACTION_CTOR_ARGS (connect), connect, key, val);
      }

      // -- place_map -----------------------------------------------
      void change_manager_t::remove_place_map (const handle::place_map& place_map)
      {
        beginMacro ("remove_place_map");

        if (place_map.get().resolved_tunnel_port())
        {
          const boost::optional<std::string> connected_via
            ( place_map.get().resolved_tunnel_port()->get().properties().get
              ("fhg.seislib.slot.connected_via")
            );
          if (connected_via)
          {
            const boost::optional<xml::parse::id::ref::place> pl
              (place_map.get().parent()->parent()->places().get (*connected_via));
            //! \note Might be the second connection which gets
            //! deleted from deleting the place, which then no longer
            //! has that place.
            if (pl)
            {
              delete_place (handle::place (*pl, place_map.document()));
            }
            place_map.get().resolved_tunnel_port()->get_ref().properties().del
              ("fhg.seislib.slot.connected_via");
          }
        }

        push ( new action::remove_place_map
               (ACTION_CTOR_ARGS (place_map), place_map.id())
             );

        endMacro();
      }

      void change_manager_t::set_property
        ( const data::handle::place_map& place_map
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::place_map>
               ( tr ("set_place_map_property_action")
               , ACTION_CTOR_ARGS (place_map), place_map, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::place_map& place_map
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (ACTION_CTOR_ARGS (place_map), place_map, key, val);
      }

      // -- transition -----------------------------------------------
      void change_manager_t::add_transition
        ( const ::xml::parse::id::ref::function& fun
        , const handle::net& net
        , const boost::optional<QPointF>& position
        )
      {
        beginMacro ("add_transition_action");

        const ::xml::parse::id::ref::transition transition
          ( ::xml::parse::type::transition_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , fun
            ).make_reference_id()
          );

        transition.get_ref().name
          ( unique_name_for_transition
            (net, fun.get().name().get_value_or ("transition"))
          );

        no_undo_move_item ( handle::transition (transition, net.document())
                          , position.get_value_or (QPointF())
                          );

        push ( new action::add_transition ( ACTION_CTOR_ARGS (net)
                                          , net.id()
                                          , transition
                                          )
             );

        foreach ( const xml::parse::id::ref::port& port
                , fun.get().ports().ids()
                )
        {
          const boost::optional<std::string> place_name
            (port.get().properties().get ("fhg.pnete.auto-connect"));
          if (place_name)
          {
            boost::optional<xml::parse::id::ref::place> place
              (net.get().places().get (*place_name));
            if (place)
            {
              add_connection ( data::handle::place (*place, net.document())
                             , data::handle::port (port, net.document())
                             );
            }
            else
            {
              throw std::runtime_error
                (std::string ("auto-connect to unknown place") + *place_name);
            }
          }
        }

        endMacro();
      }

      void change_manager_t::add_transition
        (const handle::net& net, const boost::optional<QPointF>& position)
      {
        const ::xml::parse::id::ref::transition transition
          ( ::xml::parse::type::transition_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , ::xml::parse::type::function_type
              ( net.id().id_mapper()->next_id()
              , net.id().id_mapper()
              , boost::none
              , XML_PARSE_UTIL_POSITION_GENERATED()
              , ::xml::parse::type::expression_type
                ( net.id().id_mapper()->next_id()
                , net.id().id_mapper()
                , boost::none
                , XML_PARSE_UTIL_POSITION_GENERATED()
                ).make_reference_id()
              ).make_reference_id()
            ).make_reference_id()
          );

        transition.get_ref().name (unique_name_for_transition (net));

        no_undo_move_item ( handle::transition (transition, net.document())
                          , position.get_value_or (QPointF())
                          );

        push ( new action::add_transition ( ACTION_CTOR_ARGS (net)
                                          , net.id()
                                          , transition
                                          )
             );
      }

      void change_manager_t::delete_transition
        (const handle::transition& transition)
      {
        beginMacro (tr ("remove_transition_and_connections_action"));

        //! \note remove_connection will modify transition's
        //! connections, thus copy out of there first, then modify.
        boost::unordered_set< ::xml::parse::id::ref::connect>
          connections_to_delete (transition.get().connections().ids());
        boost::unordered_set< ::xml::parse::id::ref::place_map>
          place_maps_to_delete (transition.get().place_map().ids());

        BOOST_FOREACH ( const ::xml::parse::id::ref::connect& c
                      , connections_to_delete
                      )
        {
          remove_connection (handle::connect (c, transition.document()));
        }
        BOOST_FOREACH ( const ::xml::parse::id::ref::place_map& c
                      , place_maps_to_delete
                      )
        {
          remove_place_map (handle::place_map (c, transition.document()));
        }

        push ( new action::remove_transition ( ACTION_CTOR_ARGS (transition)
                                             , transition.id()
                                             )
             );

        endMacro();
      }

      void change_manager_t::set_name
        (const data::handle::transition& transition, const QString& name)
      {
        push ( new action::meta_set_name<handle::transition>
               ( tr ("transition_set_name_action")
               , ACTION_CTOR_ARGS (transition)
               , transition
               , name
               )
             );
      }

      void change_manager_t::set_property
        ( const data::handle::transition& transition
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::transition>
               ( tr ("set_transition_property_action")
               , ACTION_CTOR_ARGS (transition), transition, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::transition& transition
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property
          (ACTION_CTOR_ARGS (transition), transition, key, val);
      }

      void change_manager_t::move_item ( const handle::transition& transition
                                       , const QPointF& position
                                       , const bool outer
                                       )
      {
        push ( new action::meta_move_item<handle::transition>
               ( tr ("move_transition_item_action"), ids::move_transition_item
               , ACTION_CTOR_ARGS (transition), transition, position, outer
               )
             );
      }

      void change_manager_t::no_undo_move_item
        (const handle::transition& transition, const QPointF& position)
      {
        action::set_property ( ACTION_CTOR_ARGS (transition)
                             , transition
                             , "fhg.pnete.position.x"
                             , action::to_property_type (position.x())
                             );
        action::set_property ( ACTION_CTOR_ARGS (transition)
                             , transition
                             , "fhg.pnete.position.y"
                             , action::to_property_type (position.y())
                             );
      }

      // -- place ----------------------------------------------------
      void change_manager_t::add_place
        (const handle::net& net, const boost::optional<QPointF>& position)
      {
        const ::xml::parse::id::ref::place place
          ( ::xml::parse::type::place_type
            ( net.id().id_mapper()->next_id()
            , net.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , unique_name_for_place (net)
            //! \todo: default type to something useful?
            , ""
            , boost::none
            ).make_reference_id()
          );

        no_undo_move_item ( handle::place (place, net.document())
                          , position.get_value_or (QPointF())
                          );

        push (new action::add_place (ACTION_CTOR_ARGS (net), net.id(), place));
      }

      namespace
      {
        bool is_connected_to_place ( const ::xml::parse::id::ref::connect& id
                                   , const ::xml::parse::id::ref::place& place
                                   )
        {
          return id.get().resolved_place()
            && *id.get().resolved_place() == place;
        }

        bool is_mapping_place ( const ::xml::parse::id::ref::place_map& id
                              , const ::xml::parse::id::ref::place& place
                              )
        {
          return id.get().resolved_real_place()
            && *id.get().resolved_real_place() == place;
        }

        bool is_associated_with ( const ::xml::parse::id::ref::port& port
                                , const ::xml::parse::id::ref::place& place
                                )
        {
          //! \todo This only checks for the same name, but might fail
          //! with shadowing. (See is_connected_to_place().)
          return port.get().place
            && *port.get().place == place.get().name();
        }

        template<typename C, typename R>
          void throw_into_set (C& container, const R& range)
        {
          container.insert (boost::begin (range), boost::end (range));
        }
      }

      void change_manager_t::delete_place (const handle::place& place)
      {
        beginMacro (tr ("remove_place_and_connections_action"));

        if (place.get().has_parent())
        {
          const ::xml::parse::type::net_type& net (place.get().parent().get());

          boost::unordered_set< ::xml::parse::id::ref::connect>
            connections_to_delete;
          boost::unordered_set< ::xml::parse::id::ref::place_map>
            place_maps_to_delete;
          boost::unordered_set< ::xml::parse::id::ref::port> ports_to_delete;

          //! \note remove_connection will modify transition's
          //! connections, thus copy out of there first, then modify.
          BOOST_FOREACH ( const ::xml::parse::type::transition_type& trans
                        , net.transitions().values()
                        )
          {
            throw_into_set
              ( connections_to_delete
              , trans.connections().ids()
              | boost::adaptors::filtered
                (boost::bind (is_connected_to_place, _1, place.id()))
              );
            throw_into_set
              ( place_maps_to_delete
              , trans.place_map().ids()
              | boost::adaptors::filtered
                (boost::bind (is_mapping_place, _1, place.id()))
              );
          }

          if (net.has_parent())
          {
            BOOST_FOREACH ( const ::xml::parse::id::ref::port& port
                          , net.parent()->ports().ids()
                          | boost::adaptors::filtered
                            (boost::bind (is_associated_with, _1, place.id()))
                          )
            {
              handle::port handle (port, place.document());

              if (handle.is_tunnel())
              {
                ports_to_delete.insert (port);
              }
              else
              {
                set_place_association (handle, boost::none);
              }
            }
          }

          BOOST_FOREACH ( const ::xml::parse::id::ref::connect& c
                        , connections_to_delete
                        )
          {
            remove_connection (handle::connect (c, place.document()));
          }
          BOOST_FOREACH ( const ::xml::parse::id::ref::place_map& pm
                        , place_maps_to_delete
                        )
          {
            remove_place_map (handle::place_map (pm, place.document()));
          }
          BOOST_FOREACH (const ::xml::parse::id::ref::port& id, ports_to_delete)
          {
            delete_port (handle::port (id, place.document()));
          }
        }

        push (new action::remove_place (ACTION_CTOR_ARGS (place), place.id()));

        endMacro();
      }

      void change_manager_t::set_name
        (const data::handle::place& place, const QString& name)
      {
        beginMacro (tr ("place_set_name_action"));

        if ( place.is_virtual()
           && place.get().has_parent() && place.get().parent().get().has_parent()
           )
        {
          const handle::port port
            ( *place.get().parent().get().parent().get().ports().get
              (std::make_pair (place.get().name(), we::type::PORT_TUNNEL))
            , place.document()
            );

          push ( new action::meta_set_name<handle::port>
                 ( tr ("port_set_name_action")
                 , ACTION_CTOR_ARGS (port), port, name
                 )
               );

          set_place_association (port, name.toStdString());
        }

        push ( new action::meta_set_name<handle::place>
               ( tr ("place_set_name_action")
               , ACTION_CTOR_ARGS (place), place, name
               )
             );

        endMacro();
      }

      void change_manager_t::set_type
        (const data::handle::place& place, const QString& type)
      {
        beginMacro (tr ("place_set_type_action"));

        if ( place.is_virtual()
           && place.get().has_parent() && place.get().parent().get().has_parent()
           )
        {
          const handle::port port
            ( *place.get().parent().get().parent().get().ports().get
              (std::make_pair (place.get().name(), we::type::PORT_TUNNEL))
            , place.document()
            );

          push ( new action::meta_set_type<handle::port>
                 ( tr ("port_set_type_action")
                 , ACTION_CTOR_ARGS (port), port, type
                 )
               );
        }

        push ( new action::meta_set_type<handle::place>
               ( tr ("place_set_type_action")
               , ACTION_CTOR_ARGS (place), place, type
               )
             );

        endMacro();
      }

      void change_manager_t::make_explicit (const data::handle::place& place)
      {
        set_property (place, "fhg.pnete.is_implicit_place", "false");
      }

      void change_manager_t::make_virtual (const data::handle::place& p)
      {
        if (p.is_virtual())
        {
          return;
        }

        beginMacro (tr ("make_virtual_action"));

        const ::xml::parse::id::ref::port port
          ( ::xml::parse::type::port_type
            ( p.id().id_mapper()->next_id()
            , p.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , p.get().name()
            , p.get().type()
            , p.get().name()
            , we::type::PORT_TUNNEL
            ).make_reference_id()
          );

        no_undo_move_item (handle::port (port, p.document()), QPointF());

        push ( new action::add_port
               ( ACTION_CTOR_ARGS (p)
               , p.get().parent()->parent().get().make_reference_id()
               , port
               )
             );

        push (new action::place_set_virtual (ACTION_CTOR_ARGS (p), p.id(), true));

        endMacro();
      }
      void change_manager_t::make_real (const data::handle::place& p)
      {
        if (!p.is_virtual())
        {
          return;
        }

        beginMacro (tr ("make_real_action"));

        delete_port
          ( handle::port ( *p.get().parent()->parent().get().ports().get
                           (std::make_pair (p.get().name(), we::type::PORT_TUNNEL))
                         , p.document()
                         )
          );

        push (new action::place_set_virtual (ACTION_CTOR_ARGS (p), p.id(), false));

        endMacro();
      }

      void change_manager_t::set_property
        ( const data::handle::place& place
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::place>
               ( tr ("set_place_property_action")
               , ACTION_CTOR_ARGS (place), place, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::place& place
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (ACTION_CTOR_ARGS (place), place, key, val);
      }

      void change_manager_t::move_item ( const handle::place& place
                                       , const QPointF& position
                                       , const bool outer
                                       )
      {
        push ( new action::meta_move_item<handle::place>
               ( tr ("move_place_item_action"), ids::move_place_item
               , ACTION_CTOR_ARGS (place), place, position, outer
               )
             );
      }

      void change_manager_t::no_undo_move_item
        (const handle::place& place, const QPointF& position)
      {
        action::set_property ( ACTION_CTOR_ARGS (place)
                             , place
                             , "fhg.pnete.position.x"
                             , action::to_property_type (position.x())
                             );
        action::set_property ( ACTION_CTOR_ARGS (place)
                             ,  place
                             , "fhg.pnete.position.y"
                             , action::to_property_type (position.y())
                             );
      }

      // - port ------------------------------------------------------
      void change_manager_t::add_port
        ( const handle::function& function
        , const we::type::PortDirection& direction
        , const boost::optional<QPointF>& position
        )
      {
        beginMacro (tr ("add_port_action"));

        const std::string name
          ( direction == we::type::PORT_TUNNEL
          ? unique_name_for_tunnel_port_and_place (function)
          : unique_name_for_port (function, direction)
          );

        //! \todo Default type?
        const std::string type ("");

        if (direction == we::type::PORT_TUNNEL)
        {
          const ::xml::parse::id::ref::place place
            ( ::xml::parse::type::place_type
              ( function.id().id_mapper()->next_id()
              , function.id().id_mapper()
              , boost::none
              , XML_PARSE_UTIL_POSITION_GENERATED()
              , name
              , type
              , true
              ).make_reference_id()
            );

          no_undo_move_item ( handle::place (place, function.document())
                            , position.get_value_or (QPointF())
                            );

          push ( new action::add_place ( ACTION_CTOR_ARGS (function)
                                       , *function.get().get_net()
                                       , place
                                       )
               );
        }

        const ::xml::parse::id::ref::port port
          ( ::xml::parse::type::port_type
            ( function.id().id_mapper()->next_id()
            , function.id().id_mapper()
            , boost::none
            , XML_PARSE_UTIL_POSITION_GENERATED()
            , name
            , type
            , boost::make_optional (direction == we::type::PORT_TUNNEL, name)
            , direction
            ).make_reference_id()
          );

        no_undo_move_item ( handle::port (port, function.document())
                          , position.get_value_or (QPointF())
                          );

        push ( new action::add_port ( ACTION_CTOR_ARGS (function)
                                    , function.id()
                                    , port
                                    )
             );

        endMacro();
      }

      namespace
      {
        bool is_connected_to_port ( const ::xml::parse::id::ref::connect& id
                                  , const ::xml::parse::id::ref::port& port
                                  )
        {
          return id.get().resolved_port() && *id.get().resolved_port() == port;
        }

        bool is_mapping_through_port ( const ::xml::parse::id::ref::place_map& id
                                     , const ::xml::parse::id::ref::port& port
                                     )
        {
          return id.get().resolved_tunnel_port()
            && *id.get().resolved_tunnel_port() == port;
        }
      }

      void change_manager_t::delete_port (const handle::port& port)
      {
        beginMacro (tr ("remove_port_and_connections_action"));

        if (port.get().place)
        {
          set_place_association (port, boost::none);
        }

        if (port.get().has_parent() && port.get().parent()->parent_transition())
        {
          //! \note remove_connection will modify transition's
          //! connections, thus copy out of there first, then modify.
          typedef boost::unordered_set< ::xml::parse::id::ref::connect>
            connections_to_delete_type;
          typedef boost::unordered_set< ::xml::parse::id::ref::place_map>
            place_maps_to_delete_type;

          connections_to_delete_type connections_to_delete
            ( boost::copy_range<connections_to_delete_type>
              ( port.get().parent()->parent_transition()->get().connections().ids()
              | boost::adaptors::filtered
                (boost::bind (is_connected_to_port, _1, port.id()))
              )
            );
          place_maps_to_delete_type place_maps_to_delete
            ( boost::copy_range<place_maps_to_delete_type>
              ( port.get().parent()->parent_transition()->get().place_map().ids()
              | boost::adaptors::filtered
                (boost::bind (is_mapping_through_port, _1, port.id()))
              )
            );

          BOOST_FOREACH ( const ::xml::parse::id::ref::connect& c
                        , connections_to_delete
                        )
          {
            remove_connection (handle::connect (c, port.document()));
          }
          BOOST_FOREACH ( const ::xml::parse::id::ref::place_map& pm
                        , place_maps_to_delete
                        )
          {
            remove_place_map (handle::place_map (pm, port.document()));
          }
        }

        if (port.is_tunnel())
        {
          const boost::optional<xml::parse::id::ref::place> place
            (port.get().resolved_place());
          if (place)
          {
            make_real (handle::place (*place, port.document()));
          }
        }

        push (new action::remove_port (ACTION_CTOR_ARGS (port), port.id()));

        endMacro();
      }

      void change_manager_t::set_property
        ( const data::handle::port& port
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::port>
               ( tr ("set_port_property_action")
               , ACTION_CTOR_ARGS (port), port, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::port& port
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (ACTION_CTOR_ARGS (port), port, key, val);
      }

      void change_manager_t::set_name
        (const data::handle::port& port, const QString& name)
      {
        beginMacro (tr ("port_set_name_action"));

        if (port.is_tunnel())
        {
          const handle::place place
            (*port.get().resolved_place(), port.document());

          push ( new action::meta_set_name<handle::place>
                 ( tr ("place_set_name_action")
                 , ACTION_CTOR_ARGS (place), place, name
                 )
               );

          set_place_association (port, name.toStdString());
        }

        push ( new action::meta_set_name<handle::port>
               ( tr ("port_set_name_action")
               , ACTION_CTOR_ARGS (port), port, name
               )
             );

        endMacro();
      }

      void change_manager_t::set_type
        (const data::handle::port& port, const QString& type)
      {
        beginMacro (tr ("port_set_type_action"));

        if (port.is_tunnel())
        {
          const handle::place place
            (*port.get().resolved_place(), port.document());

          push ( new action::meta_set_type<handle::place>
                 ( tr ("place_set_type_action")
                 , ACTION_CTOR_ARGS (place), place, type
                 )
               );
        }

        push ( new action::meta_set_type<handle::port>
               ( tr ("port_set_type_action")
               , ACTION_CTOR_ARGS (port), port, type
               )
             );

        endMacro();
      }

      void change_manager_t::set_place_association
        ( const data::handle::port& port
        , const boost::optional<std::string>& place
        )
      {
        beginMacro (tr ("set_place_association_action"));

        if (port.get().place && !place)
        {
          static QList<handle::port> recurse_check;
          if (!recurse_check.contains (port))
          {
            recurse_check.append (port);
            const handle::place place
              (*port.get().resolved_place(), port.document());
            delete_place (place);
            recurse_check.removeAll (port);
          }
        }

        push ( new action::set_place_association ( ACTION_CTOR_ARGS (port)
                                                 , port
                                                 , place
                                                 )
             );

        endMacro();
      }


      void change_manager_t::move_item ( const handle::port& port
                                       , const QPointF& position
                                       , const bool outer
                                       )
      {
        push ( new action::meta_move_item<handle::port>
               ( tr ("move_port_item_action"), ids::move_port_item
               , ACTION_CTOR_ARGS (port), port, position, outer
               )
             );
      }

      void change_manager_t::no_undo_move_item
        (const handle::port& port, const QPointF& position)
      {
        action::set_property ( ACTION_CTOR_ARGS (port)
                             , port
                             , "fhg.pnete.position.x"
                             , action::to_property_type (position.x())
                             );
        action::set_property ( ACTION_CTOR_ARGS (port)
                             , port
                             , "fhg.pnete.position.y"
                             , action::to_property_type (position.y())
                             );
      }

      // - function --------------------------------------------------
      void change_manager_t::set_function_name
        (const data::handle::function& fun, const QString& name)
      {
        push (new action::set_function_name (ACTION_CTOR_ARGS (fun), fun, name));
      }

      void change_manager_t::set_property
        ( const data::handle::function& function
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        push ( new action::meta_set_property<handle::function>
               ( tr ("set_function_property_action")
               , ACTION_CTOR_ARGS (function), function, key, val
               )
             );
      }

      void change_manager_t::no_undo_set_property
        ( const data::handle::function& function
        , const ::we::type::property::key_type& key
        , const ::we::type::property::value_type& val
        )
      {
        action::set_property (ACTION_CTOR_ARGS (function), function, key, val);
      }

      // - expression ------------------------------------------------
      void change_manager_t::set_expression
        (data::handle::expression& expression, const QString& text)
      {
        push ( new action::set_expression_content
               (ACTION_CTOR_ARGS (expression), expression, text)
             );
      }

#undef ACTION_CTOR_ARGS

#undef EMIT_SIGNAL
#undef EMIT_SIGNAL_CALL_NAME
#undef EMIT_SIGNAL_CALL_NAME_
#undef EMIT_SIGNAL_CALL_NAME__

#define EMITTER_ARGS(Z,N,TEXT) BOOST_PP_COMMA_IF(N)                     \
      typename boost::mpl::at_c                                         \
        < boost::function_types::parameter_types<Fun>                   \
        , BOOST_PP_ADD (N, 1)                                           \
        >::type BOOST_PP_CAT (arg, N)

#define EMITTER_BODY(Z,ARGC,TEXT)                                       \
      template<typename Fun>                                            \
      void change_manager_t::BOOST_PP_CAT (emit_signal, ARGC)           \
        ( Fun fun                                                       \
        , BOOST_PP_REPEAT (ARGC, EMITTER_ARGS, BOOST_PP_EMPTY)          \
        )                                                               \
      {                                                                 \
        emit (this->*fun)                                               \
          (BOOST_PP_ENUM_PARAMS (ARGC, arg));                           \
      }

      BOOST_PP_REPEAT_FROM_TO (1, 10, EMITTER_BODY, BOOST_PP_EMPTY)

#undef EMITTER_ARGS
#undef EMITTER_BODY

    }
  }
}
