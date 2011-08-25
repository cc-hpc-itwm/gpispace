#ifndef DATAPORT_HPP
#define DATAPORT_HPP 1

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Port
      {
        public:
          Port(const QString& name = QString(), const QString& type = QString(), bool notConnectable = false);

          const QString& name() const;
          const QString& type() const;
          const bool& notConnectable() const;

        private:
          QString _name;
          QString _type;
          bool _notConnectable;
      };
      QDataStream& operator<<(QDataStream& stream, const Port& port);
      QDataStream& operator>>(QDataStream& stream, Port& port);
    }
  }
}

#endif
