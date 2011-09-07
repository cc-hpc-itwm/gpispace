/*
 * PropertiesEditor.hpp
 *
 *  Created on: Sep 1, 2011
 *      Author: tiberiu rotaru
 */

#ifndef PROPERTIESEDITOR_HPP_
#define PROPERTIESEDITOR_HPP_

#include "ui/ui_PropertiesWidget.h"
#include <QDebug>
#include <QObject>
#include <QMessageBox>
#include <we/type/property.hpp>
#include <fhg/util/join.hpp>
#include <sstream>

namespace prop = we::type::property;

typedef std::string key_type;
typedef std::string value_type;

// build property from QWidgetTree
template <typename T>
void build_property(T item, const we::type::property::path_type& path,  we::type::property::type& prop )
{
	if(item->childCount() == 0)
	{
		we::type::property::path_type pathChild(path);

		QString strItemName = item->text(0);
		pathChild.push_back(strItemName.toStdString());

		std::ostringstream ossPath("");
		for(we::type::property::path_type::const_iterator it=path.begin(); it != path.end(); it++  )
			ossPath<<"."<<*it;

		ossPath<<"."<<strItemName.toStdString();

		QString qstrPath(ossPath.str().c_str());
		qDebug()<<"current path: "<<qstrPath;

		QString strItemVal = item->text(1);
		if(!strItemVal.isEmpty())
			prop.set(pathChild, strItemVal.toStdString());

	}
	else
		for( int k=0; k<item->childCount(); k++ )
		{
			T childItem = item->child(k);
			we::type::property::path_type pathChild(path);

			QString strItemName = item->text(0);
			pathChild.push_back(strItemName.toStdString());

			std::ostringstream ossPath("");
			for(we::type::property::path_type::const_iterator it=path.begin(); it != path.end(); it++  )
				ossPath<<"."<<*it;

			ossPath<<"."<<strItemName.toStdString();

			QString qstrPath(ossPath.str().c_str());
			qDebug()<<"current path: "<<qstrPath;

			QString strItemVal = item->text(1);
			if(!strItemVal.isEmpty())
				prop.set(pathChild, strItemVal.toStdString());

			build_property(childItem, pathChild, prop);
		}
}

// build QWidgetTree from property
template <typename E, typename T>
void build_tree(E* editor, QTreeWidgetItem* item, const we::type::property::type& p )
{
	for( we::type::property::map_type::const_iterator pos (p.get_map().begin()); pos != p.get_map().end() ; ++pos)
		 boost::apply_visitor( T(editor, item, pos->first), pos->second);
}

template <typename E>
class tree_builder : public boost::static_visitor<void>
{
private:
	E* editor;
	QTreeWidgetItem* item;
	const key_type& key;

public:
	tree_builder (E* _editor, QTreeWidgetItem* _item, const key_type & _key)
	: editor(_editor)
	, item (_item)
	, key (_key)
	{}

	void operator () (const value_type & v) const
	{
		QTreeWidgetItem *currItem = editor->addItem(QString::fromStdString(key), item);
		currItem->setText(1,  QString::fromStdString(v));
	}

	template<typename T>
	void operator () (const T& x) const
	{
		QTreeWidgetItem *currItem = editor->addItem(QString::fromStdString(key), item);
		build_tree<E, tree_builder>(editor, currItem, x);
	}
};

class PropertiesEditor : public QObject, public Ui::PropertiesEditorWidget
{
	Q_OBJECT
public:
	PropertiesEditor()
	{
		m_widgetPropEdit = new QWidget();
		setupUi(m_widgetPropEdit);

		m_treeWidget->setColumnCount(2);
		m_treeWidget->setVisible(true);

		QStringList headerLabels;
		headerLabels << "Properties"<<"Values";

		m_treeWidget->setHeaderLabels(headerLabels);
		m_treeWidget->header()->resizeSection(0, 200);
		m_treeWidget->header()->resizeSection(1, 300);
	}

	virtual ~PropertiesEditor()
	{
		for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i)
	    {
			qDeleteAll(m_treeWidget->topLevelItem(i)->takeChildren());
	    }

