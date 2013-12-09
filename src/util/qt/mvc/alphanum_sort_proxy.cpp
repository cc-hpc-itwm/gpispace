// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mvc/alphanum_sort_proxy.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <QDate>
#include <QDateTime>
#include <QTime>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        alphanum_sort_proxy::alphanum_sort_proxy
          (QAbstractItemModel* model, QObject* parent)
            : sort_filter_proxy (parent)
        {
          setSourceModel (model);
        }

        namespace
        {
          int locale_aware_compare_proxy
            (const QString& l, const QString& r, const Qt::CaseSensitivity)
          {
            //! \note case sensitivity ignored on purpose: Qt does not support it.
            return l.localeAwareCompare (r);
          }
          int compare_proxy
            (const QString& l, const QString& r, const Qt::CaseSensitivity cs)
          {
            return l.compare (r, cs);
          }

          //! \note This is a variation of fhg/util/alphanum.hpp,
          //! adapted for QString
          int alphanum_compare ( const QString& lhs
                               , const QString& rhs
                               , const Qt::CaseSensitivity case_sensitivity
                               , const bool locale_aware
                               )
          {
            const boost::function<int (const QString&, const QString&)> compare
              ( boost::bind ( locale_aware
                            ? &locale_aware_compare_proxy
                            : &compare_proxy
                            , _1
                            , _2
                            , case_sensitivity
                            )
              );

            enum mode_t { STRING, NUMBER } mode (STRING);

            QString::const_iterator l (lhs.begin());
            QString::const_iterator r (rhs.begin());

            while (l != lhs.end() && r != rhs.end())
            {
              if (mode == STRING)
              {
                while (l != lhs.end() && r != rhs.end())
                {
                  const bool l_digit (l->isNumber());
                  const bool r_digit (r->isNumber());

                  if (l_digit && r_digit)
                  {
                    mode = NUMBER;
                    break;
                  }

                  const int diff (compare (*l, *r));
                  if (diff) return diff < 0 ? -1 : 1;

                  ++l;
                  ++r;
                }
              }
              else
              {
                QString::const_iterator l_end (l);
                while (l_end != lhs.end() && l_end->isNumber()) ++l_end;
                const qlonglong l_int (QString (&*l, int (l_end - l)).toLongLong());
                l = l_end;

                QString::const_iterator r_end (r);
                while (r_end != rhs.end() && r_end->isNumber()) ++r_end;
                const qlonglong r_int (QString (&*r, int (r_end - r)).toLongLong());
                r = r_end;

                const qlonglong diff (l_int - r_int);
                if (diff) return diff < 0 ? -1 : 1;

                mode = STRING;
              }
            }

            if (r != rhs.end()) return -1;
            if (l != lhs.end()) return +1;
            return 0;
          }
        }

        //! \note This is 90% copied from the base class, as there is
        //! no way to extend the QString/default case only.
        bool alphanum_sort_proxy::lessThan
          (const QModelIndex& lhs, const QModelIndex& rhs) const
        {
          const QVariant l (lhs.data (sortRole()));
          const QVariant r (rhs.data (sortRole()));

          switch (l.type())
          {
          case QVariant::Invalid:
            return (r.type() == QVariant::Invalid);
          case QVariant::Int:
            return l.toInt() < r.toInt();
          case QVariant::UInt:
            return l.toUInt() < r.toUInt();
          case QVariant::LongLong:
            return l.toLongLong() < r.toLongLong();
          case QVariant::ULongLong:
            return l.toULongLong() < r.toULongLong();
          case QVariant::Double:
            return l.toDouble() < r.toDouble();
          case QVariant::Char:
            return l.toChar() < r.toChar();
          case QVariant::Date:
            return l.toDate() < r.toDate();
          case QVariant::Time:
            return l.toTime() < r.toTime();
          case QVariant::DateTime:
            return l.toDateTime() < r.toDateTime();

          case QVariant::String:
          default:
            return alphanum_compare ( l.toString()
                                    , r.toString()
                                    , sortCaseSensitivity()
                                    , isSortLocaleAware()
                                    ) < 0;
          }
        }
      }
    }
  }
}
