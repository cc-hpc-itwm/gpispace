// bernd.loerwald@itwm.fraunhofer.de

#include "file_line_edit.hpp"

#include <QHBoxLayout>

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

  connect (_button, SIGNAL (clicked()), SLOT (open_chooser()));
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
