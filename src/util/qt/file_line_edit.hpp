// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class file_line_edit : public QWidget
      {
        Q_OBJECT;

      public:
        file_line_edit (QFileDialog::FileMode, const QString&, QWidget* = nullptr);

        QString text() const;

      public slots:
        void open_chooser();

      private:
        QLineEdit* _lineedit;
        QPushButton* _button;

        QFileDialog::FileMode _mode;
      };
    }
  }
}
