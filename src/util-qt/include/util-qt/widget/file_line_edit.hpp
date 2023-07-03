// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
                         , ::boost::filesystem::path const&
                         , QWidget* = nullptr
                         );

          ::boost::filesystem::path value() const;
          void value (::boost::filesystem::path const&);

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
