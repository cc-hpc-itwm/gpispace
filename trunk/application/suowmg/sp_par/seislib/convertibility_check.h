#ifndef CONVERTIBILITY_CHECK
#define CONVERTIBILITY_CHECK

template <class T, class U>
    class Conversion
{
  typedef char Small;             // sizeof(Small) = 1
  class Big {char dummy[2]; };    // sizeof(Big) > 1  
  static Small Test(U);
  static Big Test(...);
  static T MakeT();
  
  public:
    enum {exists = sizeof(Test(MakeT())) == sizeof(Small) };
    enum { sameType = false };
};

template <class T>
    class Conversion<T,T>
{
  public:
    enum { exists = 1, sameType = 1 };
};



#define INHERITS(T,U) \
  ( Conversion<const U *, const T *>::exists && \
   !Conversion<const T *, const void *>::sameType) 
   
#define INHERITS_STRICT(T,U) \
  ( INHERITS(T,U) && \
   !Conversion<const T, const U>::sameType) 
   
#endif