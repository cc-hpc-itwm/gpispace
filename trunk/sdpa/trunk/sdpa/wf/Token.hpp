#ifndef SDPA_TOKEN_HPP
#define SDPA_TOKEN_HPP 1
/***********************************************************************/
/** @file Token.hpp
 *
 * $Id:$
 *
 *
 *
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

namespace sdpa { namespace wf {
  /**
    A class representing a Token.

    A Token just transports arbitrary data through a workflow.
   */
  class Token : public sdpa::Properties {
    public:
      typedef boost::any any_t; //!< the storage type we use for the transported data

      /**
        Assigns different data to this token.

        @param [in] the new data
       */
      void reset(const any_t & some_data) {
        data_ = some_data;
        initialized_ = true;
      }

      /**
        Returns true iff the Token has been initialized.
      */
      inline bool initialized() const {
        return initialized_;
      }

      /**
        Retrieve the data of the token.
        @return the stored data
      */
      const any_t & data() const {
        return data_;
      }

      /**
        Try to convert the stored data to the given type.
        */
      template<typename T> inline T as() const {
        return boost::any_cast<T>(data_);
      }

      /**
        Store a new value into the storage space.
      */
      template<typename T> inline void store_data(T val) {
        data_ = val;
      }

      virtual ~Token() {}

    public:
      Token()
        : data_(any_t()), initialized_(false) {}
      Token(const any_t & some_data)
        : data_(some_data), initialized_(true) { }
      template <typename T>
      Token(T some_data)
        : data_(some_data), initialized_(true) { }

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
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Token &t);

#endif // SDPA_TOKEN_HPP
