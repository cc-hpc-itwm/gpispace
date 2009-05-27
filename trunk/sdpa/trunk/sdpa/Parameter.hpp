#ifndef SDPA_PARAMETER_HPP
#define SDPA_PARAMETER_HPP 1

#include <string>
#include <ostream>
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
      inline const Token & token() const { return token_; }
      inline Token & token() { return token_; }
      inline EdgeType edge_type() const { return edge_type_; }

      void writeTo(std::ostream &) const;
    private:
      Token token_;
      std::string name_;
      EdgeType edge_type_;
  };
}

extern std::ostream & operator<<(std::ostream & os, const sdpa::Parameter &p);
#endif
