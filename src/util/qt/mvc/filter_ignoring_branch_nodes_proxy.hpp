// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          bool filterAcceptsRow (int, QModelIndex const& parent) const override;
        };
      }
    }
  }
}
