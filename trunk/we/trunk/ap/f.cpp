// Tom Niemann: "Operator Precedence Parsing."
// mirko.rahn@itwm.fraunhofer.de

#include <string>
#include <iostream>
#include <sstream>

#include <stdexcept>

#include <vector>
#include <stack>

#include <cstdlib>
#include <cassert>

#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

namespace parse
{
  class exception : public std::runtime_error
  {
  public:
    explicit exception (const std::string & msg)
      : std::runtime_error("parse error: " + msg) {}
  };

  class expected : public exception
  {
  public:
    explicit expected (const std::string & what)
      : exception ("excpected '" + what + "'") {}
  };

  namespace token
  {
    enum type
    { lt, le, gt, ge, ne, eq  // prec 0, left assoziative
    , add, sub                // prec 1, left assoziative
    , mul, div                // prec 2, left assoziative
    , mod                     // prec 3, left assoziative
    , pow                     // prec 4, right assoziative
    , neg                     // prec 5, unary minus
    , fac, com                // factorial, combinations
    , sep                     // comma
    , lpr, rpr                // parenthesis
    , val                     // value
    , ref                     // reference to context
    , eof
    };

    namespace function
    {
      template<typename T>
      static T lt (const T & l, const T & r) { return l < r; }
      template<typename T>
      static T le (const T & l, const T & r) { return l <= r; }
      template<typename T>
      static T gt (const T & l, const T & r) { return l > r; }
      template<typename T>
      static T ge (const T & l, const T & r) { return l >= r; }
      template<typename T>
      static T ne (const T & l, const T & r) { return l != r; }
      template<typename T>
      static T eq (const T & l, const T & r) { return l == r; }
      template<typename T>
      static T add (const T & l, const T & r) { return l + r; }
      template<typename T>
      static T sub (const T & l, const T & r) { return l - r; }
      template<typename T>
      static T mul (const T & l, const T & r) { return l * r; }
      template<typename T>
      static T div (const T & l, const T & r) { return l / r; }
      template<typename T>
      static T mod (const T & l, const T & r) { return l % r; }
      template<typename T>
      static T pow (const T & l, const T & r)
      {
        T p (1);

        for (T i (1); i < r; ++i)
          p *= l;

        return p; 
      }

      template<typename T>
      static T neg (const T & x) { return -x; }

      template<typename T>
      static T fac (const T & x) 
      {
        T f (1);

        for (T i (1); i < x; ++i)
          f *= i;

        return f;
      }

      template<typename T>
      static T com (const T & l, const T & r)
      {
        return (fac<T>(l)) / (fac<T>(l-r));
      }
    }

    static bool is_function (const type & token)
    {
      return (token > neg && token < sep);
    }

    static bool is_unary (const type & token)
    {
      return (token == neg || token == fac);
    }

    static bool is_prefix (const type & token)
    {
      return (token == fac || token == com);
    }

    static std::ostream & operator << (std::ostream & s, const type & token)
    {
      switch (token)
        {
        case lt: return s << " < ";
        case le: return s << " <= ";
        case gt: return s << " > ";
        case ge: return s << " >= ";
        case ne: return s << " != ";
        case eq: return s << " == ";
        case add: return s << " + ";
        case sub: return s << " - ";
        case mul: return s << " * ";
        case div: return s << " / ";
        case mod: return s << " % ";
        case pow: return s << "^";
        case neg: return s << "-";
        case fac: return s << "f";
        case com: return s << "c";
        case sep: return s << ", ";
        case lpr: return s << "(";
        case rpr: return s << ")";
        case val: return s << "<val>";
        case ref: return s << "<ref>";
        case eof: return s << "<eof>";
        default: throw std::runtime_error ("token");
        }
    }

    template<typename T>
    struct tokenizer
    {
    private:
      std::string::const_iterator pos;
      const std::string::const_iterator end;
      token::type token;
      T tokval;
      std::string::const_iterator refname_begin;
      std::string::const_iterator refname_end;

      void get (void)
      {
        while (pos != end && isspace(*pos))
          ++pos;

        if (pos == end)
          token = eof;
        else
          switch (*pos)
            {
            case '<':
              ++pos;
              if (pos == end)
                token = lt;
              else
                switch (*pos)
                  {
                  case '=': token = le; ++pos; break;
                  default: token = lt; break;
                  }
              break;
            case '>':
              ++pos;
              if (pos == end)
                token = gt;
              switch (*pos)
                {
                case '=': token = ge; ++pos; break;
                default: token = gt; break;
                }
              break;
            case '!':
              ++pos;
              if (pos == end)
                throw expected("=");
              else
                switch (*pos)
                  {
                  case '=': token = ne; ++pos; break;
                  default: throw expected ("="); break;
                  }
              break;
            case '=':
              ++pos;
              if (pos == end)
                throw expected("=");
              else
                switch (*pos)
                  {
                  case '=': token = eq; ++pos; break;
                  default: throw expected ("="); break;
                  }
              break;

            case '+': token = add; ++pos; break;
            case '-':
              if (token != val && token != rpr)
                token = neg;
              else
                token = sub;
              ++pos;
              break;
            case '*': token = mul; ++pos; break;
            case '/': token = div; ++pos; break;
            case '%': token = mod; ++pos; break;
            case '^': token = pow; ++pos; break;
            case 'f': token = fac; ++pos; break;
            case 'c': token = com; ++pos; break;
            case ',': token = sep; ++pos; break;
            case '(': token = lpr; ++pos; break;
            case ')': token = rpr; ++pos; break;
            case '$':
              token = ref;
              ++pos;
              if (pos == end)
                throw expected ("{");
              else
                switch (*pos)
                  {
                  case '{':
                    ++pos;
                    refname_begin = pos;
                    while (pos != end && *pos != '}')
                      ++pos;
                    if (*pos == '}')
                      refname_end = pos;
                    else
                      throw expected ("}");
                    ++pos;
                    break;
                  default: throw expected ("{"); break;
                  }
              break;
            default:
              token = val;
              tokval = 0;
              while (isdigit(*pos))
                {
                  tokval *= 10;
                  tokval += *pos - '0';
                  ++pos;
                }
              break;
            }
      }

