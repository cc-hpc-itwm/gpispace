#ifndef WE_MGMT_CONTEXT_HPP
#define WE_MGMT_CONTEXT_HPP 1

namespace we
{
  namespace mgmt
  {
    template <typename R = void>
    struct context
    {
    public:
      typedef R result_type;
    };
  }
}

#endif
