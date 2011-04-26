#ifndef HELPERXMLQUERY_HPP
#define HELPERXMLQUERY_HPP 1

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace helper
    {
      class XMLQuery
      {
        public:
          XMLQuery(const QString& path);
          
          bool exec(const QString& xpath, QString* result) const;
          bool exec(const QString& xpath, QStringList* result) const;
          
        private:
          QString _path;
      };
    }
  }
}

#endif
