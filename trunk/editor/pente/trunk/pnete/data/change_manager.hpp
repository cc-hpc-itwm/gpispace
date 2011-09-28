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

        void set_transition_name
        ( ::xml::parse::type::transition_type&
        , const QString&
        );
        void set_function_name
        ( ::xml::parse::type::function_type&
        , const QString&
        );

      signals:
        void signal_set_transition_name
        ( ::xml::parse::type::transition_type&
        , const QString&
        );
        void signal_set_function_name
        ( ::xml::parse::type::function_type&
        , const QString&
        );

      private:
        internal_type & _internal;
      };
    }
  }
}

#endif
