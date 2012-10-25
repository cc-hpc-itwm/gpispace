// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MOD_HPP
#define _XML_PARSE_TYPE_MOD_HPP

#include <xml/parse/id/mapper.fwd.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/type/function.fwd.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> port_args_type;
      typedef std::list<std::string> cincludes_type;
      typedef std::list<std::string> flags_type;
      typedef std::list<std::string> links_type;

      struct mod_type
      {
      public:
        mod_type ( const id::module& id
                 , const id::function& parent
                 , id::mapper* id_mapper
                 );
        mod_type ( const id::module& id
                 , const id::function& parent
                 , id::mapper* id_mapper
                 , const std::string & _name
                 , const std::string & _function
                 , const boost::filesystem::path & path
                 );

        const id::module& id() const;
        const id::function& parent() const;

        bool is_same (const mod_type& other) const;

        bool operator == (const mod_type& other) const;

        void sanity_check (const function_type & fun) const;

        friend std::size_t hash_value (const mod_type&);

        std::string name;
        std::string function;
        fhg::util::maybe<std::string> port_return;
        port_args_type port_arg;

        fhg::util::maybe<std::string> code;
        cincludes_type cincludes;
        flags_type ldflags;
        flags_type cxxflags;
        links_type links;
        boost::filesystem::path path;

      private:
        id::module _id;
        id::function _parent;
        id::mapper* _id_mapper;
      };

      typedef boost::unordered_map<std::string, mod_type> mc_by_function_type;
      typedef boost::unordered_map<std::string, mc_by_function_type> mcs_type;

      std::size_t hash_value (const mod_type& m);

      namespace dump
      {
        std::string dump_fun (const mod_type & m);

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const mod_type & m
                  );
      }
    }
  }
}

#endif
