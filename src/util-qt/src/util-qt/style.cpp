// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/style.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      void repolish (QWidget* widget)
      {
        auto const style (widget->style());
        style->unpolish (widget);
        style->polish (widget);
      }

      void set_css_class_with_value
        (QWidget* widget, std::string const& style_class, QVariant value)
      {
        auto const property ("style-" + style_class);

        widget->setProperty (property.c_str(), value);

        repolish (widget);
      }

      void remove_css_class
        (QWidget* widget, std::string const& style_class)
      {
        //! \note An invalid variant clears a given property, removing
        //! the styling.
        set_css_class_with_value (widget, style_class, QVariant());
      }

      void set_css_class_enabled
        (QWidget* widget, std::string const& style_class, bool enabled)
      {
        if (enabled)
        {
          set_css_class_with_value
            (widget, style_class, QVariant::fromValue (true));
        }
        else
        {
          remove_css_class (widget, style_class);
        }
      }
    }
  }
}
