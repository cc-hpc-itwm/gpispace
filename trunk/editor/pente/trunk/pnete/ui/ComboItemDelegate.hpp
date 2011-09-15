/*
 * ComoItemDelegate.hpp
 *
 *  Created on: Sep 14, 2011
 *      Author: tiberiu rotaru
 */

#ifndef COMBOITEMDELEGATE_HPP_
#define COMBOITEMDELEGATE_HPP_

#include <QStyledItemDelegate>
#include <QComboBox>
#include <vector>

class ComboBoxItemDelegate : public QStyledItemDelegate
{
    //Q_OBJECT

public:
    ComboBoxItemDelegate(QStringList qarrPredefinedTypes, QObject *parent = 0)
    : QStyledItemDelegate(parent)
    , m_qarrPredefinedTypes(qarrPredefinedTypes)
    {
    }

    virtual ~ComboBoxItemDelegate()
    {
    }

	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QComboBox *comboBox = new QComboBox(parent);
		//comboBox->setEditable(true);
		comboBox->setInsertPolicy(QComboBox::NoInsert);

		connect(comboBox, SIGNAL(activated(int)), this, SLOT(emitCommitData()));
		connect(comboBox->lineEdit(), SIGNAL(editingFinished()), this, SLOT(emitCommitData()));

		QString currentText = index.model()->data(index, Qt::DisplayRole).toString();

		int selectedItem = comboBox->findText(currentText);
		if(selectedItem == -1)
		   comboBox->setEditText(index.model()->data(index, Qt::DisplayRole).toString());
		else
		   comboBox->setCurrentIndex(selectedItem);

		return comboBox;
	}

	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		if( QComboBox *comboBox = qobject_cast<QComboBox*>(editor) )
		{
			//comboBox->clear();
			if(!m_qarrPredefinedTypes.isEmpty())
				comboBox->addItems(m_qarrPredefinedTypes);

			comboBox->model()->sort(0);

			QString currentText = index.model()->data(index, Qt::DisplayRole).toString();

			int selectedItem = comboBox->findText(currentText);

			if(selectedItem == -1)
				comboBox->setEditText(index.model()->data(index, Qt::DisplayRole).toString());
			else
				comboBox->setCurrentIndex(selectedItem);
		 }
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		if( QComboBox* comboBox = qobject_cast<QComboBox*>(editor) )
		{
			QStringList value;

			for(int i=0;i<comboBox->count();++i)
				value.append(comboBox->itemText(i));

			int selectedItem = comboBox->findText(comboBox->currentText());

			if(selectedItem == -1)
			{
				model->setData(index, comboBox->currentText(), Qt::DisplayRole);
			}
			else
			{
				model->setData(index, comboBox->itemText(selectedItem), Qt::DisplayRole);
			}
		}
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

    void emitCommitData()
    {
    	emit commitData(qobject_cast<QWidget *>(sender()));
    }

    QStringList m_qarrPredefinedTypes;
};

#endif /* COMOITEMDELEGATE_HPP_ */
