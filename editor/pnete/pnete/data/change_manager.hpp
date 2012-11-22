// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <pnete/data/change_manager.fwd.hpp>

#include <pnete/data/handle/connect.fwd.hpp>
#include <pnete/data/handle/expression.fwd.hpp>
#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/net.fwd.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>

#include <we/type/property.fwd.hpp>

//! \todo Should not be here.
#include <xml/parse/id/types.fwd.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/expression.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>

#include <QUndoStack>

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>

class QPointF;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace action
      {
        // ## editing action forward declarations ####################
        template<typename handle_type>
          void set_property ( const handle_type& handle
                            , const ::we::type::property::key_type& key
                            , const ::we::type::property::value_type& val
                            , change_manager_t& change_manager
                            , const QObject* origin
                            );
        // - net -----------------------------------------------------
        template<typename handle_type> class meta_move_item;
        // -- transition ---------------------------------------------
        class add_transition;
        class remove_transition;
        // -- place --------------------------------------------------
        class add_place;
        class remove_place;
        // - function ------------------------------------------------
        // - expression ----------------------------------------------
      }

      class change_manager_t : public QUndoStack
      {
        Q_OBJECT;

      public:
        //! \todo This is only a hack. There should not be a link to the state!
        change_manager_t (::xml::parse::state::type& state);

        // ## editing methods ########################################
        // - net -----------------------------------------------------

        // -- connection ---------------------------------------------
        void set_property ( const QObject*
                          , const data::handle::connect&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::connect&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );

        // -- transition ---------------------------------------------
        void add_transition ( const QObject*
                            , const data::handle::net&
                            );
        void add_transition ( const QObject*
                            , const ::xml::parse::id::ref::function& fun
                            , const data::handle::net&
                            );

        void delete_transition ( const QObject*
                               , const data::handle::transition&
                               );

        void set_property ( const QObject*
                          , const data::handle::transition&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::transition&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );
        void move_item ( const QObject*
                       , const handle::transition&
                       , const QPointF&
                       );
        void no_undo_move_item ( const QObject*
                               , const handle::transition&
                               , const QPointF&
                               );

        // -- place --------------------------------------------------
        void add_place ( const QObject*
                       , const data::handle::net&
                       );

        void delete_place ( const QObject*
                          , const data::handle::place&
                          );

        void set_property ( const QObject*
                          , const data::handle::place&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::place&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );
        void move_item ( const QObject*
                       , const handle::place&
                       , const QPointF&
                       );
        void no_undo_move_item ( const QObject*
                               , const handle::place&
                               , const QPointF&
                               );

        // -- port ---------------------------------------------------
        void set_property ( const QObject*
                          , const data::handle::port&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::port&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );
        void move_item ( const QObject*
                       , const handle::port&
                       , const QPointF&
                       );
        void no_undo_move_item ( const QObject*
                               , const handle::port&
                               , const QPointF&
                               );

        // - function ------------------------------------------------
        void set_function_name ( const QObject*
                               , const data::handle::function&
                               , const QString&
                               );
        void set_property ( const QObject*
                          , const data::handle::function&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::function&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );

        // - expression ----------------------------------------------
        void set_expression ( const QObject*
                            , data::handle::expression&
                            , const QString&
                            );

      signals:

        // ## signals after edit  ####################################
        // - net -----------------------------------------------------
        // -- connection ---------------------------------------------
        void property_changed ( const QObject*
                              , const data::handle::connect&
                              , const ::we::type::property::key_type&
                              , const ::we::type::property::value_type&
                              );

        // -- transition ---------------------------------------------
        void transition_added ( const QObject*
                              , const data::handle::transition&
                              );
        void transition_deleted ( const QObject*
                                , const data::handle::transition&
                                );
        void property_changed ( const QObject*
                              , const data::handle::transition&
                              , const ::we::type::property::key_type&
                              , const ::we::type::property::value_type&
                              );

        // -- place --------------------------------------------------
        void place_added (const QObject*, const data::handle::place&);
        void place_deleted (const QObject*, const data::handle::place&);
        void property_changed ( const QObject*
                              , const data::handle::place&
                              , const ::we::type::property::key_type&
                              , const ::we::type::property::value_type&
                              );

        // - port ----------------------------------------------------
        void property_changed ( const QObject*
                              , const data::handle::port&
                              , const ::we::type::property::key_type&
                              , const ::we::type::property::value_type&
                              );


        // - function ------------------------------------------------
        void function_name_changed ( const QObject*
                                   , const data::handle::function&
                                   , const QString&
                                   );

        void property_changed ( const QObject*
                              , const data::handle::function&
                              , const ::we::type::property::key_type&
                              , const ::we::type::property::value_type&
                              );

        // - expression ----------------------------------------------
        void signal_set_expression ( const QObject*
                                   , const data::handle::expression&
                                   , const QString&
                                   );
        void signal_set_expression_parse_result ( const QObject*
                                                , const data::handle::expression&
                                                , const QString&
                                                );

      private:

        // ## friend classes  ########################################
        // - net -----------------------------------------------------
        // -- connection ---------------------------------------------
        friend void action::set_property<handle::connect>
          ( const handle::connect& handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          , change_manager_t& change_manager
          , const QObject* origin
          );

        // -- transition ---------------------------------------------
        friend class action::add_transition;
        friend class action::remove_transition;
        friend void action::set_property<handle::transition>
          ( const handle::transition& handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          , change_manager_t& change_manager
          , const QObject* origin
          );
        friend class action::meta_move_item<handle::transition>;
        // -- place --------------------------------------------------
        friend class action::add_place;
        friend class action::remove_place;
        friend void action::set_property<handle::place>
          ( const handle::place& handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          , change_manager_t& change_manager
          , const QObject* origin
          );
        friend class action::meta_move_item<handle::place>;
        // -- port ---------------------------------------------------
        friend void action::set_property<handle::port>
          ( const handle::port& handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          , change_manager_t& change_manager
          , const QObject* origin
          );
        friend class action::meta_move_item<handle::port>;
        // - function ------------------------------------------------
        friend void action::set_property<handle::function>
          ( const handle::function& handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          , change_manager_t& change_manager
          , const QObject* origin
          );
        // - expression ----------------------------------------------

#define ARG_TYPE(function_type,n)                                       \
  boost::mpl::at_c<boost::function_types::parameter_types<function_type>,n>::type

        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1));
        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1)
                                 , typename ARG_TYPE(Fun,2));
        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1)
                                 , typename ARG_TYPE(Fun,2)
                                 , typename ARG_TYPE(Fun,3));
        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1)
                                 , typename ARG_TYPE(Fun,2)
                                 , typename ARG_TYPE(Fun,3)
                                 , typename ARG_TYPE(Fun,4));
        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1)
                                 , typename ARG_TYPE(Fun,2)
                                 , typename ARG_TYPE(Fun,3)
                                 , typename ARG_TYPE(Fun,4)
                                 , typename ARG_TYPE(Fun,5));
        template<typename Fun>
        void emit_signal (Fun fun, typename ARG_TYPE(Fun,1)
                                 , typename ARG_TYPE(Fun,2)
                                 , typename ARG_TYPE(Fun,3)
                                 , typename ARG_TYPE(Fun,4)
                                 , typename ARG_TYPE(Fun,5)
                                 , typename ARG_TYPE(Fun,6));
#undef ARG_TYPE

        ::xml::parse::state::type& _state;
      };
    }
  }
}

#endif
