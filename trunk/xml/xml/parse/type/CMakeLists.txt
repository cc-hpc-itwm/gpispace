set (hdrs
  connect.hpp
  expression.hpp
  function.hpp
  mod.hpp
  net.hpp
  place.hpp
  place_map.hpp
  port.hpp
  require.hpp
  specialize.hpp
  struct.hpp
  token.hpp
  transition.hpp
  use.hpp
  )

install (FILES ${hdrs}
  DESTINATION include/xml/parse/type
  COMPONENT headers)
