// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#define SAVE_TO_ARCHIVE(WHAT)                   \
  _archive << (WHAT)

//! \note Archive<<(int) e.g. is broken and can't handle values
//! returned from a function call, thus needs a temporary.
#define SAVE_TO_ARCHIVE_WITH_TEMPORARY(T, WHAT) \
  {                                             \
    const T temp (WHAT);                        \
    _archive << temp;                           \
  }

#define LOAD_FROM_ARCHIVE(TYPE, VARIABLE_NAME)  \
  TYPE VARIABLE_NAME; _archive >> VARIABLE_NAME


#define SAVE_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)                    \
  template<class Archive> inline void save_construct_data               \
    (Archive& _archive, const TYPE* VARIABLE_NAME, unsigned int)

#define LOAD_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)              \
  template<class Archive> inline void load_construct_data         \
    (Archive& _archive, TYPE* VARIABLE_NAME, unsigned int)


#define SAVE_SDPAEVENT_CONSTRUCT_DATA(EVENT_VARIABLE) \
  (void) _archive;                                    \
  (void) EVENT_VARIABLE
#define LOAD_SDPAEVENT_CONSTRUCT_DATA()         \
  (void) _archive


#define SAVE_JOBEVENT_CONSTRUCT_DATA(EVENT_VARIABLE)    \
  SAVE_SDPAEVENT_CONSTRUCT_DATA (EVENT_VARIABLE);       \
  SAVE_TO_ARCHIVE (EVENT_VARIABLE->job_id())

#define LOAD_JOBEVENT_CONSTRUCT_DATA(JOB_ID_VAR_NAME)                \
  LOAD_SDPAEVENT_CONSTRUCT_DATA();                                   \
  LOAD_FROM_ARCHIVE (sdpa::job_id_t, JOB_ID_VAR_NAME)


#define SAVE_MGMTEVENT_CONSTRUCT_DATA(EVENT_VARIABLE) \
  SAVE_SDPAEVENT_CONSTRUCT_DATA (EVENT_VARIABLE)

#define LOAD_MGMTEVENT_CONSTRUCT_DATA()         \
  LOAD_SDPAEVENT_CONSTRUCT_DATA()


#define CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD(TYPE) \
  SAVE_CONSTRUCT_DATA_DEF (TYPE, e)                           \
  {                                                           \
    SAVE_JOBEVENT_CONSTRUCT_DATA (e);                         \
  }                                                           \
  LOAD_CONSTRUCT_DATA_DEF (TYPE, e)                           \
  {                                                           \
    LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);                    \
                                                              \
    ::new (e) TYPE (job_id);                                  \
  }

#define CONSTRUCT_DATA_DEFS_FOR_EMPTY_MGMTEVENT_OVERLOAD(TYPE) \
  SAVE_CONSTRUCT_DATA_DEF (TYPE, e)                            \
  {                                                            \
    SAVE_MGMTEVENT_CONSTRUCT_DATA (e);                         \
  }                                                            \
  LOAD_CONSTRUCT_DATA_DEF (TYPE, e)                            \
  {                                                            \
    LOAD_MGMTEVENT_CONSTRUCT_DATA();                           \
                                                               \
    ::new (e) TYPE();                                          \
  }

#include <boost/serialization/list.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/set.hpp>
