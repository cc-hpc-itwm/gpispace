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

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/
#include <cassert>
#include <cstring> // memcpy

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * File scope Variables (Variables share by several functions in
 *                       the same file )
 *
 *---------------------------------------------------------------------*/
namespace sdpa {
  class Token {
    public:
      typedef char byte_t;
      typedef byte_t* data_t;
      typedef std::size_t length_t;

      // is there a way to avoid "virtual" for this function?
      // i.e. is it possible to somehow overwrite the implemenation in a subclass depending on the template parameter
      // regular data tokens just return initialized_, control token return their boolean value
      inline bool operator!() const {
        return initialized_;
      };

      void reset(const data_t data, length_t len) {
        assert(data != 0); assert(len > 0);

        if (data_)
          delete [] data_;
        data_ = new byte_t[len];
        length_ = len;
        memcpy(data_, data, length_);
        initialized_ = true;
      }

      inline bool initialized() const {
        return initialized_;
      }

      const data_t data() const {
        return data_;
      }
      const length_t length() const {
        return length_;
      }

      template<typename T> const  T & as() {
        assert(sizeof(T) == length_);
        return *(static_cast<T*>((void*)data_));
      }

      template<typename T> void store(const T &val) {
        assert(sizeof(T) == length_);
        memcpy(data_, &val, sizeof(T));
      }

      ~Token() {
        if (data_) {
          delete [] data_;
          data_ = 0;
        }
      }

    public:
      Token()
        : data_(0), length_(0), initialized_(false) {}
      Token(data_t data, length_t len)
        : data_(0), length_(0), initialized_(true) {
        reset(data, len);
      }

      Token(const Token & other) {
        reset(other.data(), other.length());
      }

      const Token & operator=(const Token & rhs) {
        reset(rhs.data(), rhs.length());
        return *this;
      }

    private:
      data_t data_;
      std::size_t length_;
      bool initialized_;
  };

  template <typename T>
    class DataToken : public Token {
      public:
        typedef T value_t;
        explicit
          DataToken (const value_t &arg) : Token((Token::data_t)&arg, sizeof(value_t)) { }

        virtual ~DataToken() {}
    };

  typedef DataToken<bool> ControlToken;
  typedef DataToken<int> IntToken;
  //    typedef DataToken<VMMemoryToken> MemoryToken;
}

/*---------------------------------------------------------------------*
 * External Variables
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Extern Functions declarations
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Functions declarations
 *
 *---------------------------------------------------------------------*/

