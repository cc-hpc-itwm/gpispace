#ifndef SDPA_TOKEN_HPP
#define SDPA_TOKEN_HPP 1

#include <cassert> // assert
#include <ostream>
#include <stdexcept> // logic_error

#include <sdpa/sdpa-config.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/Properties.hpp>

namespace sdpa { namespace wf {
  /**
    A class representing a Token.

    A Token just transports arbitrary data through a workflow.
   */
  class Token {
    public:
      typedef std::string data_t;
      typedef sdpa::Properties properties_t;

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
#ifdef ENABLE_TYPE_CHECKING
        type_ = typeid(T).name();
#endif
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
        if (typeid(T).name() != type_) {
          throw std::logic_error(std::string("type mismatch occured: expected:")+type_+ " got:"+typeid(T).name() + " data:"+data_);
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
#ifdef ENABLE_TYPE_CHECKING
        , type_(typeid(some_data).name())
#endif
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
#ifdef ENABLE_TYPE_CHECKING
        type_ = typeid(T).name();
#endif
        properties().put("datatype", typeid(some_data).name());
      }

      Token(const Token & other)
        : data_(other.data())
#ifdef ENABLE_TYPE_CHECKING
        , type_(other.type_)
#endif
        , properties_(other.properties())
      {
      }

      const Token & operator=(const Token & rhs) {
        if (this != &rhs)
        {
          data(rhs.data());
#ifdef ENABLE_TYPE_CHECKING
          type_ = rhs.type_;
#endif
          properties_ = rhs.properties();
        }
        return *this;
      }

      inline properties_t & properties() { return properties_; }
      inline const properties_t & properties() const { return properties_; }
      void writeTo(std::ostream &os) const {
        if (data().empty()) {
          os << "Token()";
        } else {
          os << "Token(" << properties().get("datatype") << ":\""<< data() << "\")";
        }
      }
    private:
      data_t data_;
#ifdef ENABLE_TYPE_CHECKING
      std::string type_;
#endif
      properties_t properties_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Token &t);

#endif // SDPA_TOKEN_HPP
