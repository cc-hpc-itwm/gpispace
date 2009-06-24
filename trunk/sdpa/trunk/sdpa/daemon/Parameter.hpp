#ifndef SDPA_PARAMETER_HPP
#define SDPA_PARAMETER_HPP 1

#include <string>
#include <ostream>
#include <sdpa/daemon/Token.hpp>

namespace sdpa { namespace daemon {
  /**
    This class describes the parameter to an activity.

    A Parameter can either be an input or an output  parameter  for  a  function
    call.  Parameters constist of  a  Token  transporting  the  actual  data,  a
    parameter-name   and   an   edge    type    for    further    optimizations.
   */
  class Parameter {
    public:
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
      Parameter(const Token & token, const std::string & name, EdgeType edge_type);

      /**
        Construct an output Parameter with an empty token.

        \todo{an input parameter should never be created with an empty token}

        @param name the parameter name (or edge description)
        @param edge_type the type of the edge this parameter belongs to
       */
      Parameter(const std::string & name, EdgeType edge_type);
      Parameter(const Parameter &);
      const Parameter & operator=(const Parameter &);

      ~Parameter() {}

      inline const std::string & name() const { return name_; }
      inline const Token & token() const { return token_; }
      inline Token & token() { return token_; }
      inline EdgeType edge_type() const { return edge_type_; }

      void writeTo(std::ostream &) const;
    private:
      Token token_;
      std::string name_;
      EdgeType edge_type_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::daemon::Parameter &p);
#endif
