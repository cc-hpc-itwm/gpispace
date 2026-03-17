#pragma once

#include <gspc/util/qt/widget/file_line_edit.fwd.hpp>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <filesystem>




      namespace gspc::util::qt::widget
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
                         , std::filesystem::path const&
                         , QWidget* = nullptr
                         );

          std::filesystem::path value() const;
          void value (std::filesystem::path const&);

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
