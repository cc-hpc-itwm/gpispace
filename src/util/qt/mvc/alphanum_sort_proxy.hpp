// Copyright (C) 2025 Fraunhofer ITWM
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
        class alphanum_sort_proxy : public sort_filter_proxy
        {
          Q_OBJECT

        public:
          alphanum_sort_proxy (QAbstractItemModel*, QObject* = nullptr);

        protected:
          bool lessThan (QModelIndex const&, QModelIndex const&) const override;
        };
      }
    }
  }
}
