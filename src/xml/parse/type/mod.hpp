#pragma once

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type/eureka.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct module_type : with_position_of_definition
      {
      public:
        module_type ( const util::position_type&
                    , const std::string& name
                    , const std::string& function
                    , const boost::optional<std::string>& target
                    , const boost::optional<std::string>& port_return
                    , const std::list<std::string>& port_arg
                    , boost::optional<std::string> _memory_buffer_return
                    , std::list<std::string> _memory_buffer_arg
                    , const boost::optional<std::string>& code
                    , const boost::optional<util::position_type>& pod_of_code
                    , const std::list<std::string>& cincludes
                    , const std::list<std::string>& ldflags
                    , const std::list<std::string>& cxxflags
                    , const boost::optional<bool> &pass_context
                    , const boost::optional<we::type::eureka_id_type> &eureka_id
                    , bool require_function_unloads_without_rest
                    , bool require_module_unloads_without_rest
                    );

        const std::string& name() const;
        const std::string& function() const;
        const boost::optional<std::string>& target() const;
        const boost::optional<std::string>& port_return() const;
        const std::list<std::string>& port_arg() const;
        const boost::optional<std::string>& memory_buffer_return() const;
        const std::list<std::string>& memory_buffer_arg() const;
        const boost::optional<std::string>& code() const;
        const boost::optional<util::position_type>
          position_of_definition_of_code() const;
        const std::list<std::string>& cincludes() const;
        const std::list<std::string>& ldflags() const;
        const std::list<std::string>& cxxflags() const;
        bool pass_context () const;
        const boost::optional<we::type::eureka_id_type>& eureka_id() const;
        bool require_function_unloads_without_rest() const;
        bool require_module_unloads_without_rest() const;

        bool operator== (const module_type&) const;

      private:
        std::string _name;
        std::string _function;
        boost::optional<std::string> _target;
        boost::optional<std::string> _port_return;
        std::list<std::string> _port_arg;
        boost::optional<std::string> _memory_buffer_return;
        std::list<std::string> _memory_buffer_arg;
        boost::optional<std::string> _code;
        boost::optional<util::position_type> _position_of_definition_of_code;
        std::list<std::string> _cincludes;
        std::list<std::string> _ldflags;
        std::list<std::string> _cxxflags;
        boost::optional<bool> _pass_context;
        boost::optional<we::type::eureka_id_type> _eureka_id;
        bool _require_function_unloads_without_rest;
        bool _require_module_unloads_without_rest;
      };

      std::size_t hash_value (const module_type&);

      namespace dump
      {
        std::string dump_fun (const module_type&);

        void dump (::fhg::util::xml::xmlstream&, const module_type&);
      }
    }
  }
}
