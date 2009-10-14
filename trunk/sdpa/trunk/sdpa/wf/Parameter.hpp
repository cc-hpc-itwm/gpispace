#ifndef SDPA_PARAMETER_HPP
#define SDPA_PARAMETER_HPP 1

#include <string>
#include <ostream>

#include <sdpa/memory.hpp>
#include <sdpa/wf/Token.hpp>

namespace sdpa { namespace wf {
  /**
    This class describes the parameter to an activity.

    A Parameter can either be an input or an output  parameter  for  a  function
    call.  Parameters constist of  a  Token  transporting  the  actual  data,  a
    parameter-name   and   an   edge    type    for    further    optimizations.
   */
  class Parameter {
    public:
      typedef shared_ptr<Parameter> ptr_t;

      /**
        The type of the edge this parameter belongs to.
        
        \todo{should be moved to another file and maybe into an own class-hierarchy}
        */
      typedef enum {
        INPUT_EDGE = 0,
        READ_EDGE,
        OUTPUT_EDGE,
        WRITE_EDGE,
        EXCHANGE_EDGE,
        UPDATE_EDGE
      } EdgeType;

      /**
        Create a new Parameter with the given components.

        @param token the Token holding the data, usually for input data
        @param name the parameter name (or edge description)
        @param edge_type the type of the edge this parameter belongs to
       */
      Parameter(const std::string &name, EdgeType edge_type, const Token & token);

      Parameter(const Parameter &);
      Parameter & operator=(const Parameter &);

      ~Parameter() {}

      inline const std::string & name() const { return name_; }
      inline const Token & token() const { return token_; }
      inline Token & token() { return token_; }
      inline EdgeType edge_type() const { return edge_type_; }

      void writeTo(std::ostream &) const;
    private:
      std::string name_;
      EdgeType edge_type_;
      Token token_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Parameter &p);
#endif
