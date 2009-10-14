#ifndef SDPA_TOKEN_HPP
#define SDPA_TOKEN_HPP 1

#include <cassert> // assert
#include <ostream>
#include <stdexcept> // logic_error

#include <sdpa/memory.hpp>
#include <sdpa/Properties.hpp>

namespace sdpa { namespace wf {
  /**
    A class representing a Token.

    A Token just transports arbitrary data through a workflow.
   */
  class Token : public sdpa::Properties {
    public:
      typedef std::string data_t;

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
      }

      std::string data_as() const throw (std::logic_error)
      {
        return data();
      }

      /**
        Try to convert the stored data to the given type.
        */
      template<typename T> inline T data_as() const throw (std::logic_error) {
        T val;
        std::istringstream istr(data());
        istr >> val;
        if (! istr.good()) throw std::logic_error(std::string("could not convert data") + (typeid(T).name()));
        return val;
      }

    public:
      Token()
        : Properties()
        , data_("")
      {}

      explicit
      Token(const data_t & some_data)
        : Properties()
        , data_(some_data)
      {}

      template <typename T>
      Token(T some_data)
        : Properties()
        , data_("")
      {
        std::ostringstream ostr;
        ostr << some_data;
        data_ = ostr.str();
      }

      Token(const Token & other)
        : Properties()
        , data_(other.data())
      { }

      const Token & operator=(const Token & rhs) {
        if (this != &rhs)
        {
          data(rhs.data());
        }
        return *this;
      }

      void writeTo(std::ostream &os) const {
        if (data().empty()) {
          os << "Token()";
        } else {
          os << "Token(\""<< data() << "\")";
        }
      }
    private:
      data_t data_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Token &t);

#endif // SDPA_TOKEN_HPP
