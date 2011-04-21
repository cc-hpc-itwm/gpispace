#ifndef NODES_H
#define NODES_H

#include <vector>
#include <QString>

//! \todo Templates

//! \todo Types / structs
typedef QString Type;

struct Place
{
  QString name;
  QString type;
//  std::map<QString,QString> properties;
//! \todo Token
};

struct Port
{
  QString name;
  QString type;
  QString place;
};

struct Connection
{
  QString port;
  QString place;
};

struct Function
{
  std::vector<Port> in;
  std::vector<Port> out;
  QString expression;
};

struct Transition
{
  QString name;
  Function function;
  std::vector<Connection> in;
  std::vector<Connection> out;
};

struct Net
{
  std::vector<Type> types;
  std::vector<Place> places;
  std::vector<Transition> transitions;
};

#endif // NODES_H
