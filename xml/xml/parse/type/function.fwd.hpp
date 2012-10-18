// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_FWD_HPP
#define _XML_PARSE_TYPE_FUNCTION_FWD_HPP

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // typedef xml::util::unique<port_type>::elements_type ports_type;
      // typedef std::list<std::string> conditions_type;
      // typedef xml::util::unique<function_type>::elements_type functions_type;
      // typedef xml::util::unique<function_type>::elements_type templates_type;
      // typedef xml::util::unique<specialize_type>::elements_type specializes_type;

      template<typename Net>
        class function_resolve;

      template<typename Net, typename Fun>
        class function_specialize;

      template<typename Net, typename Fun>
        class function_sanity_check;

      template<typename Net>
        class function_type_check;

      template<typename Net>
        class function_distribute_function;

      template<typename Activity, typename Net, typename Fun>
        class function_synthesize;

      class function_is_net;

      struct function_type;

      struct fun_info_type;

      // typedef boost::unordered_set<fun_info_type> fun_infos_type;

      // typedef boost::unordered_map<std::string,fun_infos_type> fun_info_map;

      // typedef boost::filesystem::path path_t;

      namespace visitor
      {
        template<typename NET, typename TRANS>
          class transition_find_module_calls;

        struct port_with_type;

        template<typename NET>
          class find_module_calls;

        class transition_struct_to_cpp;

        template<typename NET>
          class struct_to_cpp_visitor;
      }

      namespace dump
      {
        namespace visitor
        {
          template<typename NET>
            class function_dump;
        }
      } // namespace dump
    }
  }
}

#endif
