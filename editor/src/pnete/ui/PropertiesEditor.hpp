/*
 * PropertiesEditor.hpp
 *
 *  Created on: Sep 1, 2011
 *      Author: tiberiu rotaru
 */

#ifndef PROPERTIESEDITOR_HPP_
#define PROPERTIESEDITOR_HPP_

#include <QDebug>
#include <QObject>
#include <QtGui>
#include <QMessageBox>
#include <we/type/property.hpp>
#include <fhg/util/join.hpp>
#include <sstream>
#include <QAction>
#include <pnete/ui/ui_PropertiesEditorForm.h>

namespace prop = we::type::property;

typedef std::string key_type;
typedef std::string value_type;

namespace propedit {
// build property from QTreeView
template <typename T>
void build_property(T itemK, const we::type::property::path_type& path,  we::type::property::type& prop )
{
	for( int k=0; k<itemK->rowCount(); k++ )
	{
		T childItem = itemK->child(k);
		we::type::property::path_type pathChild(path);

		QString strItemName = childItem->text();
		pathChild.push_back(strItemName.toStdString());

		std::ostringstream ossPath("");
		for( we::type::property::path_type::const_iterator it=pathChild.begin(); it != pathChild.end(); it++  )
		{
			ossPath<<*it;
			if( it != path.end()-1)
				ossPath<<".";
		}

		QString qstrPath(QString::fromStdString (ossPath.str()));
		qDebug()<<"current path: "<<qstrPath;

		T itemV = itemK->child(k, 1);

		if(itemV)
		{
			QString strItemVal = itemV->text();
			if(!strItemVal.isEmpty())
				prop.set(pathChild, strItemVal.toStdString());
		}

		build_property(childItem, pathChild, prop);
	}
}

// build QTreeView from property
template <typename E, typename T>
void build_tree(E* editor, QStandardItem* item, const we::type::property::type& p )
{
	for( we::type::property::map_type::const_iterator pos (p.get_map().begin()); pos != p.get_map().end() ; ++pos)
		 boost::apply_visitor( T(editor, item, pos->first), pos->second);
}

template <typename E>
class tree_builder : public boost::static_visitor<void>
{
private:
	E* editor;
	QStandardItem* item;
	const key_type& key;

public:
	tree_builder (E* _editor, QStandardItem* _item, const key_type & _key)
	: editor(_editor)
	, item (_item)
	, key (_key)
	{}

	void operator () (const value_type & v) const
	{
		editor->addItem(QString::fromStdString(key),  QString::fromStdString(v), item);
	}

	template<typename T>
	void operator () (const T& x) const
	{
		QStandardItem *currItem = editor->addItem(QString::fromStdString(key), "", item);
		build_tree<E, tree_builder>(editor, currItem, x);
	}
};

class PropertiesEditor : public QObject, public Ui::PropertiesEditorWidgetMCV
{
	Q_OBJECT
public:
	PropertiesEditor() : model(0,2)
	{
		m_widgetPropEdit = new QWidget();
		setupUi(m_widgetPropEdit);

		QStringList headerLabels;
		headerLabels << "Properties"<<"Values";

		model.setHorizontalHeaderLabels(headerLabels);
		m_treeView->setModel(&model);

		m_treeView->setColumnWidth(0, 300);
		m_treeView->setColumnWidth(1, 200);

		m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	}

	virtual ~PropertiesEditor()
	{
		delete m_treeView;
		delete m_widgetPropEdit;
	}

