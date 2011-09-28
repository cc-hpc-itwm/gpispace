// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_MANAGER_HPP
#define _PNETE_DATA_MANAGER_HPP 1

#include <pnete/data/internal.hpp>

#include <QString>

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/optional.hpp>

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
        typedef boost::bimaps::set_of<internal_type::ptr> datas_type;
        typedef boost::bimap<ids_type, datas_type> bimap_type;

        bimap_type _files;

        std::size_t _unnamed_current;

      public:
        static manager& instance();

        internal_type::ptr load (const QString& filename);
        internal_type::ptr create();
        void save ( const internal_type::ptr & data
                  , const boost::optional<const QString&> filename = boost::none
                  ) const;
      };
    }
  }
}

#endif
