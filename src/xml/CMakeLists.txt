gspc_add_library (NAME gspc_xml_parse
  TYPE STATIC
  SOURCES "error.cpp"
          "parser.cpp"
          "state.cpp"
          "warning.cpp"
          "type/connect.cpp"
          "type/eureka.cpp"
          "type/expression.cpp"
          "type/function.cpp"
          "type/memory_buffer.cpp"
          "type/memory_transfer.cpp"
          "type/mod.cpp"
          "type/multi_mod.cpp"
          "type/net.cpp"
          "type/place.cpp"
          "type/place_map.cpp"
          "type/port.cpp"
          "type/preferences.cpp"
          "type/require.cpp"
          "type/response.cpp"
          "type/specialize.cpp"
          "type/struct.cpp"
          "type/template.cpp"
          "type/transition.cpp"
          "type/use.cpp"
          "type/with_position_of_definition.cpp"
          "util/cdata.cpp"
          "util/expect.cpp"
          "util/mk_fstream.cpp"
          "util/name_element.cpp"
          "util/optional.cpp"
          "util/position.cpp"
          "util/property.cpp"
          "util/required.cpp"
          "util/show_node_type.cpp"
          "util/skip.cpp"
          "util/valid_name.cpp"
          "util/validprefix.cpp"
          "util/validstructfield.cpp"
          "util/weparse.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  LIBRARIES gspc_we
            gspc_util
            GPISpace::header-only
            rapidxml
            Boost::program_options
)
