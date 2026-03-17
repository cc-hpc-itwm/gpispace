#define GSPC_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, cond_, templ_)       \
  namespace gspc                                                               \
  {                                                                            \
    namespace testing                                                          \
    {                                                                          \
      namespace detail                                                         \
      {                                                                        \
        template<templ_>                                                       \
          struct random_impl<type_, cond_>                                     \
          {                                                                    \
            type_ operator()() const;                                          \
          };                                                                   \
      }                                                                        \
    }                                                                          \
  }

#define GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE_IMPL(type_)                      \
  GSPC_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, void, )                    \
  inline type_ gspc::testing::detail::random_impl<type_, void>                 \
    ::operator()() const

#define GSPC_TESTING_RANDOM_SPECIALIZE_FULL_IMPL(type_, cond_, templ_...)      \
  GSPC_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, cond_, templ_)             \
  template<templ_>                                                             \
    inline type_ gspc::testing::detail::random_impl<type_, cond_>              \
      ::operator()() const
