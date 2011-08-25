#ifndef HELPERTRAVERSERRECEIVER_HPP
#define HELPERTRAVERSERRECEIVER_HPP 1

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Transition;
      class Connection;
    }
    namespace helper
    {
      class TraverserReceiver
      {
        public:
          TraverserReceiver();
          virtual ~TraverserReceiver();

          virtual void transitionFound(const data::Transition& transition);
          virtual void connectionFound(const data::Connection& connection);
      };
    }
  }
}

#endif
