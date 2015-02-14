/*
 * ComboItemDelegate.hpp
 *
 *  Created on: Sep 14, 2011
 *      Author: tiberiu rotaru
 */

#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>
#include <vector>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class ComboBoxItemDelegate : public QStyledItemDelegate
      {
      public:
        ComboBoxItemDelegate(const QStringList& types, QObject *parent = nullptr);

        virtual QWidget* createEditor ( QWidget *parent
                                      , const QStyleOptionViewItem &option
                                      , const QModelIndex &index
                                      ) const override;

        virtual void setEditorData ( QWidget *editor
                                   , const QModelIndex &index
                                   ) const override;
        virtual void setModelData ( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index
                                  ) const override;
        void emitCommitData();

      private:
        const QStringList _types;

        void set_text (const QModelIndex&, QComboBox*) const;
      };
    }
  }
}
