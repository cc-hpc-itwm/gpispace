#pragma once

#include <functional>
#include <utility>


  namespace gspc::util
  {
    namespace
    {
      template<typename This, typename Memfn>
        struct this_bound_mem_fn
      {
        This* _this;
        Memfn _fun;

        template<typename... Args>
          typename Memfn::result_type operator() (Args&&... args) const
        {
          return _fun (_this, std::forward<Args> (args)...);
        }
      };
    }

    template<typename This, typename Fun>
      auto bind_this (This* that, Fun&& fun)
        -> this_bound_mem_fn
             <This, decltype (std::mem_fn (std::forward<Fun> (fun)))>
    {
      return {that, std::mem_fn (std::forward<Fun> (fun))};
    }
  }
