#ifndef GUARD_SYMBOL_H_e861df17cd1c4e309b7be28d396457ca
#define GUARD_SYMBOL_H_e861df17cd1c4e309b7be28d396457ca

#ifndef SYMBOL_VALUE
#define SYMBOL_VALUE 0x0D06F00D
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAKE_NAME(PRE,POST)\
  GUARD_##PRE_##POST

#define MAKE_SYMBOL(VAL) \
  MAKE_NAME(SYMBOL, VAL)

  extern const int MAKE_SYMBOL(SYMBOL_VALUE);

#ifdef __cplusplus
}
#endif

#endif
