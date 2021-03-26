// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
