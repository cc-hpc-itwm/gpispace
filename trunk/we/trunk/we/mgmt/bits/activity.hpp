/*
 * =====================================================================================
 *
 *       Filename:  activity.hpp
 *
 *    Description:  implements a generic activity descriptor
 *
 *        Version:  1.0
 *        Created:  03/25/2010 12:00:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_ACTIVITY_HPP
#define WE_MGMT_BITS_ACTIVITY_HPP 1

#include <boost/unordered_set.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename Id, typename ModuleCall, typename Net, typename Expression>
  class activity
  {
    typedef Id id_type;
    typedef ModuleCall module_call_type;
    typedef Net net_type;
    typedef Expression expression_type;

    typedef typename net_type::input_t input_t;
    typedef typename net_type::output_t output_t;
    typedef typename net_type::output_descr_t output_descr_t;

    typedef boost::unordered_set<id_type> children_set_t;

    enum ActivityType
    {
      MODULE_CALL = 0
    , EXPRESSION
    , DISTRIBUTED_NET
    , SEQUENTIAL_NET
    };

    union
    {
      module_call_type *module_call;
      net_type *net;
      expression_type *expression;
    } data;

    struct
    {
      bool suspended : 1;
      bool cancelling : 1;
      bool cancelled : 1;
      bool distributed : 1;
    } flags;

    // lockable concept implementation
    void lock()
    {
      mutex_.lock();
    }

    bool try_lock()
    {
      return mutex_.try_lock();
    }

    void unlock()
    {
      mutex_.unlock();
    }

    // timed lockable concept implementation
    bool timed_lock(boost::system_time const& abs_time)
    {
      return mutex_.timed_lock(abs_time);
    }

    template<typename DurationType> bool timed_lock(DurationType const& rel_time)
    {
      return mutex_.timed_lock(rel_time);
    }

    // shared lockable concept implementation
    void lock_shared()
    {
      mutex_.lock_shared();
    }

    bool try_lock_shared()
    {
      return mutex_.try_lock_shared();
    }

    bool timed_lock_shared(boost::system_time const& abs_time)
    {
      return mutex_.timed_lock_shared(abs_time);
    }

    template<typename DurationType> bool timed_lock_shared(DurationType const& rel_time)
    {
      return mutex_.timed_lock_shared(rel_time);
    }

    void unlock_shared()
    {
      mutex_.unlock_shared();
    }

  private:
    ActivityType type_;
    id_type id_;
    mutable boost::shared_mutex mutex_;
    children_set_t children_;
    id_type parent_;
  };
}}}

#endif
