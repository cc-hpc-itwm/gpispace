#ifndef SDPA_PLUGIN_OBSERVER_HPP
#define SDPA_PLUGIN_OBSERVER_HPP 1

#include <list>
#include <boost/any.hpp>

namespace observe
{
  class Observable;
  class Observer
  {
    typedef std::list<Observable*> observed_list_t;
  public:
    virtual ~Observer()
    {
      stop_to_observe();
    }

    virtual void notify(const Observable*, boost::any const &) = 0;
  protected:
    void start_to_observe(observe::Observable* o)
    {
      m_observed.push_back (o);
      o->add_observer(this);
    }

    void stop_to_observe(observe::Observable* o)
    {
      o->del_observer(this);
      m_observed.remove(o);
    }

    void stop_to_observe()
    {
      while (not m_observed.empty())
      {
        stop_to_observe(m_observed.front());
      }
    }
  private:
    observed_list_t m_observed;
  };
}

#endif
