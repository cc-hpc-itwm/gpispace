#ifndef SDPA_TOKEN_HPP
#define SDPA_TOKEN_HPP 1

#include <cassert> // assert
#include <ostream>
#include <stdexcept> // logic_error

#ifdef HAVE_CONFIG_H
#include <sdpa/sdpa-config.hpp>
#endif

#include <sdpa/memory.hpp>
#include <sdpa/util/Properties.hpp>

#include <fhglog/fhglog.hpp>

namespace sdpa { namespace wf {
  /**
    A class representing a Token.

    A Token just transports arbitrary data through a workflow.
   */
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
    private:
      data_t data_;
      properties_t properties_;
  };
}}

inline std::ostream & operator<<(std::ostream & os, const sdpa::wf::Token &t)
{
  t.writeTo(os);
  return os;
}

#endif // SDPA_TOKEN_HPP
