#pragma once



    namespace gspc::util::serialization
    {
      namespace detail
      {
        template<typename T>
          struct base_class_t {};
      }

      //! Tag type for marking a base class in serialization generators.
      template<typename T>
        constexpr static detail::base_class_t<T> base_class() { return {}; }
    }
