// bernd.loerwald@itwm.fraunhofer.de

#include <util-generic/print_exception.hpp>

#include <string>
#include <iostream>
#include <sstream>

#include <boost/serialization/access.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/enable_if.hpp>

// -----------------------------------------------------------------------

//! \note Serialize & deserialize serializable classes

/*
  namespace serialization
  {
    void serialize<Archive, T> (Archive& ar, const T& x);
    T deserialize<Archive, T> (Archive& ar);
  }

  // mark class TYPE as serializable
  // place inside of class TYPE
  #define SERIALIZE_MAKE_SERIALIZEABLE(TYPE)

  // begin definition of how to save/load class TYPE
  // place in same namespace as class TYPE
  // VARIABLE_NAME is to be used in following macros
  #define SERIALIZE_SAVE_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)
  #define SERIALIZE_LOAD_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)

  // save WHAT (shall depend on VARIABLE_NAME)
  #define SERIALIZE_SAVE_TO_ARCHIVE(WHAT)
  // load a variable VARIABLE_NAME of type TYPE
  #define SERIALIZE_LOAD_FROM_ARCHIVE(TYPE, VARIABLE_NAME)

  // construct object VARIABLE_NAME (given in LOAD_CONSTRUCT_DATA) of
  // TYPE with given ctor arguments
  #define SERIALIZE_CONSTRUCT(TYPE, VARIABLE_NAME, ...)
*/

namespace serialization
{
  namespace
  {
    //! \note Traits to check if T has is_properly_serializable_tag
    template<typename T> struct is_properly_serializable
    {
      typedef char yes[1];
      typedef char no [2];
      template <typename U> struct type_check;
      template <typename U> static yes &chk
        (type_check<char[sizeof(&U::__is_properly_serializable_tag)]> *);
      template <typename  > static no  &chk
        (...);
      BOOST_STATIC_CONSTANT (bool, value = sizeof (chk<T> (nullptr)) == sizeof (yes));
    };
  }

  template<typename T, typename Archive>
    void serialize ( Archive& ar, const T& x
                   , typename boost::enable_if_c<is_properly_serializable<T>::value>::type* = nullptr
                   )
  {
    const T* temp (&x);
    ar << temp;
  }
  template<typename T, typename Archive>
    void serialize ( Archive& ar, const T& x
                   , typename boost::disable_if_c<is_properly_serializable<T>::value>::type* = nullptr
                   )
  {
    ar << x;
  }

  template<typename T, typename Archive>
    T deserialize ( Archive& ar
                  , typename boost::enable_if_c<is_properly_serializable<T>::value>::type* = nullptr
                  )
  {
    typename boost::aligned_storage< sizeof (T)
                                   , boost::alignment_of<T>::value
                                   >::type buffer;
    T* temp (static_cast<T*> (static_cast<void*> (&buffer)));
    ar >> temp;
    return *temp;
  }
  template<typename T, typename Archive>
    T deserialize ( Archive& ar
                  , typename boost::disable_if_c<is_properly_serializable<T>::value>::type* = nullptr
                  )
  {
    T temp;
    ar >> temp;
    return temp;
  }
}


//! \note Macros to allow serialization for a class

#define SERIALIZE_MAKE_SERIALIZEABLE(TYPE)                                \
  public: BOOST_STATIC_CONSTANT (int, __is_properly_serializable_tag = 0);\
  private: friend class boost::serialization::access;                     \
  template <class Archive> void serialize (Archive & ar, unsigned int) {} \
  template<class Archive> friend void save_construct_data                 \
    (Archive&, const TYPE*, const unsigned int);                          \
  template<class Archive> friend void load_construct_data                 \
    (Archive&, const TYPE*, const unsigned int)

#define SERIALIZE_SAVE_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)            \
  template<class Archive> inline void save_construct_data                 \
    (Archive& _ARCHIVE, const TYPE* VARIABLE_NAME, const unsigned int)

#define SERIALIZE_LOAD_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)            \
  template<class Archive> inline void load_construct_data                 \
    (Archive& _ARCHIVE, TYPE* VARIABLE_NAME, const unsigned int)

#define SERIALIZE_SAVE_TO_ARCHIVE(WHAT)                                   \
  ::serialization::serialize (_ARCHIVE, WHAT)
#define SERIALIZE_LOAD_FROM_ARCHIVE(TYPE, VARIABLE_NAME)                  \
  TYPE VARIABLE_NAME (::serialization::deserialize<TYPE> (_ARCHIVE))
