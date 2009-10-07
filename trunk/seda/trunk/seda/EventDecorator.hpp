#ifndef SEDA_EVENT_DECORATOR_HPP
#define SEDA_EVENT_DECORATOR_HPP 1

#include <seda/IEvent.hpp>

namespace seda {
  class EventDecorator : public IEvent {
  public:
    EventDecorator(IEvent *e) : event(e) {}
    virtual ~EventDecorator() {
      if (event) {
	delete event; event = 0;
      }
    }

    virtual void visit(EventQueue* queue) {
      event->visit(queue);
    }

    virtual std::string str() const {
      return event->str();
    }
  private:
    IEvent* event;
  };
}

#endif // !SEDA_EVENT_DECORATOR_HPP
