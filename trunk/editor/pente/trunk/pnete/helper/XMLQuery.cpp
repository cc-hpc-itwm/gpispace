#include "XMLQuery.hpp"

#include <QXmlQuery>
#include <QVariant>
#include <QStringList>

namespace fhg
{
  namespace pnete
  {
    namespace helper
    {      
      XMLQuery::XMLQuery(const QString& path)
      : _path(path)
      {
        //! \todo Detection of zero byte files?
      }
      
      bool XMLQuery::exec(const QString& xpath, QString* result) const
      {
        QXmlQuery query;
        query.bindVariable("path", QVariant(_path));
        query.setQuery("doc($path)" + xpath + "/string()");
        QString temp;
        if(query.isValid() && query.evaluateTo(&temp))
        {
          // why in the name of god is there a newline at the end? Oo
          *result = temp.remove("\n");
          return true;
        }
        return false;
      }
      
      bool XMLQuery::exec(const QString& xpath, QStringList* result) const
      {
        QXmlQuery query;
        query.bindVariable("path", QVariant(_path));
        query.setQuery("doc($path)" + xpath + "/string()");
        QStringList temp;
        if(query.isValid() && query.evaluateTo(&temp))
        {
          *result = temp;
          return true;
        }
        return false;
      }
    }
  }
}
