// bernd.loerwald@itwm.fraunhofer.de

#ifndef FILE_LINE_EDIT_HPP
#define FILE_LINE_EDIT_HPP

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class file_line_edit : public QWidget
{
  Q_OBJECT;

public:
  file_line_edit (QFileDialog::FileMode, const QString&, QWidget* = NULL);

  QString text() const;

public slots:
  void open_chooser();

private:
  QLineEdit* _lineedit;
  QPushButton* _button;

  QFileDialog::FileMode _mode;
};

#endif
