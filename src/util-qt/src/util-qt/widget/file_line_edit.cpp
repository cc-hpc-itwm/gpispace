// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/widget/file_line_edit.hpp>

#include <QtWidgets/QHBoxLayout>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace widget
      {
        file_line_edit::file_line_edit ( QFileDialog::FileMode mode
                                       , const QString& content
                                       , QWidget* parent
                                       )
          : QWidget (parent)
          , _lineedit (new QLineEdit (content, this))
          , _button (new QPushButton (tr ("choose.."), this))
          , _mode (mode)
        {
          new QHBoxLayout (this);
          layout()->addWidget (_lineedit);
          static_cast<QBoxLayout*> (layout())->addSpacing (2);
          layout()->addWidget (_button);
          layout()->setSpacing (0);
          layout()->setContentsMargins (0, 0, 0, 0);

          QObject::connect (_button, &QPushButton::clicked, this, &file_line_edit::open_chooser);
          QObject::connect (_lineedit, &QLineEdit::textChanged, this, &file_line_edit::text_changed);
        }

        file_line_edit::file_line_edit ( QFileDialog::FileMode mode
                                       , QWidget* parent
                                       )
          : file_line_edit (mode, QString(), parent)
        {}

        file_line_edit::file_line_edit ( QFileDialog::FileMode mode
                                       , ::boost::filesystem::path const& content
                                       , QWidget* parent
                                       )
          : file_line_edit
              (mode, QString::fromStdString (content.string()), parent)
        {}

        ::boost::filesystem::path file_line_edit::value() const
        {
          return text().toStdString();
        }
        void file_line_edit::value (::boost::filesystem::path const& val)
        {
          text (QString::fromStdString (val.string()));
        }

        void file_line_edit::open_chooser()
        {
          QFileDialog dialog (this, QString(), _lineedit->text());
          dialog.setFileMode (_mode);

          if (dialog.exec())
          {
            _lineedit->setText (dialog.selectedFiles().first());
          }
        }

        QString file_line_edit::text() const
        {
          return _lineedit->text();
        }
        void file_line_edit::text (QString v)
        {
          _lineedit->setText (v);
        }
      }
    }
  }
}
