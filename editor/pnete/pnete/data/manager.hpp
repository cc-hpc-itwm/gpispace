// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_MANAGER_HPP
#define _PNETE_DATA_MANAGER_HPP 1

//! \note boost/multi_index/ordered_index.hpp uses a namespace
//! 'foreach' which gets defined to 'Q_FOREACH' in QObject (included
//! from QString), thus breaks.
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>

#include <boost/optional.hpp>

#include <pnete/data/internal.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class manager
      {
      private:
        manager();

        typedef QString id_type;
        typedef boost::bimaps::set_of<id_type> ids_type;
        typedef boost::bimaps::set_of<internal_type*> datas_type;
        typedef boost::bimap<ids_type, datas_type> bimap_type;

        bimap_type _files;

        std::size_t _unnamed_current;

      public:
        static manager& instance();

        internal_type* load (const QString& filename);
        internal_type* create (const internal_type::kind&);
        void save ( internal_type* data
                  , const boost::optional<const QString&> filename = boost::none
                  ) const;
        //! \todo add close!
      };
    }
  }
}

#endif
