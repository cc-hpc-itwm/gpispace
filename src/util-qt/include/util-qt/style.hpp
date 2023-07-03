// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QStyle>
#include <QtWidgets/QWidget>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      void repolish (QWidget* widget);

      void set_css_class_with_value
        (QWidget* widget, std::string const& style_class, QVariant value);

      void remove_css_class
        (QWidget* widget, std::string const& style_class);

      void set_css_class_enabled
        (QWidget* widget, std::string const& style_class, bool enabled = true);
    }
  }
}
