#ifndef SDPA_TOKEN_HPP
#define SDPA_TOKEN_HPP 1
/***********************************************************************/
/** @file Token.hpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-05-25
 *  @email  kai.krueger@itwm.fhg.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

#include <cassert> // assert
#include <cstring> // memcpy
#include <ostream>

#include <boost/any.hpp>
#include <sdpa/Properties.hpp>

namespace sdpa {
  class Token : public sdpa::Properties {
    public:
      typedef boost::any any_t;

      void reset(const any_t & data) {
        data_ = data;
        initialized_ = true;
      }

      inline bool initialized() const {
        return initialized_;
      }

      const any_t & data() const {
        return data_;
      }

      template<typename T> inline T as() const {
        return boost::any_cast<T>(data_);
      }

      template<typename T> inline void store_data(T val) {
        data_ = val;
      }

      virtual ~Token() {}

    public:
      Token()
        : data_(any_t()), initialized_(false) {}
      Token(const any_t & data)
        : data_(data), initialized_(true) { }
      template <typename T>
      Token(T data)
        : data_(data), initialized_(true) { }

      Token(const Token & other)
        : data_(other.data()), initialized_(true) {
      }

      const Token & operator=(const Token & rhs) {
        reset(rhs.data());
        return *this;
      }

      void writeTo(std::ostream &os) const {
        if (data_.empty()) {
          os << "<empty>";
        } else {
          os << "<replace me with token data>";
        }
      }
    private:
      any_t data_;
      bool initialized_;
  };
}

std::ostream & operator<<(std::ostream & os, const sdpa::Token &t);

#endif // SDPA_TOKEN_HPP
