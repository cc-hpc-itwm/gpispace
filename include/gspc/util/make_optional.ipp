#include <optional>



#define FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_...)                  \
    gspc::util::detail::make_optional (cond_, [&] { return how_; })

    namespace gspc::util::detail
    {
      template<typename Fun>
        auto make_optional (bool cond, Fun&& fun)
          -> std::optional<decltype (fun())>
      {
        using T = std::optional<decltype (fun())>;
        return cond ? T (fun()) : T();
      }
    }
