// Tom Niemann: "Operator Precedence Parsing."
// mirko.rahn@itwm.fraunhofer.de

#include <util/timer.hpp>

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
#include <boost/bind.hpp>

namespace parse
{
  template<typename T>
  std::string show (const T & x)
  {
    std::ostringstream s; s << x; return s.str();
  }

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
      : exception ("expected '" + what + "'") {}
  };

  namespace token
  {
    enum type
    { _or                     // prec  0, left associative
    , _and                    // prec  1, left associative
    , _not                    // prec 30, right associative
    , lt, le, gt, ge, ne, eq  // prec 10, left associative
    , add, sub                // prec 21, left associative
    , mul, div                // prec 22, left associative
    , mod                     // prec 23, left associative
    , pow                     // prec 24, right associative
    , neg                     // prec 25, unary minus
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
      static T unary (const type & token, const T & x)
      {
        switch (token)
          {
          case _not: return !x;
          case neg: return -x;
          case fac:
            {
              T f (1);

              for (T i (1); i <= x; ++i)
                f *= i;

              return f;
            }
          default: throw std::runtime_error ("unary " + show(token));
          }
      }

      template<typename T>
      static T binary (const type & token, const T & l, const T & r)
      {
        switch (token)
          {
          case _or: return l | r;
          case _and: return l & r;
          case lt: return l < r;
          case le: return l <= r;
          case gt: return l > r;
          case ge: return l >= r;
          case ne: return l != r;
          case eq: return l == r;
          case add: return l + r;
          case sub: return l - r;
          case mul: return l * r;
          case div:
            if (r == 0) throw exception ("divide by zero"); return l / r;
          case mod: return l % r;
          case pow:
            {
              T p (1);

              for (T i (0); i < r; ++i)
                p *= l;
              
              return p; 
            }
          case com: return (unary<T>(fac, l)) / (unary<T>(fac, l-r));
          default: throw std::runtime_error ("binary " + show(token));
          }
      }

      static bool is_builtin_function (const type & token)
      {
        return (token > neg && token < sep);
      }
    } // namespace function

    static bool is_prefix (const type & token)
    {
      return (token == fac || token == com || token == _not);
    }

    static std::ostream & operator << (std::ostream & s, const type & token)
    {
      switch (token)
        {
        case _or: return s << " | ";
        case _and: return s << " & ";
        case _not: return s << "!";
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
        default: throw std::runtime_error ("token " + show(token));
        }
    }

    static bool can_be_unary (const type & token)
    {
      return (token != val && token != rpr && token != ref);
    }

    template<typename T>
    struct tokenizer
    {
    private:
      std::string::const_iterator pos;
      const std::string::const_iterator end;
      token::type token;
      T tokval;
      std::string _refname;

      void get (void)
      {
        while (pos != end && isspace(*pos))
          ++pos;

        if (pos == end)
          token = eof;
        else
          switch (*pos)
            {
            case '|': token = _or; ++pos; break;
            case '&': token = _and; ++pos; break;
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
                throw expected("= or expression");
              else
                switch (*pos)
                  {
                  case '=': token = ne; ++pos; break;
                  default:
                    if (can_be_unary (token))
                      token = _not;
                    else
                      throw exception ("misplaced negation");
                    break;
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
              if (can_be_unary (token))
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
                    _refname.clear();
                    while (pos != end && *pos != '}')
                      {
                        _refname.push_back (*pos);
                        ++pos;
                      }
                    if (*pos != '}')
                      throw expected ("}");
                    ++pos;
                    break;
                  default: throw expected ("{"); break;
                  }
              break;
            default:
              if (!isdigit(*pos))
                throw expected ("digit");
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
      std::string refname (void) const { return _refname; }
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
  } // namespace token

  namespace prec
  {
    typedef unsigned int type;

    static type prec (const token::type & token)
    {
      switch (token)
        {
        case token::_or: return 0;
        case token::_and: return 1;
        case token::_not: return 30;
        case token::lt:
        case token::le:
        case token::gt:
        case token::ge:
        case token::ne:
        case token::eq: return 10;
        case token::add:
        case token::sub: return 21;
        case token::mul:
        case token::div: return 22;
        case token::mod: return 23;
        case token::pow: return 24;
        case token::neg: return 25;
        default: throw std::runtime_error ("prec " + show(token));
        }
    }
  } // namespace prec

  namespace associativity
  {
    enum type {left, right};

    static type associativity (const token::type & token)
    {
      switch (token)
        {
        case token::_or:
        case token::_and: return left;
        case token::_not: return right;
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
        case token::pow: 
        case token::neg: return right;
        default: throw std::runtime_error ("associativity " + show(token));
        }
    }
  } // namespace associativity

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
        default: throw std::runtime_error ("action " + show(action));
        }
    }

    static type action (const token::type & top, const token::type & inp)
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

          if (token::function::is_builtin_function (inp))
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

      if (token::function::is_builtin_function (top))
        {
          if (inp == token::lpr)
            return shift;

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

      if (token::function::is_builtin_function (inp))
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

      if (top == token::_not && inp == token::neg)
        return shift;

      return reduce;
    }
  } // namespace action

  namespace node
  {
    template<typename T>
    struct type
    {
      bool is_value;
      T value;

      bool is_refname;
      std::string refname;

      bool is_unary;
      bool is_binary;
      token::type token;
      type<T> * child0;
      type<T> * child1;
      
      type (const T & _value) 
        : is_value (true)
        , value (_value)
        , is_refname (false)
        , is_unary (false)
        , is_binary (false)
      {}

      type (const std::string & _refname)
        : is_value (false)
        , is_refname (true)
        , refname (_refname)
        , is_unary (false)
        , is_binary (false)
      {}

      type (const token::type & _token, type<T> * _child0)
        : is_value (false)
        , is_refname (false)
        , is_unary (true)
        , is_binary (false)
        , token (_token)
        , child0 (_child0)
      {}   

      type (const token::type & _token, type<T> * _child0, type<T> * _child1)
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
    std::ostream & operator << (std::ostream & s, const type<T> & nd)
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

      return s;
    }
  } // namespace node

  namespace eval
  {
    class missing_binding : public std::runtime_error
    {
    public:
      explicit missing_binding (const std::string & name)
        : std::runtime_error ("missing binding for: ${" + name + "}") {};
    };

    static unsigned long cnt_bind (0);
    static unsigned long cnt_value (0);

    template<typename T>
    struct context
    {
    private:
      boost::unordered_map<std::string,T> container;
    public:
      context (void) { cnt_bind = cnt_value = 0; }
      ~context (void)
      {
        std::cout << "bind " << cnt_bind << " value " << cnt_value << std::endl;
      }
      void bind (const std::string & name, const T & value)
      {
        ++cnt_bind; container[name] = value;
      }
      const T & value (const std::string & name) const
      {
        ++cnt_value;

        typename boost::unordered_map<std::string,T>::const_iterator
          it (container.find (name));

        if (it == container.end())
          throw missing_binding (name);
        else
          return it->second;
      }
      void clear () { container.clear(); }
    };

    template<typename T>
    T eval (const node::type<T> & node, const context<T> & context)
    {
      if (node.is_value)
        return node.value;

      if (node.is_refname)
        return context.value (node.refname);

      if (node.is_unary)
        return token::function::unary<T> ( node.token
                                         , eval (*node.child0, context)
                                         );

      if (node.is_binary)
        return token::function::binary<T> ( node.token
                                          , eval (*node.child0, context)
                                          , eval (*node.child1, context)
                                          );

      assert (false);

      return T();
    }
  } // namespace eval

  template<typename T>
  node::type<T> refnode_value ( const eval::context<T> & context
                              , const std::string & name
                              )
  {
    return node::type<T>(context.value(name));
  }

  template<typename T>
  node::type<T> refnode_name (const std::string & name) 
  {
    return node::type<T>(name);
  }

  template<typename T>
  struct parser
  {
  private:
    typedef node::type<T> nd_t;
    typedef std::stack<nd_t> nd_stack_t;
    typedef std::stack<token::type> op_stack_t;
    nd_stack_t nd_stack;
    op_stack_t op_stack;

    void unary (const token::type & token)
    {
      if (nd_stack.empty())
        throw exception ("unary operator missing operand: " + show(token));

      nd_t * c = new nd_t(nd_stack.top()); nd_stack.pop();

      if (c->is_value)
        {
          nd_stack.push (nd_t(token::function::unary (token, c->value)));
          delete c;
        }
      else
        {
          nd_stack.push (nd_t (token, c));
        }
    }

    void binary (const token::type & token)
    {
      if (nd_stack.empty())
        throw exception ("binary operator missing operand r: " + show(token));

      nd_t * r = new nd_t(nd_stack.top()); nd_stack.pop();

      if (nd_stack.empty())
        throw exception ("binary operator missing operand l: " + show(token));

      nd_t * l = new nd_t(nd_stack.top()); nd_stack.pop();

      if (l->is_value && r->is_value)
        {
          nd_stack.push 
            (nd_t(token::function::binary (token, l->value, r->value)));
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
        case token::_or:
        case token::_and: binary (op_stack.top()); break;
        case token::_not: unary (op_stack.top()); break;
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
        case token::mod:
        case token::pow: binary (op_stack.top()); break;
        case token::neg:
        case token::fac: unary (op_stack.top()); break;
        case token::com: binary (op_stack.top()); break;
        case token::rpr: op_stack.pop(); break;
        default: break;
        }
      op_stack.pop();
    }

    void parse ( const std::string & input
               , const boost::function<nd_t (const std::string &)> & refnode
               )
    {
      op_stack.push (token::eof);

      token::tokenizer<T> token (input);

      do
        {
          ++token;

          switch (*token)
            {
            case token::val: nd_stack.push (nd_t(token())); break;
            case token::ref: nd_stack.push (refnode(token.refname())); break;
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
                    throw exception (show(action));
                  }
                break;
              }
            }
        }
      while (*token != token::eof);
    }

  public:
    parser (const std::string & input, const eval::context<T> & context)
    {
      parse (input, boost::bind (refnode_value<T>, boost::ref(context), _1));
    }

    parser (const std::string & input)
    {
      parse (input, boost::bind (refnode_name<T>, _1));
    }

    const nd_t & operator * (void) const { return nd_stack.top(); }

    const T eval (const eval::context<T> & context)
    {
      return eval::eval (this->operator * (), context);
    }
  };
}

