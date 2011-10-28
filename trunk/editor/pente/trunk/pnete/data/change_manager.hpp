// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <QObject>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type;

      class change_manager_t : public QObject
      {
        Q_OBJECT;

      public:
        change_manager_t (internal_type &);

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
                               , const ::xml::parse::type::transition_type&
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
                                 , ::xml::parse::type::net_type&
                                 );

      private:
        internal_type& _internal;
      };
    }
  }
}

#endif
