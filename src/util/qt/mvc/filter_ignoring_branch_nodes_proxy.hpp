// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_MVC_FILTER_IGNORING_BRANCH_NODES_PROXY_HPP
#define FHG_UTIL_QT_MVC_FILTER_IGNORING_BRANCH_NODES_PROXY_HPP

#include <util/qt/mvc/fixed_proxy_models.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class filter_ignoring_branch_nodes_proxy : public sort_filter_proxy
        {
          Q_OBJECT

        public:
          filter_ignoring_branch_nodes_proxy (QAbstractItemModel*, QObject* = NULL);

        protected:
          virtual bool filterAcceptsRow (int, const QModelIndex& parent) const;
        };
      }
    }
  }
}

#endif
