#pragma once

#include <gspc/StencilCache.hpp>
#include <gspc/stencil_cache/callback.hpp>
#include <gspc/stencil_cache/types.hpp>

#include <we/expr/eval/context.hpp>
#include <we/type/value.hpp>

#include <util-generic/dynamic_linking.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace we
{
  namespace type
  {
    struct scoped_neighbors_callback
    {
      scoped_neighbors_callback (std::string, std::vector<char> const&);
      ~scoped_neighbors_callback();

      using Coordinate = gspc::stencil_cache::Coordinate;

      std::list<Coordinate> operator() (Coordinate) const;

    private:
      fhg::util::scoped_dlhandle _implementation;
      void* _state;
      decltype (gspc_stencil_cache_callback_neighbors)* _neighbors;
    };

    struct stencil_cache
    {
      using SCache = gspc::StencilCache< gspc::stencil_cache::Stencil
                                       , gspc::stencil_cache::Coordinate
                                       , gspc::stencil_cache::Slot
                                       , gspc::stencil_cache::Counter
                                       >;

      using PutToken =
        std::function<void (std::string, pnet::type::value::value_type)>;

      stencil_cache (expr::eval::context const&, PutToken);
      ~stencil_cache();

      void alloc (expr::eval::context const&);
      void prepared (expr::eval::context const&);
      void free (expr::eval::context const&);

    private:
      using Allocate = std::pair<gspc::stencil_cache::Stencil, SCache::Inputs>;
      using QAllocate = fhg::util::interruptible_threadsafe_queue<Allocate>;

      std::string _place_prepare;
      std::string _place_ready;
      pnet::type::value::value_type _input_memory;
      unsigned long _input_size;
      gspc::stencil_cache::Slot _M;
      PutToken _put_token;
      scoped_neighbors_callback _neighbors;
      SCache _scache;
      QAllocate _queue_allocate;
      std::thread _allocate;
    };

    struct stencil_caches
    {
      void operator() (expr::eval::context const&, stencil_cache::PutToken);

    private:
      std::unordered_map<unsigned long, stencil_cache> _;
    };
  }
}
