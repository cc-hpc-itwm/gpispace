
#include <pnete/ui/ComboItemDelegate.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      ComboBoxItemDelegate::ComboBoxItemDelegate ( const QStringList& types
                                                 , QObject *parent
                                                 )
        : QStyledItemDelegate (parent)
        , _types (types)
      {}

      QWidget* ComboBoxItemDelegate::createEditor
      ( QWidget *parent
      , const QStyleOptionViewItem &option
      , const QModelIndex &index
      ) const
      {
        QComboBox *comboBox (new QComboBox (parent));

        comboBox->setInsertPolicy(QComboBox::NoInsert);
        comboBox->setAutoCompletion(true);

        // only if it is editable!
        //comboBox->setEditable(true);
        //connect(comboBox, SIGNAL(activated(int)), this, SLOT(emitCommitData()));
        //connect(comboBox->lineEdit(), SIGNAL(editingFinished()), this, SLOT(emitCommitData()));

        set_text (index, comboBox);

        return comboBox;
      }

      void ComboBoxItemDelegate::setEditorData( QWidget *editor
                                              , const QModelIndex &index
                                              ) const
      {
        if(QComboBox *comboBox = qobject_cast<QComboBox*>(editor))
          {
            comboBox->addItems(_types);

            comboBox->model()->sort(0);

            set_text (index, comboBox);
          }
        else
          {
            QStyledItemDelegate::setEditorData(editor, index);
          }
      }

      void ComboBoxItemDelegate::set_text ( const QModelIndex& index
                                          , QComboBox* comboBox
                                          ) const
      {
        const QString currentText
          (index.model()->data(index, Qt::DisplayRole).toString());

        const int selectedItem (comboBox->findText (currentText));

        if(selectedItem == -1)
          {
            comboBox->setEditText ( index.model()->data( index
                                                       , Qt::DisplayRole
                                                       ).toString()
                                  );
          }
        else
          {
            comboBox->setCurrentIndex (selectedItem);
          }
      }

      void ComboBoxItemDelegate::setModelData ( QWidget *editor
                                              , QAbstractItemModel *model
                                              , const QModelIndex &index
                                              ) const
      {
        if(QComboBox* comboBox = qobject_cast<QComboBox*>(editor))
          {
            QStringList value;

            for(int i (0); i < comboBox->count(); ++i)
              {
                value.append (comboBox->itemText(i));
              }

            const int selectedItem
              (comboBox->findText (comboBox->currentText()));

            if(selectedItem == -1)
              {
                model->setData ( index
                               , comboBox->currentText()
                               , Qt::DisplayRole
                               );
              }
            else
              {
                model->setData ( index
                               , comboBox->itemText(selectedItem)
                               , Qt::DisplayRole
                               );
              }
          }
        else
          {
            QStyledItemDelegate::setEditorData(editor, index);
          }
      }

      void ComboBoxItemDelegate::emitCommitData()
      {
        emit commitData(qobject_cast<QWidget *>(sender()));
      }
    }
  }
}
