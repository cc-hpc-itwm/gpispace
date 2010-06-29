#ifndef WE_MGMT_DETAIL_BASIC_CONTEXT_HPP
#define WE_MGMT_DETAIL_BASIC_CONTEXT_HPP 1

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      template <typename R = void>
      struct basic_context
      {
      public:
        typedef R result_type;
      };
    }
  }
}

#endif
