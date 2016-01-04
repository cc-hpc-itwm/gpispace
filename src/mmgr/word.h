
#pragma once

#include <inttypes.h>

  typedef unsigned long Word_t, *PWord_t;
  typedef unsigned long Size_t, *PSize_t;

  typedef Size_t Offset_t, *POffset_t;
  typedef Word_t Key_t, *PKey_t;
  typedef Word_t Value_t, *PValue_t;
  typedef Size_t Handle_t, *PHandle_t;

  typedef Size_t (*fUser_t) (PValue_t);

  static const fUser_t fUserNone = (fUser_t) nullptr;
