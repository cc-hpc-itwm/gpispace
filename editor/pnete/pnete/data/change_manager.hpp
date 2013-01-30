// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <pnete/data/change_manager.fwd.hpp>

#include <pnete/data/handle/connect.fwd.hpp>
#include <pnete/data/handle/expression.fwd.hpp>
#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/net.fwd.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/place_map.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>

#include <we/type/property.fwd.hpp>
#include <we/type/port.hpp> // we::type::PortDirection

#include <xml/parse/id/types.fwd.hpp>

#include <QUndoStack>

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <string>

class QPointF;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class change_manager_t : public QUndoStack
      {
        Q_OBJECT;

      public:
        change_manager_t (QObject* parent = NULL);

        // ## editing methods ########################################
        // - net -----------------------------------------------------

        // -- connection ---------------------------------------------
        void add_connection ( const QObject*
                            , const data::handle::place&
                            , const data::handle::port&
                            , bool no_make_explicit = false
                            );
        //! \note port <-> implicit_place <-> port (convenience)
        void add_connection ( const QObject*
                            , const data::handle::port&
                            , const data::handle::port&
                            , const data::handle::net&
                            );
        void remove_connection (const QObject*, const data::handle::connect&);
        void connection_is_read
          (const QObject*, const data::handle::connect&, const bool&);

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

        // -- place_map ------------------------------------------------
        void remove_place_map (const QObject*, const data::handle::place_map&);

        void set_property ( const QObject*
                          , const data::handle::place_map&
                          , const ::we::type::property::key_type&
                          , const ::we::type::property::value_type&
                          );
        void no_undo_set_property ( const QObject*
                                  , const data::handle::place_map&
                                  , const ::we::type::property::key_type&
                                  , const ::we::type::property::value_type&
                                  );

        // -- transition ---------------------------------------------
        void add_transition ( const QObject*
                            , const data::handle::net&
                            , const boost::optional<QPointF>&
                            );
        void add_transition ( const QObject*
                            , const ::xml::parse::id::ref::function& fun
                            , const data::handle::net&
                            , const boost::optional<QPointF>&
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
                       , const bool outer = false
                       );
        void no_undo_move_item ( const QObject*
                               , const handle::transition&
                               , const QPointF&
                               );

        void set_name
          (const QObject*, const data::handle::transition&, const QString&);

        // -- place --------------------------------------------------
        void add_place ( const QObject*
                       , const data::handle::net&
                       , const boost::optional<QPointF>&
                       );
        void delete_place (const QObject*, const data::handle::place&);

        void set_name
          (const QObject*, const data::handle::place&, const QString&);
        void set_type
          (const QObject*, const data::handle::place&, const QString&);

        void make_explicit (const QObject*, const data::handle::place&);

        void make_virtual (const QObject*, const data::handle::place&);
        void make_real (const QObject*, const data::handle::place&);

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
                       , const bool outer = false
                       );
        void no_undo_move_item ( const QObject*
                               , const handle::place&
                               , const QPointF&
                               );

        // -- port ---------------------------------------------------
        void add_port ( const QObject*
                      , const data::handle::function&
                      , const we::type::PortDirection&
                      , const boost::optional<QPointF>&
                      );
        void delete_port (const QObject*, const data::handle::port&);

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

        void set_name
          (const QObject*, const data::handle::port&, const QString&);
        void set_type
          (const QObject*, const data::handle::port&, const QString&);

        void set_place_association
          ( const QObject*
          , const data::handle::port&
          , const boost::optional<std::string>& place = boost::none
          );

        void move_item ( const QObject*
                       , const handle::port&
                       , const QPointF&
                       , const bool outer = false
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

#define EMITTER_ARGS(Z,N,TEXT) BOOST_PP_COMMA_IF(N)                     \
      typename boost::mpl::at_c                                         \
        < boost::function_types::parameter_types<Fun>                   \
        , BOOST_PP_ADD (N, 1)                                           \
        >::type

#define EMITTER_BODY(Z,ARGC,TEXT)                                       \
      template<typename Fun>                                            \
      void BOOST_PP_CAT (emit_signal, ARGC)                             \
        ( Fun fun                                                       \
        , BOOST_PP_REPEAT (ARGC, EMITTER_ARGS, BOOST_PP_EMPTY)          \
        );

      BOOST_PP_REPEAT_FROM_TO (1, 10, EMITTER_BODY, BOOST_PP_EMPTY)

#undef EMITTER_ARGS
#undef EMITTER_BODY

      signals:

        // ## signals after edit  ####################################
        // - net -----------------------------------------------------
        // -- connection ---------------------------------------------
        void property_changed ( const QObject*
                              , const data::handle::connect&
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
                              );
        void connection_added ( const QObject*
                              , const data::handle::connect&
                              , const data::handle::place&
                              , const data::handle::port&
                              );
        void connection_removed ( const QObject*
                                , const data::handle::connect&
                                );
        void connection_direction_changed
          (const QObject*, const data::handle::connect&);

        // -- place_map ---------------------------------------------
        void property_changed ( const QObject*
                              , const data::handle::place_map&
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
                              );
        void place_map_added ( const QObject*
                             , const data::handle::place_map&
                             );
        void place_map_removed ( const QObject*
                               , const data::handle::place_map&
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
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
                              );
        void name_set
          (const QObject*, const data::handle::transition&, const QString&);

        // -- place --------------------------------------------------
        void place_added (const QObject*, const data::handle::place&);
        void place_deleted (const QObject*, const data::handle::place&);
        void place_is_virtual_changed
          (const QObject*, const data::handle::place&, bool);
        void property_changed ( const QObject*
                              , const data::handle::place&
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
                              );
        void name_set
          (const QObject*, const data::handle::place&, const QString&);
        void type_set
          (const QObject*, const data::handle::place&, const QString&);

        // - port ----------------------------------------------------
        void port_added (const QObject*, const data::handle::port&);
        void port_deleted (const QObject*, const data::handle::port&);

        void property_changed ( const QObject*
                              , const data::handle::port&
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
                              );

        void name_set
          (const QObject*, const data::handle::port&, const QString&);
        void type_set
          (const QObject*, const data::handle::port&, const QString&);
        void place_association_set
          (const QObject*, const data::handle::port&, const boost::optional<std::string>&);

        // - function ------------------------------------------------
        void function_name_changed ( const QObject*
                                   , const data::handle::function&
                                   , const QString&
                                   );

        void property_changed ( const QObject*
                              , const data::handle::function&
                              , const we::type::property::key_type&
                              , const we::type::property::value_type&
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
      };
    }
  }
}

#endif
