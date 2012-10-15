// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <pnete/data/handle/net.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>
#include <pnete/data/handle/place.fwd.hpp>

#include <xml/parse/types.hpp>

#include <QUndoStack>

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace action
      {
        // ## editing action forward declarations ####################
        // - net -----------------------------------------------------
        // -- transition ---------------------------------------------
        class remove_transition;
        // -- place --------------------------------------------------
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

        // -- transition ---------------------------------------------
        void add_transition ( const QObject*
                            , const data::handle::net&
                            );
        void add_transition ( const QObject*
                            , const ::xml::parse::type::function_type& fun
                            , const data::handle::net&
                            );

        void delete_transition ( const QObject*
                               , const data::handle::transition&
                               );

        // -- place --------------------------------------------------
        void add_place ( const QObject*
                       , const data::handle::net&
                       );

        void delete_place ( const QObject*
                          , const data::handle::place&
                          );

        // - function ------------------------------------------------
        void set_function_name ( const QObject*
                               , ::xml::parse::type::function_type&
                               , const QString&
                               );

        // - expression ----------------------------------------------
        void set_expression ( const QObject*
                            , ::xml::parse::type::expression_type&
                            , const QString&
                            );

      signals:

        // ## signals after edit  ####################################
        // - net -----------------------------------------------------

        // -- transition ---------------------------------------------
        void transition_added ( const QObject*
                              , const data::handle::transition&
                              );
        void transition_deleted ( const QObject*
                                , const data::handle::transition&
                                );

        // -- place --------------------------------------------------
        void place_added (const QObject*, const data::handle::place&);
        void place_deleted (const QObject*, const data::handle::place&);

        // - function ------------------------------------------------
        void signal_set_function_name ( const QObject*
                                      , const ::xml::parse::type::function_type&
                                      , const QString&
                                      );

        // - expression ----------------------------------------------
        void signal_set_expression ( const QObject*
                                   , const ::xml::parse::type::expression_type&
                                   , const QString&
                                   );
        void signal_set_expression_parse_result ( const QObject*
                                                , const ::xml::parse::type::expression_type&
                                                , const QString&
                                                );

      private:

        // ## friend classes  ########################################
        // - net -----------------------------------------------------
        // -- transition ---------------------------------------------
        friend class action::remove_transition;
        // -- place --------------------------------------------------
        friend class action::remove_place;
        // - function ------------------------------------------------
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
