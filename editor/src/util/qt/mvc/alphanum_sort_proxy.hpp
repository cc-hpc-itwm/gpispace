// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_MVC_ALPHANUM_SORT_PROXY_HPP
#define FHG_UTIL_QT_MVC_ALPHANUM_SORT_PROXY_HPP

#include <util/qt/mvc/fixed_proxy_models.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class alphanum_sort_proxy : public sort_filter_proxy
        {
          Q_OBJECT

        public:
          alphanum_sort_proxy (QAbstractItemModel*, QObject* = NULL);

        protected:
          virtual bool lessThan (const QModelIndex&, const QModelIndex&) const;
        };
      }
    }
  }
}

#endif