using std::cin;
using std::cout;
using std::endl;

int main (void)
{
//   std::string input 
//     ("4*f(2) + --1 - 4*(-13+25/${j}) + c(4,8) <= 4*(f(2)+--1) - 4*-13 + 25 / ${ii}");

  {
    parse::eval::context<int> context;
    std::string input;

    while (getline(cin, input).good())
      switch (input[0])
        {
        case '#':
          context.clear();
          cout << "context deleted" << endl;
          break;
        case ':':
          {
            std::string::const_iterator pos (input.begin());
            const std::string::const_iterator end (input.end());

            ++pos;

            while (pos != end && isspace(*pos))
              ++pos;

            std::string name;

            while (pos != end && *pos != '=' && !isspace(*pos))
              {
                name.push_back (*pos);
                ++pos;
              }

            while (pos != end && isspace(*pos))
              ++pos;

            if (*pos != '=')
              cout << "parse error: syntax: name = value" << endl;

            ++pos;

            while (pos != end && isspace(*pos))
              ++pos;

            int value(0);

            while (isdigit(*pos))
              {
                value *= 10;
                value += *pos - '0';
                ++pos;
              }

            cout << "bind: " << name << " = " << value << endl;

            context.bind (name, value);
          }
          break;
        default:
          try
            {
              parse::parser<int> parser (input);
              
              cout << "parsed expression: " << *parser << endl;

              try
                {
                  cout << "evaluated value: " << parser.eval (context) << endl;
                }
              catch (parse::eval::missing_binding e)
                {
                  cout << e.what() << endl;
                }
            }
          catch (parse::exception e)
            {
              cout << e.what() << endl;
            }
        }
  }

  cout << "measure..." << endl;

  {
    const unsigned int round (1000);
    const unsigned int max (1000);
    const std::string input ("${i} < ${max}");

    {
      Timer_t timer ("parse once, evaluate often", max * round);

      parse::eval::context<unsigned int> context;

      context.bind("max",max);

      parse::parser<unsigned int> parser (input);

      for (unsigned int r (0); r < round; ++r)
        {
          unsigned int i (0);

          context.bind ("i",i);

          while (parser.eval (context))
            context.bind ("i",++i);
        }
    }

    {
      Timer_t timer ("parse with evaluate often", max * round);

      parse::eval::context<unsigned int> context;

      context.bind("max",max);

      for (unsigned int r (0); r < round; ++r)
        {
          unsigned int i (0);

          do
            context.bind ("i",i++);
          while (parse::parser<unsigned int>(input, context).eval (context));
        }
    }
  }

  return EXIT_SUCCESS;
}