	// build_tree another init method
	// that loads items from a we::type::property object!
	void initEditor()
	{
		QObject::connect(m_buttonDelete, SIGNAL(clicked()), this, SLOT(deleteItem()));
		QObject::connect(m_buttonAdd, 	 SIGNAL(clicked()), this, SLOT(addItem()));
		QObject::connect(m_buttonClose,  SIGNAL(clicked()), this, SLOT(closeEditor()));

		m_addAction = new QAction(tr("Add"), this );
		connect(m_addAction, SIGNAL(triggered()), this, SLOT(addItem()));

		m_delAction = new QAction(tr("Delete"), this );
		connect(m_delAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

		// connect custom context menu
		connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	}

	void show() { m_widgetPropEdit->show(); }

public slots:
	void deleteItem()
	{
		QModelIndexList listSelectedItems = m_treeView->selectionModel()->selectedIndexes();

		if(listSelectedItems.empty())
			QMessageBox::critical(NULL, tr("Properties editor"),tr("Please, select a property to delete first!"));
		else
		{
			QModelIndex currModelIdx = listSelectedItems.at(0);
     		QStandardItem* currItem = model.itemFromIndex(currModelIdx);

     		if( QStandardItem* parent = currItem->parent() )
     		{
     			parent->removeRow(currItem->row());

     			if( !parent->hasChildren() )
     			{
					QStandardItem* itemV = new QStandardItem();
					itemV->setText("");

					int row = parent->row();
					if(parent->parent())
						parent->parent()->setChild( row, 1, itemV );
     			}

     			m_treeView->scrollTo(model.indexFromItem(parent));
     			m_treeView->setCurrentIndex(model.indexFromItem(parent));
     		}
     		else
     			model.removeRow(currItem->row());
		}
	}

	void addItem()
	{
		if( model.invisibleRootItem()->rowCount() == 0)
		{
			QStandardItem* newItem = addItem("New Item", "", model.invisibleRootItem() );

			QStandardItem* currItem = model.invisibleRootItem();
			m_treeView->expand(model.indexFromItem(currItem));
			m_treeView->scrollTo(model.indexFromItem(newItem));
			m_treeView->setCurrentIndex(model.indexFromItem(newItem));
		}
		else
		{
			// check first if is empty
			QModelIndexList listSelectedItems = m_treeView->selectionModel()->selectedIndexes();

			if(listSelectedItems.empty())
				QMessageBox::critical(NULL, tr("Properties editor"), tr("Please, select a property first!"));
			else
			{
				QModelIndex currModelIdx = listSelectedItems.at(0);
				QStandardItem* currItem  = model.itemFromIndex(currModelIdx);
				QStandardItem* newItem   = addItem("New item", "", currItem);

				m_treeView->expand(model.indexFromItem(currItem));
				m_treeView->scrollTo(model.indexFromItem(newItem));
				m_treeView->setCurrentIndex(model.indexFromItem(newItem));
			}
		}
	}

	void closeEditor()
	{
		we::type::property::type propOut;
		we::type::property::path_type rootPath;

		QStandardItem* rootItem = model.invisibleRootItem();
		build_property(rootItem, rootPath, propOut);

		print_property(propOut);
		m_widgetPropEdit->close();
	}

	void showContextMenu(const QPoint& pnt)
	{
		QList<QAction *> actions;
		if (m_treeView->indexAt(pnt).isValid())
		{
			actions.append(m_addAction);
			actions.append(m_delAction);
		}

		if (actions.count() > 0)
			QMenu::exec(actions, m_treeView->mapToGlobal(pnt));
	}

public:
	void initProp(we::type::property::type& prop)
	{
		prop.set ("A.A.A", "value_of (A.A.A)");

		prop.set ("A.A", "value_of (A.A)");
		prop.set ("A", "value_of (A)");
		prop.set ("A.A.A", "value_of (A.A.A)");
		prop.set ("A.A.B", "value_of (A.A.B)");
		prop.set ("A.A.C", "value_of (A.A.C)");
		prop.set ("A.B.A", "value_of (A.B.A)");
		prop.set ("A.B.B", "value_of (A.B.B)");
		prop.set ("A.C.A", "value_of (A.C.A)");
		prop.set ("A.C.A", "value_of (A.C.A)");
		prop.set ("B.A.A", "value_of (B.A.A)");
		prop.set ("A.A.B.A", "value_of (A.A.B.A)");
	}

	void loadFromProp(const we::type::property::type& prop)
	{
		build_tree<PropertiesEditor, tree_builder<PropertiesEditor> >(this,  model.invisibleRootItem(), prop);
	}

	void print_property(const we::type::property::type& prop)
	{
		qDebug() << "# visit all leafs:";

		prop::traverse::stack_type stack (prop::traverse::dfs (prop));

		while (!stack.empty())
		{
			const prop::traverse::pair_type elem (stack.top());
			const prop::path_type path (elem.first);
			const prop::value_type value (elem.second);

			qDebug()<< QString::fromStdString(fhg::util::join (path, ".")) << " => " << QString::fromStdString(value);

			stack.pop();
		}
	}

	void testEditor()
	{
		we::type::property::type propIn;

		initEditor();
		initProp(propIn);
		loadFromProp(propIn);

		show();
	}

	QStandardItem *addItem(QString key, QString value, QStandardItem* parent)
	{
		QStandardItem* itemK = new QStandardItem();
		itemK->setText(key);

		QStandardItem* itemV = new QStandardItem();
		itemV->setText(value);

		QStandardItem* itemP;
		if( parent == model.invisibleRootItem() && value.isEmpty() )
		{
			itemV->setText("");
			itemV->setEnabled(false);
			itemV->setEditable(false);
			itemV->setSelectable(false);
		}
		else
		if( parent->parent() && (itemP = parent->parent()->child(parent->row(),1)) )
		{
			itemP->setText("");
			itemP->setEnabled(false);
			itemP->setEditable(false);
			itemP->setSelectable(false);
		}

		QList<QStandardItem *> qListItems;
		qListItems<<itemK<<itemV;
		parent->appendRow(qListItems);

		return itemK;
	}

private:

	//actions
	QAction* m_addAction;
	QAction* m_delAction;

	QStandardItemModel model;
	QWidget *m_widgetPropEdit;
};
}

#endif /* PROPERTIESEDITOR_HPP_ */