		delete m_treeWidget;
		delete m_widgetPropEdit;
	}

	// build_tree another init method
	// that loads items from a we::type::property object!
	void initEditor()
	{
		QObject::connect(m_buttonDelete, SIGNAL(clicked()), this, SLOT(deleteItem()));
		QObject::connect(m_buttonAdd, 	 SIGNAL(clicked()), this, SLOT(addItem()));
		QObject::connect(m_buttonClose,  SIGNAL(clicked()), this, SLOT(closeEditor()));
	}

	void addItem(QString itemName, QString parentName)
	{
		// first find parent name parentName
		Qt::MatchFlags flags = static_cast<Qt::MatchFlags> ( Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive );
		QList<QTreeWidgetItem *> arrItems = m_treeWidget->findItems( parentName, flags, 0 );

		if(arrItems.size() == 0)
		{
			qDebug()<<"No item named "<<parentName<<" was found!";

			if( m_treeWidget->topLevelItemCount () == 0)
			{
				QTreeWidgetItem* newItem = addItem(itemName);
				m_treeWidget->insertTopLevelItem(0,newItem);
				m_treeWidget->setCurrentItem(newItem);
				m_treeWidget->expandItem(newItem);
			}
		}
		else
			if(arrItems.size()>1)
				qDebug()<<"Several items named "<<parentName<<" were found!";
			else
			{
				QTreeWidgetItem* newItem = addItem(itemName, arrItems[0]);
				//m_treeWidget->expandItem(arrItems[0]);
				//m_treeWidget->scrollToItem(newItem);
			}

	}

	void deleteItem(QString itemName)
	{
		// first find parent name parentName
		Qt::MatchFlags flags = static_cast<Qt::MatchFlags> ( Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive );
		QList<QTreeWidgetItem *> arrItems = m_treeWidget->findItems( itemName, flags, 0 );

		if(arrItems.size() == 0)
			qDebug()<<"No item named "<<itemName<<" was found!";
		else
			if(arrItems.size()>1)
				qDebug()<<"Several items named "<<itemName<<" were found!";
			else
			{
				QTreeWidgetItem* item = arrItems[0];

				int i = m_treeWidget->indexOfTopLevelItem(item);
				m_treeWidget->takeTopLevelItem(i);
				delete item; // do not forget to delete the item if it is not owned by any other widget.

			}
	}

	void show() { m_widgetPropEdit->show(); }

	// save/serialize the content into a we::type::property
public slots:
	void deleteItem()
	{
		QList<QTreeWidgetItem *>  listSelectedItems = m_treeWidget->selectedItems ();
		if(listSelectedItems.empty())
			QMessageBox::critical(NULL, tr("Properties editor"),tr("Please, select a property to delete first!"));
		else
		{
			//QTreeWidgetItem* item = m_treeWidget->currentItem();
			int i = m_treeWidget->indexOfTopLevelItem(listSelectedItems[0]);
			m_treeWidget->takeTopLevelItem(i);
			delete listSelectedItems[0];
		}
	}

	void addItem()
	{
		//QTreeWidgetItem* currItem = m_treeWidget->currentItem();
		if( m_treeWidget->topLevelItemCount () == 0)
		{
			QTreeWidgetItem* newItem = addItem("New Item");
			m_treeWidget->insertTopLevelItem(0,newItem);
			m_treeWidget->setCurrentItem(newItem);
			m_treeWidget->expandItem(newItem);
		}
		else
		{
			QList<QTreeWidgetItem *>  listSelectedItems = m_treeWidget->selectedItems ();
			if( listSelectedItems.empty())
				QMessageBox::critical(NULL, tr("Properties editor"), tr("Please, select a property first!"));
			else
			{
				QTreeWidgetItem* currItem = listSelectedItems[0];
				QTreeWidgetItem* newItem = addItem("New item", currItem);
				currItem->setExpanded(true);
				newItem->setSelected(true);
				m_treeWidget->setCurrentItem(newItem);
			}
		}
	}

	void closeEditor()
	{
		we::type::property::type propOut;
		we::type::property::path_type rootPath;
		QTreeWidgetItem* rootItem = m_treeWidget->topLevelItem(0);

		build_property(rootItem, rootPath, propOut);
		print_property(propOut);

		m_widgetPropEdit->close();
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

	void loadFromWeProp(const we::type::property::type& prop)
	{
		build_tree<PropertiesEditor, tree_builder<PropertiesEditor> >(this, NULL, prop);
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

	void testWidget()
	{
		we::type::property::type propIn;

		initEditor();
		initProp(propIn);

		loadFromWeProp(propIn);

		show();
	}

	QTreeWidgetItem *addItem(QString name, QTreeWidgetItem* parent=0)
	{
		QTreeWidgetItem *retval = new QTreeWidgetItem(parent);
		retval->setFlags( Qt::ItemIsSelectable
						| Qt::ItemIsEditable
						| Qt::ItemIsDragEnabled
						| Qt::ItemIsDropEnabled
						| Qt::ItemIsEnabled );

		retval->setText(0, name);

		if(!parent)
			m_treeWidget->addTopLevelItem(retval);

		return retval;
	}

private:

	QWidget *m_widgetPropEdit;
};


#endif /* PROPERTIESEDITOR_HPP_ */
