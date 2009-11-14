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
      Parameter(const std::string &a_name, EdgeType a_edge_type, const Token & a_token)
        : name_(a_name)
          , edge_type_(a_edge_type)
          , token_(a_token)
    { }

      Parameter()
        : name_("unknown")
          , edge_type_(OUTPUT_EDGE)
          , token_()
    { }

      Parameter(const Parameter &other)
        : name_(other.name())
          , edge_type_(other.edge_type())
          , token_(other.token())
    { }

      Parameter & operator=(const Parameter &rhs)
      {
        if (this != &rhs)
        {
          name_ = rhs.name();
          edge_type_ = rhs.edge_type();
          token_ = rhs.token();
        }
        return *this;
      }

      ~Parameter() {}

      inline const std::string & name() const { return name_; }
      inline std::string & name() { return name_; }
      inline const Token & token() const { return token_; }
      inline Token & token() { return token_; }
      inline const EdgeType & edge_type() const { return edge_type_; }
      inline EdgeType & edge_type() { return edge_type_; }

      void writeTo(std::ostream &os, bool verbose = true) const
      {
        if (verbose)
        {
          os << "{"
             << "param"
             << ","
             << name()
             << ","
             << edge_type_to_char_code(edge_type())
             << ","
             << token()
             << "}";
        }
        else
        {
          os << edge_type_to_char_code(edge_type())
             << ":"
             << name()
             << "="
             << token().data();
        }
      }
    private:
      std::string name_;
      EdgeType edge_type_;
      Token token_;

      char edge_type_to_char_code(EdgeType etype) const
      {
        switch (etype)
        {
          case INPUT_EDGE:
            return 'i';
          case READ_EDGE:
            return 'r';
          case OUTPUT_EDGE:
            return 'o';
          case WRITE_EDGE:
            return 'w';
          default:
            LOG(ERROR, "got a strange edge type: " << etype);
            throw std::runtime_error("unhandled edge type");
        }
      }
  };
}}

inline std::ostream & operator<<(std::ostream & os, const sdpa::wf::Parameter &p)
{
  p.writeTo(os);
  return os;
}

#endif
