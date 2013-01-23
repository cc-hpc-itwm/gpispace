// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_STRUCT_FWD_HPP
#define _XML_PARSE_TYPE_STRUCT_FWD_HPP

// #include <we/type/signature/types.hpp>

// #include <list>

// #include <boost/unordered/unordered_map_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct structure_type;

      // typedef std::list<struct_t> structs_type;
    }

    namespace structure_type
    {
      // typedef boost::unordered_map< signature::field_name_t
      //                             , type::struct_t
      //                             > set_type;
      // typedef boost::unordered_map< signature::field_name_t
      //                             , std::string
      //                             > forbidden_type;

      class get_literal_type_name;

      class resolve;
    }
  }
}

#endif