      template<typename U>
      friend std::ostream & operator << (std::ostream &, const tokenizer<U> &);

    public:
      tokenizer (const std::string & input) 
        : pos (input.begin()), end (input.end()), token (eof) {}
      const T & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      std::string refname (void) const
      {
        return std::string (refname_begin, refname_end);
      }
    };

    template<typename T>
    static std::ostream & operator << (std::ostream & s, const tokenizer<T> & t)
    {
      switch (*t)
        {
        case val: return s << t();
        case ref: return s << "${" << t.refname() << "}";
        default: return s << *t;
        }
    }
  }

  namespace prec
  {
    typedef unsigned int type;

    static type prec (const token::type & token)
    {
      switch (token)
        {
        case token::lt:
        case token::le:
        case token::gt:
        case token::ge:
        case token::ne:
        case token::eq: return 0;
        case token::add:
        case token::sub: return 1;
        case token::mul:
        case token::div: return 2;
        case token::mod: return 3;
        case token::pow: return 4;
        case token::neg: return 5;
        default: throw std::runtime_error ("prec");
        }
    }
  }

  namespace associativity
  {
    enum type {left, right};

    static type associativity (const token::type & token)
    {
      switch (token)
        {
        case token::lt:
        case token::le:
        case token::gt:
        case token::ge:
        case token::ne:
        case token::eq:
        case token::add:
        case token::sub:
        case token::mul:
        case token::div:
        case token::mod: return left;
        case token::pow: return right;
        default: throw std::runtime_error ("associativity");
        }
    }
  }

  namespace action
  {
    enum type
    { shift
    , reduce
    , accept
    , error1
    , error2
    , error3
    , error4
    };

    static std::ostream & operator << (std::ostream & s, const type & action)
    {
      switch (action)
        {
        case shift: return s << "shift";
        case reduce: return s << "reduce";
        case accept: return s << "accept";
        case error1: return s << "error: missing right parenthesis";
        case error2: return s << "error: missing operator";
        case error3: return s << "error: unbalanced parenthesis";
        case error4: return s << "error: invalid function argument";
        default: throw std::runtime_error ("action");
        }
    }

    static type action (const token::type & top, const token::type inp)
    {
      if (top == token::lpr)
        {
          if (inp == token::eof)
            return error1;

          return shift;
        }

      if (top == token::rpr)
        {
          if (inp == token::lpr)
            return error2;
          
          if (inp == token::sep)
            return error4;

          if (token::is_function (inp))
            return error3;

          return reduce;
        }

      if (top == token::eof)
        {
          if (inp == token::eof)
            return accept;

          if (inp == token::rpr)
            return error3;

          if (inp == token::sep)
            return error4;

          return shift;
        }

      if (top == token::sep)
        {
          if (inp == token::sep)
            return error4;

          if (inp == token::eof)
            return error4;

          return reduce;
        }

      if (token::is_function (top))
        {
          if (inp == token::lpr)
            return shift;

          if (inp < token::lpr)
            return error4;

          return reduce;
        }

      assert (top < token::fac);

      if (inp == token::sep)
        return reduce;

      if (inp == token::lpr)
        return shift;

      if (inp == token::rpr)
        return reduce;

      if (inp == token::eof)
        return reduce;

      if (token::is_function (inp))
        return shift;

      assert (inp < token::fac);

      if (prec::prec (top) < prec::prec (inp))
        return shift;

      if (top == inp)
        {
          if (associativity::associativity (top) == associativity::left)
            return reduce;

          return shift;
        }

      return reduce;
    }
  }

  template<typename T>
  struct node_t
  {
    bool is_value;
    T value;

    bool is_refname;
    std::string refname;

    bool is_unary;
    bool is_binary;
    token::type token;
    node_t<T> * child0;
    node_t<T> * child1;

    node_t (const T & _value) 
      : is_value (true)
      , value (_value)
      , is_refname (false)
      , is_unary (false)
      , is_binary (false)
    {}

