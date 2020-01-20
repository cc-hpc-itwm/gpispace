#pragma once

#include <gspc/Forest.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <exception>

namespace gspc
{
  namespace interface
  {
    //! \todo what is base class, what is implementation specific!?
    //! likely: _resources, _resource_usage_by_id, add+remove base,
    //! NOT available_by_x
    //! _resources -> what about the lock
    //! _resource_usage_by_id -> is ref counting always needed?
    //! e.g. what if assert_singletons_only
    //! add+remove -> into/from what state?
    //! Alternative: Factor shared states
    class ResourceManager
    {
    public:
      using Resources = Forest<resource::ID, resource::Class>;

      virtual ~ResourceManager() = default;

      // - shall throw on duplicate id
      virtual void add (Resources) = 0;
      virtual void remove (Forest<resource::ID>) = 0;

      struct Interrupted : public std::exception{};
      //! \note once called all running and future acquire will throw
      //! Interrupted
      //! \todo Discuss this implies that a single resource manager can not be
      //! shared between multiple clients
      virtual void interrupt() = 0;
      //! \todo maybe return optional<...> instead of throwing Interrupted
      // virtual ? acquire (?)
      // virtual void release (?)

      //! \todo crash recovery: maybe record clients in order to
      //! release all resources acquired by a crashed client -> maybe
      //! required for interrupt anyways
    private:
      //! \todo
      // comm::runtime_system::resource_manager::Server _comm_server_for_runtime_system;
    };
  }
}
