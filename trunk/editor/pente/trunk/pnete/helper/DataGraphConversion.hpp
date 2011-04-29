#ifndef HELPERDATAGRAPHCONVERSION_HPP
#define HELPERDATAGRAPHCONVERSION_HPP 1

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition;
      class Port;
      class Connection;
    }
    namespace data
    {
      class Transition;
      class Port;
      class Connection;
    }
    namespace helper
    {
      class DataGraphConversion
      {
        public:
          static data::Transition* transitionFromGraph(const graph::Transition&);
          static data::Port* portFromGraph(const graph::Port&);
          static data::Connection* connectionFromGraph(const graph::Connection&);
          
          static graph::Transition* transitionFromData(const data::Transition&);
          static graph::Port* portFromData(const data::Port&);
          static graph::Connection* connectionFromData(const data::Connection&);
      };
    }
  }
}

#endif
