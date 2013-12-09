// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_SECTION_INDEX_HPP
#define FHG_UTIL_QT_SECTION_INDEX_HPP

#include <QHeaderView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        struct section_index
        {
          QAbstractItemModel* _model;
          Qt::Orientation _orientation;
          int _section;

          //! \note Required for Q_DECLARE_METATYPE.
          section_index();
          section_index (const QAbstractItemModel*, Qt::Orientation, int section);
          section_index (const QHeaderView*, int section);
          section_index (QModelIndex, Qt::Orientation);

          QVariant data (int role = Qt::DisplayRole) const;
          bool data (QVariant, int role = Qt::EditRole) const;

          bool operator< (const section_index&) const;
          bool operator== (const section_index&) const;
        };

        QDebug operator<< (QDebug, section_index);
        size_t hash_value (section_index);
        uint qHash (section_index);
      }
    }
  }
}

Q_DECLARE_METATYPE (fhg::util::qt::mvc::section_index)

#endif
