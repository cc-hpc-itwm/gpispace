// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <QtWidgets/QMessageBox>

#include <util-generic/callable_signature.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace detail
      {
        struct default_button_tag;
        template<QMessageBox::StandardButton, typename, typename>
          struct action_t;
        template<QMessageBox::StandardButton Button, typename Ret>
          using button_t = detail::action_t<Button, Ret, void>;
        template<QMessageBox::StandardButton Button, typename Ret>
          using default_button_t = detail::action_t<Button, Ret, detail::default_button_tag>;
      }

      //! Opens a QMessageBox with the given icon, parent, title and
      //! text as well as the actions listed and executes the user
      //! selected action.
      //!
      //! \param actions buttons and their connected action, as created
      //!                by \ref button() and \ref default_button().
      //!
      //! \note Exactly one or no button may be a default button (if
      //!       none, Qt chooses a sane default).
      //! \note Actions may have return values, but the return value
      //!       must be consistent for all actions.
      //!
      //! \code
      //! // Example: Show a question message box for a boolean choice.
      //! bool const result
      //!   ( message_box
      //!       ( QMessageBox::Question
      //!       , nullptr
      //!       , "question"
      //!       , "yes or no?"
      //!       , button<QMessageBox::Yes> ([] { return true; })
      //!       , default_button<QMessageBox::No> ([] { return false; })
      //!       )
      //!   );
      //! \endcode
      template<typename Ret, QMessageBox::StandardButton... Buttons, typename... Tags>
        Ret message_box ( QMessageBox::Icon icon
                        , QWidget* parent
                        , QString const& title
                        , QString const& text
                        , detail::action_t<Buttons, Ret, Tags>... actions
                        );

      //! Create a button for \ref message_box().
      template<QMessageBox::StandardButton Button, typename Fun>
        detail::button_t<Button, return_type<Fun>> button (Fun);
      //! Create a button for \ref message_box() that is selected by default.
      template<QMessageBox::StandardButton Button, typename Fun>
        detail::default_button_t<Button, return_type<Fun>> default_button (Fun);
    }
  }
}

#include <util-qt/message_box.ipp>
