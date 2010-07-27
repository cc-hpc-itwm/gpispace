#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include <stdexcept>
#include <unistd.h>

#include <map>

typedef std::map<unsigned int, unsigned long> call_cnt_map_t ;
static call_cnt_map_t call_cnt_map;

static void fun ( void *
                , const we::loader::input_t & input
                , we::loader::output_t & output
                )
{
  const long & x (we::loader::get_input<long>(input, "x"));
  const long & id (we::loader::get_input<long>(input, "id"));

  MLOG (DEBUG, fun << ": " << id);

  ++call_cnt_map[x];

  we::loader::put_output (output, "done", control());
}

static unsigned long call_cnt_A = 0;
static void A ( void *
              , const we::loader::input_t & input
              , we::loader::output_t & output
              )
{
  const long & id (we::loader::get_input<long>(input, "id"));

  MLOG (DEBUG, "A : " << id);

  ++call_cnt_A;

  we::loader::put_output (output, "done", control());
}

static unsigned long call_cnt_B = 0;
static void B ( void *
              , const we::loader::input_t & input
              , we::loader::output_t & output
              )
{
  const long & id (we::loader::get_input<long>(input, "id"));

  MLOG (DEBUG, "B : " << id);

  ++call_cnt_B;

  we::loader::put_output (output, "done", control());
}

static unsigned long call_cnt_C = 0;
static void C ( void *
              , const we::loader::input_t & input
              , we::loader::output_t & output
              )
{
  const long & id (we::loader::get_input<long>(input, "id"));

  MLOG (DEBUG, "C : " << id);

  ++call_cnt_C;

  we::loader::put_output (output, "done", control());
}

static unsigned long call_cnt_D = 0;
static void D ( void *
              , const we::loader::input_t & input
              , we::loader::output_t & output
              )
{
  const long & id (we::loader::get_input<long>(input, "id"));

  MLOG (DEBUG, "D : " << id);

  ++call_cnt_D;

  we::loader::put_output (output, "done", control());
}

static void finalize ( void *
                     , const we::loader::input_t & input
                     , we::loader::output_t & output
                     )
{
  for ( call_cnt_map_t::const_iterator cnt (call_cnt_map.begin())
      ; cnt != call_cnt_map.end()
      ; ++cnt
      )
    {
      MLOG (INFO, "call_cnt [ " << cnt->first << " ] = " << cnt->second);
    }

  MLOG (INFO, "call_cnt_A = " << call_cnt_A);
  MLOG (INFO, "call_cnt_B = " << call_cnt_B);
  MLOG (INFO, "call_cnt_C = " << call_cnt_C);
  MLOG (INFO, "call_cnt_D = " << call_cnt_D);

  we::loader::put_output (output, "done", control());
}

WE_MOD_INITIALIZE_START (concurrent);
{
  WE_REGISTER_FUN (A);
  WE_REGISTER_FUN (B);
  WE_REGISTER_FUN (C);
  WE_REGISTER_FUN (D);
  WE_REGISTER_FUN (fun);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (concurrent);

WE_MOD_FINALIZE_START (concurrent);
{
}
WE_MOD_FINALIZE_END (concurrent);
