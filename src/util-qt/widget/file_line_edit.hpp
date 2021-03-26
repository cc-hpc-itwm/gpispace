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

#pragma once

#include <util-qt/widget/file_line_edit.fwd.hpp>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <boost/filesystem/path.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace widget
      {
        class file_line_edit : public QWidget
        {
          Q_OBJECT

        public:
          file_line_edit (QFileDialog::FileMode, QWidget* = nullptr);
          file_line_edit ( QFileDialog::FileMode
                         , QString const&
                         , QWidget* = nullptr
                         );
          file_line_edit ( QFileDialog::FileMode
                         , boost::filesystem::path const&
                         , QWidget* = nullptr
                         );

          boost::filesystem::path value() const;
          void value (boost::filesystem::path const&);

          QString text() const;
          void text (QString);

        public slots:
          void open_chooser();

        signals:
          void text_changed();

        private:
          QLineEdit* _lineedit;
          QPushButton* _button;

          QFileDialog::FileMode _mode;
        };
      }
    }
  }
}
