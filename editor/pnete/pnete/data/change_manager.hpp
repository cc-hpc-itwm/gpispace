// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <QUndoStack>

#include <xml/parse/types.hpp>

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
        class remove_transition;
        class remove_place;
      }
      namespace internal { class type; }

      class change_manager_t : public QUndoStack
      {
        Q_OBJECT;

      public:
        change_manager_t (internal::type &);

        internal::type& internal () const;

      public:

        void set_function_name ( const QObject*
                               , ::xml::parse::type::function_type&
                               , const QString&
                               );
        void set_expression ( const QObject*
                            , ::xml::parse::type::expression_type&
                            , const QString&
                            );
        void delete_transition ( const QObject*
                               , ::xml::parse::type::transition_type&
                               , ::xml::parse::type::net_type&
                               );
        void delete_place ( const QObject*
                          , ::xml::parse::type::place_type&
                          , ::xml::parse::type::net_type&
                          );

        void add_transition ( const QObject*
                            , ::xml::parse::type::net_type&
                            );
        void add_transition ( const QObject*
                            , ::xml::parse::type::function_type&
                            , ::xml::parse::type::net_type&
                            );
        void add_place ( const QObject*
                       , ::xml::parse::type::net_type&
                       );

      signals:
        void signal_set_expression ( const QObject*
                                   , const ::xml::parse::type::expression_type&
                                   , const QString&
                                   );
        void signal_set_expression_parse_result
             ( const QObject*
             , const ::xml::parse::type::expression_type&
             , const QString&
             );
        void signal_set_function_name ( const QObject*
                                      , const ::xml::parse::type::function_type&
                                      , const QString&
                                      );
        void
        signal_delete_transition ( const QObject*
                                 , const ::xml::parse::type::transition_type&
                                 , const ::xml::parse::type::net_type&
                                 );
        void signal_add_transition ( const QObject*
                                   , ::xml::parse::type::transition_type&
                                   , ::xml::parse::type::net_type&
                                   );
        void
        signal_delete_place ( const QObject*
                            , const ::xml::parse::type::place_type&
                            , const ::xml::parse::type::net_type&
                            );
        void signal_add_place ( const QObject*
                              , ::xml::parse::type::place_type&
                              , ::xml::parse::type::net_type&
                              );

      private:
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

        friend class action::remove_transition;
        friend class action::remove_place;

        internal::type& _internal;
      };
    }
  }
}

#endif
