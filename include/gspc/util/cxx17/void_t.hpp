#pragma once

//! \note n3911: TransformationTrait Alias `void_t`



    namespace gspc::util::cxx17
    {
      template<typename...> struct make_void { using type = void; };
      template<typename... Ts> using void_t
        = typename make_void<Ts...>::type;
    }
