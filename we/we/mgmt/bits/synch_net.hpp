// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_MGMT_LAYER_BITS_SYNCH_NET_HPP
#define _WE_MGMT_LAYER_BITS_SYNCH_NET_HPP 1

#include <boost/thread.hpp>
#include <boost/random.hpp>

#include <iostream>

namespace we { namespace mgmt { namespace detail {

  // sequentialize critical accesses to the net
  template<typename NET, typename CNT = unsigned long>
  struct synch_net
  {
  private:
    typedef typename NET::activity_t activity_t;
    typedef typename NET::output_t output_t;

    NET & n;
    CNT _extract;
    CNT _inject;

    mutable boost::shared_mutex mutex;
    boost::mt19937 engine;

  public:
    synch_net (NET & _net)
      : n (_net)
      , _extract(0)
      , _inject(0)
    {}

	template <typename Output>
    void inject (Output const & output)
    {
      boost::unique_lock<boost::shared_mutex> lock (mutex);

      n.inject_activity_result (output);

      ++_inject;
    }

    activity_t extract (void)
    {
      boost::unique_lock<boost::shared_mutex> lock (mutex);

      activity_t act = n.extract_activity_random (engine);

      ++_extract;

      return act;
    }

    bool done (void) const
    {
      boost::shared_lock<boost::shared_mutex> lock (mutex);

      return (_extract == _inject && not n.can_fire());
    }

    bool can_fire (void) const
    {
      boost::shared_lock<boost::shared_mutex> lock (mutex);

      return n.can_fire();
    }

  private:
	synch_net (const synch_net & other);
	synch_net & operator= (const synch_net & other);
  };
}}}

#endif
