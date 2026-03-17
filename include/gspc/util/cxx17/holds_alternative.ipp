#include <gspc/util/functor_visitor.hpp>



    namespace gspc::util::cxx17
    {
      template<typename T, typename... Ts>
        bool holds_alternative (::boost::variant<Ts...> const& variant) noexcept
      {
        return ::gspc::util::visit<bool>
          ( variant
          , [] (T const&) { return true; }
          , [] (auto const&) { return false; }
          );
      }
    }
