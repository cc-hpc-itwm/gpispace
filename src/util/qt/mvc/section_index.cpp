// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mvc/section_index.hpp>

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <QDebug>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        section_index::section_index()
          : _model (NULL)
          , _orientation (Qt::Horizontal)
          , _section (-1)
        { }

        section_index::section_index ( const QAbstractItemModel* model
                                     , Qt::Orientation orientation
                                     , int section
                                     )
          : _model (const_cast<QAbstractItemModel*> (model))
          , _orientation (orientation)
          , _section (section)
        { }

        section_index::section_index (const QHeaderView* view, int section)
          : _model (view->model())
          , _orientation (view->orientation())
          , _section (section)
        { }

        section_index::section_index (QModelIndex index, Qt::Orientation orientation)
          : _model (const_cast<QAbstractItemModel*> (index.model()))
          , _orientation (orientation)
          , _section (_orientation == Qt::Horizontal ? index.column() : index.row())
        { }

        QVariant section_index::data (int role) const
        {
          return _model->headerData (_section, _orientation, role);
        }
        bool section_index::data (QVariant val, int role) const
        {
          return _model->setHeaderData (_section, _orientation, val, role);
        }

        bool section_index::operator< (const section_index& other) const
        {
          return boost::tie (_model, _orientation, _section)
               < boost::tie (other._model, other._orientation, other._section);
        }
        bool section_index::operator== (const section_index& other) const
        {
          return boost::tie (_model, _orientation, _section)
              == boost::tie (other._model, other._orientation, other._section);
        }

        QDebug operator<< (QDebug d, section_index index)
        {
          return d << index._model << index._orientation << index._section;
        }

        size_t hash_value (section_index index)
        {
          size_t seed (0);
          boost::hash_combine (seed, index._model);
          boost::hash_combine (seed, index._orientation);
          boost::hash_combine (seed, index._section);
          return seed;
        }

        uint qHash (section_index index)
        {
          return static_cast<uint> (hash_value (index));
        }
      }
    }
  }
}