    node_t (const std::string & _refname)
      : is_value (false)
      , is_refname (true)
      , refname (_refname)
      , is_unary (false)
      , is_binary (false)
    {}

    node_t (const token::type & _token, node_t<T> * _child0)
      : is_value (false)
      , is_refname (false)
      , is_unary (true)
      , is_binary (false)
      , token (_token)
      , child0 (_child0)
    {}   

    node_t ( const token::type & _token
           , node_t<T> * _child0
           , node_t<T> * _child1
           )
      : is_value (false)
      , is_refname (false)
      , is_unary (false)
      , is_binary (true)
      , token (_token)
      , child0 (_child0)
      , child1 (_child1)
    {}   
  };

  template<typename T>
  std::ostream & operator << (std::ostream & s, const node_t<T> & nd)
  {
    if (nd.is_value)
      return s << nd.value;

    if (nd.is_refname)
      return s << "${" << nd.refname << "}";

    if (nd.is_unary)
      return s << nd.token << "(" << *(nd.child0) << ")";

    if (nd.is_binary)
      {
        if (token::is_prefix (nd.token))
          return s << nd.token 
                   << "(" << *(nd.child0) << ", " <<  *(nd.child1) << ")";

        return s << "(" << *(nd.child0) << nd.token << *(nd.child1) << ")";
      }

    assert (false);
  }

  template<typename T>
  struct parser_t
  {
  private:
    typedef node_t<T> nd_t;
    typedef std::stack<nd_t> nd_stack_t;
    typedef std::stack<token::type> op_stack_t;
    nd_stack_t nd_stack;
    op_stack_t op_stack;

    typedef boost::unordered_map<std::string,T> context_t;

    void unary ( const token::type & token
               , const boost::function<T (const T & x)> & f
               )
    {
      nd_t * c = new nd_t(nd_stack.top()); nd_stack.pop();

      if (c->is_value)
        {
          nd_stack.push (nd_t(f (c->value)));
          delete c;
        }
      else
        {
          nd_stack.push (nd_t (token, c));
        }
    }

    void binary ( const token::type & token
                , const boost::function<T (const T & l, const T & r)> & f
                )
    {
      nd_t * r = new nd_t(nd_stack.top()); nd_stack.pop();
      nd_t * l = new nd_t(nd_stack.top()); nd_stack.pop();

      if (l->is_value && r->is_value)
        {
          nd_stack.push (nd_t(f (l->value, r->value)));
          delete l;
          delete r;
        }
      else
        {
          nd_stack.push (nd_t (token, l, r));
        }
    }

    void reduce (void)
    {
      switch (op_stack.top())
        {
        case token::lt: binary (token::lt, token::function::lt<T>); break;
        case token::le: binary (token::le, token::function::le<T>); break;
        case token::gt: binary (token::gt, token::function::gt<T>); break;
        case token::ge: binary (token::ge, token::function::ge<T>); break;
        case token::ne: binary (token::ne, token::function::ne<T>); break;
        case token::eq: binary (token::eq, token::function::eq<T>); break;
        case token::add: binary (token::add, token::function::add<T>); break;
        case token::sub: binary (token::sub, token::function::sub<T>); break;
        case token::mul: binary (token::mul, token::function::mul<T>); break;
        case token::div: binary (token::div, token::function::div<T>); break;
        case token::mod: binary (token::mod, token::function::mod<T>); break;
        case token::pow: binary (token::pow, token::function::pow<T>); break;
        case token::neg: unary (token::neg, token::function::neg<T>); break;
        case token::fac: unary (token::fac, token::function::fac<T>); break;
        case token::com: binary (token::com, token::function::com<T>); break;
        case token::rpr: op_stack.pop(); break;
        default: break;
        }
      op_stack.pop();
    }

  public:
    parser_t (const std::string & input)
    {
      op_stack.push (token::eof);

      token::tokenizer<T> token (input);

      do
        {
          ++token;

          switch (*token)
            {
            case token::val: nd_stack.push (nd_t(token())); break;
            case token::ref: nd_stack.push (nd_t(token.refname())); break;
            default:
              {
              ACTION:
                action::type action (action::action (op_stack.top(), *token));

                switch (action)
                  {
                  case action::reduce:
                    reduce();
                    goto ACTION;
                    break;
                  case action::shift:
                    op_stack.push (*token);
                    break;
                  case action::accept:
                    break;
                  default:
                    {
                      std::ostringstream oss;
                      oss << action;
                      throw exception (oss.str());
                    }
                    break;
                  }
                break;
              }
            }
        }
      while (*token != token::eof);
    }

    const nd_t & operator * (void) const { return nd_stack.top(); }
  };
}

using std::cin;
using std::cout;
using std::endl;

int main (void)
{
//   std::string input 
//     ("4*f(2) + --1 - 4*(-13+25/${j}) + c(4,8) <= 4*(f(2)+--1) - 4*-13 + 25 / ${ii}");

  while (1)
    {
      std::string input;

      cin >> input;

      cout << input << endl;

      parse::parser_t<int> parser (input);

      cout << *parser << endl;
    }

  return EXIT_SUCCESS;
}
