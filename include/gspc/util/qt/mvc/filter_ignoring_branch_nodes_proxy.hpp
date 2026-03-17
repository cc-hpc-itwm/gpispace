// Copyright (C) 2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/qt/mvc/fixed_proxy_models.hpp>




      namespace gspc::util::qt::mvc
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
