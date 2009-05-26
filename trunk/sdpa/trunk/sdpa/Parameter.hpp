#ifndef SDPA_PARAMETER_HPP
#define SDPA_PARAMETER_HPP 1

#include <string>
#include <sdpa/Token.hpp>

namespace sdpa {
  class Parameter {
  public:
    typedef enum {
      INPUT_EDGE = 0,
      READ_EDGE,
      OUTPUT_EDGE,
      WRITE_EDGE,
      EXCHANGE_EDGE,
      UPDATE_EDGE
    } EdgeType;

    Parameter(const Token & token, const std::string & name, EdgeType edge_type);
    Parameter(const Parameter &);
    const Parameter & operator=(const Parameter &);

    ~Parameter() {}

    inline const std::string & name() const { return name_; }
    inline const Token & const_token() const { return token_; }
    inline Token & token() { return token_; }
    inline EdgeType edge_type() const { return edge_type_; }
  private:
    Token token_;
    std::string name_;
    EdgeType edge_type_;
  };
}

#endif