#define SERIALIZE_CONSTRUCT(TYPE, VARIABLE_NAME, ...)                     \
  ::new (VARIABLE_NAME) TYPE (__VA_ARGS__)

// -----------------------------------------------------------------------

class type
{
  SERIALIZE_MAKE_SERIALIZEABLE (type);

/*
public:
  static const int __is_properly_serializable_tag = 0;
private:
  friend class boost::serialization::access;
  template <class Archive> void serialize (Archive & ar, unsigned int) {}
  template<class Archive>
    friend void save_construct_data (Archive&, const type*, const unsigned int);
  template<class Archive>
    friend void load_construct_data (Archive&, const type*, const unsigned int);
  */

public:
  type() : _ ("DEFAULT CTOR") { std::cout << "type::type(): " << *this << std::endl; }
  type (const std::string& __) : _ (__) { std::cout << "type::type(std::string): " << *this << std::endl; }
  ~type() { std::cout << "type::~type(): " << *this << std::endl;}

  friend std::ostream& operator<< (std::ostream& os, const type&);

private:
  std::string _;
};

SERIALIZE_SAVE_CONSTRUCT_DATA_DEF (type, x)
{
  SERIALIZE_SAVE_TO_ARCHIVE (x->_);
}
SERIALIZE_LOAD_CONSTRUCT_DATA_DEF (type, x)
{
  SERIALIZE_LOAD_FROM_ARCHIVE (std::string, str);

  SERIALIZE_CONSTRUCT (type, x, str);
}

/*
  template<class Archive>
  inline void save_construct_data
    (Archive& _ARCHIVE, const type* x, const unsigned int)
  {
    ::serialization::serialize (_ARCHIVE, x->_);
  }
  template<class Archive>
  inline void load_construct_data
    (Archive& _ARCHIVE, type* x, const unsigned int)
  {
    std::string str (::serialization::deserialize<std::string> (_ARCHIVE));

    ::new (x) type (str);
  }
*/


class type_2
{
  SERIALIZE_MAKE_SERIALIZEABLE (type_2);

public:
  type_2 (const type& __) : _ (__) { std::cout << "type_2::type_2(type): " << *this << std::endl; }
  ~type_2() { std::cout << "type_2::~type_2(): " << *this << std::endl;}

  friend std::ostream& operator<< (std::ostream& os, const type_2&);

private:
  type _;
};

SERIALIZE_SAVE_CONSTRUCT_DATA_DEF (type_2, x)
{
  SERIALIZE_SAVE_TO_ARCHIVE (x->_);
}
SERIALIZE_LOAD_CONSTRUCT_DATA_DEF (type_2, x)
{
  SERIALIZE_LOAD_FROM_ARCHIVE (type, _);

  SERIALIZE_CONSTRUCT (type_2, x, _);
}


#include <boost/serialization/shared_ptr.hpp>

class type_3
{
  SERIALIZE_MAKE_SERIALIZEABLE (type_3);

public:
  type_3 (const boost::shared_ptr<type_2>& __) : _ (__) { std::cout << "type_3::type_3(boost::shared_ptr<type_2>): " << *this << std::endl; }
  ~type_3() { std::cout << "type_3::~type_3(): " << *this << std::endl;}

  friend std::ostream& operator<< (std::ostream& os, const type_3&);

private:
  boost::shared_ptr<type_2> _;
};

SERIALIZE_SAVE_CONSTRUCT_DATA_DEF (type_3, x)
{
  SERIALIZE_SAVE_TO_ARCHIVE (x->_);
}
SERIALIZE_LOAD_CONSTRUCT_DATA_DEF (type_3, x)
{
  SERIALIZE_LOAD_FROM_ARCHIVE (boost::shared_ptr<type_2>, _);

  SERIALIZE_CONSTRUCT (type_3, x, _);
}



std::ostream& operator<< (std::ostream& os, const type& x)
{
  return os << "type {" << x._ << "}";
}
std::ostream& operator<< (std::ostream& os, const type_2& x)
{
  return os << "type_2 {" << x._ << "}";
}
std::ostream& operator<< (std::ostream& os, const type_3& x)
{
  return os << "type_3 {" << x._ << ": " << *x._ << "}";
}



#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

int main (int, char**)
try
{
  std::stringstream ss;

  {
    type_3 _ (boost::shared_ptr<type_2> (new type_2 (type())));


    boost::archive::text_oarchive ar_out (ss);
    ::serialization::serialize (ar_out, _);
  }

  std::cout << "\n";

  {
    boost::archive::text_iarchive ar_in (ss);
    const type_3 _ (::serialization::deserialize<type_3> (ar_in));
  }

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
  return 1;
}
