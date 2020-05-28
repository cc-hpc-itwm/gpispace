#pragma once

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
          filter_ignoring_branch_nodes_proxy (QAbstractItemModel*, QObject* = nullptr);

        protected:
          virtual bool filterAcceptsRow (int, const QModelIndex& parent) const override;
        };
      }
    }
  }
}
