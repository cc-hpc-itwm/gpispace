/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  typedefinitions
 *
 *        Version:  1.0
 *        Created:  11/15/2009 04:49:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULES_TYPES_HPP
#define SDPA_MODULES_TYPES_HPP 1

#include <list>
#include <map>
#include <cassert> // assert
#include <ostream>
#include <stdexcept> // logic_error

#ifdef HAVE_CONFIG_H
#include <sdpa/sdpa-config.hpp>
#endif

#include <sdpa/memory.hpp>
#include <sdpa/util/Properties.hpp>

#include <fhglog/fhglog.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>

namespace sdpa { namespace wf {
class Token {
    public:
      typedef std::string data_t;
      typedef sdpa::util::Properties properties_t;

      virtual ~Token() {}

      /**
        Retrieve the data of the token.
        @return the stored data
      */
      inline const data_t & data() const {
        return data_;
      }
      inline data_t & data()
      {
        return data_;
      }

      inline void data(const data_t &d)
      {
        data_ = d;
        properties().put("datatype", typeid(d).name());
      }

      template <typename T> inline void data(const T &some_data)
      {
        std::ostringstream ostr;
        ostr << some_data;
        data_ = ostr.str();
        properties().put("datatype", typeid(T).name());
      }

      data_t data_as() const throw (std::logic_error)
      {
        return data();
      }

      /**
        Try to convert the stored data to the given type.
        */
      template<typename T> inline T data_as() const throw (std::logic_error) {
#if defined(ENABLE_TYPE_CHECKING) && (ENABLE_TYPE_CHECKING == 1)
        if (this->properties().get("datatype", "") != typeid(T).name()) {
          throw std::logic_error(std::string("type mismatch occured: expected:")+this->properties().get("datatype", "")+ " got:"+typeid(T).name() + " data:"+data_);
        }
#endif
        T val;
        std::istringstream istr(data());
        istr >> val;
        if (! istr) throw std::logic_error(std::string("could not convert data to type: ") + (typeid(T).name()));
        return val;
      }

    public:
      Token()
        : data_("")
      {
        properties().put("datatype", "unknown");
      }

      explicit
      Token(const data_t & some_data)
        : data_(some_data)
      {
        properties().put("datatype", typeid(some_data).name());
      }

      template <typename T>
      explicit
      Token(T some_data)
        : data_("")
      {
        std::ostringstream ostr;
        ostr << some_data;
        data_ = ostr.str();
        properties().put("datatype", typeid(some_data).name());
      }

      Token(const Token & other)
        : data_(other.data())
        , properties_(other.properties())
      {
      }

      const Token & operator=(const Token & rhs) {
        if (this != &rhs)
        {
          data(rhs.data());
          properties_ = rhs.properties();
        }
        return *this;
      }

      inline properties_t & properties() { return properties_; }
      inline const properties_t & properties() const { return properties_; }
      void writeTo(std::ostream &os) const {
		/* Token-Format:
		 *
		 * control:
		 *    {token, true|false, []}
		 * data:
		 *    {token, {data, ""}, []}
		 * */

        os << "{";
        os << "token, ";
        if (properties().get<std::string>("datatype", "unknown") == ("control"))
		{
          os << std::ios::boolalpha;
          os << properties().get<bool>("control");
          os << std::ios::boolalpha;
        }
        else
        {
          os << "{"
             << "data"
             << ", "
             << "\""
             << data()
             << "\""
             << "}";
        }
		/*
        os << ", ";
		os << properties();
	    */
		os << "}";
      }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int file_version )
	{
    	ar & data_;
		ar & properties_;
	}

    private:
		data_t data_;
		properties_t properties_;
  };

inline std::ostream & operator<<(std::ostream & os, const sdpa::wf::Token &t)
{
  t.writeTo(os);
  return os;
}


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

      Parameter &set_name(const std::string &new_name) { name_ = new_name; return *this; }

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

namespace sdpa { namespace modules {
  class IModule;

  typedef sdpa::wf::Parameter parameter_t;
  typedef std::map<std::string, parameter_t> data_t;

  typedef void (*InitFunction)(IModule*);
  typedef void (*GenericFunction)(data_t&);

  typedef std::list<std::string> param_names_list_t;
  typedef std::pair<GenericFunction, param_names_list_t> parameterized_function_t;
}}

#endif
