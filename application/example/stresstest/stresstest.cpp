#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <stdexcept>
#include <unistd.h>

// ************************************************************************* //

#include <boost/random.hpp>
#include <limits>

void generate (long * a, const unsigned long & N, const unsigned long & seed)
{
  MLOG (INFO, "generating " << N << " values with seed " << seed);

  boost::uniform_int<long> rand ( std::numeric_limits<long>::min()
                                , std::numeric_limits<long>::max()
                                );

  boost::mt19937 engine (seed);

  for (unsigned long i (0); i < N; ++i, ++a)
    {
      *a = rand (engine);
    }
}

void verify (long * a, const unsigned long & N, const unsigned long & seed)
{
  MLOG (INFO, "verifiying " << N << " values with seed " << seed);

  boost::uniform_int<long> rand ( std::numeric_limits<long>::min()
                                , std::numeric_limits<long>::max()
                                );

  boost::mt19937 engine (seed);

  for (unsigned long i (0); i < N; ++i, ++a)
    {
      const long v (rand(engine));

      if (*a != v)
        {
          std::ostringstream s;

          s << "BUMMER! wrong value read"
            << " expected in slot " << i << " the value " << v
            << " but got the value " << *a
            ;

          throw std::runtime_error (s.str());
        }
    }
}

// ************************************************************************* //

static unsigned long call_cnt = 0;

using we::loader::get;

// ************************************************************************* //

static void initialize ( void *
                       , const we::loader::input_t & input
                       , we::loader::output_t & output
                       )
{
  const long & num_long (get<long>(input, "num_long"));

  const size_t size (num_long * sizeof (long));

  if (size % fvmGetNodeCount() != 0)
    {
      throw std::runtime_error ("BUMMER: size % fvmGetNodeCount() != 0");
    }

  if (size > fvmGetShmemSize())
    {
      throw std::runtime_error ("BUMMER: size > fvmGetShmemSize()");
    }

  const fvmAllocHandle_t handle (fvmGlobalAlloc (size / fvmGetNodeCount()));
  const fvmAllocHandle_t scratch (fvmGlobalAlloc (size));

  if (size > 0 && !handle)
    {
      throw std::runtime_error ("BUMMER: alloc (handle) returned 0");
    }

  if (size > 0 && !scratch)
    {
      throw std::runtime_error ("BUMMER: alloc (scratch) returned 0");
    }

  const long & seed (get<long>(input, "seed"));
  const bool & _verify (get<bool>(input, "verify"));

  if (_verify)
    {
      generate ((long *)fvmGetShmemPtr(), num_long, seed);

      waitComm (fvmPutGlobalData (handle, 0, size, 0, scratch));
    }

  value::structured_t config;

  config["handle"] = static_cast<long>(handle);
  config["scratch"] = static_cast<long>(scratch);
  config["sleeptime"] = get<long>(input, "sleeptime");
  config["num_long"] = num_long;
  config["seed"] = seed;
  config["verify_all_mem"] = get<bool>(input, "verify_all_mem");
  config["verify"] = _verify;
  config["communicate"] = get<bool>(input, "communicate");

  MLOG (INFO, "initialize: config " << config);

  output.bind ("config", value::type (config));
}

// ************************************************************************* //

static void run ( void *
                , const we::loader::input_t & input
                , we::loader::output_t & output
                )
{
  ++call_cnt;

  const fvmAllocHandle_t & handle (get<long> (input, "config", "handle"));
  const fvmAllocHandle_t & scratch  (get<long> (input, "config", "scratch"));
  const long & sleeptime (get<long> (input, "config", "sleeptime"));
  const long & seed (get<long>(input, "config", "seed"));
  const long & num_long (get<long>(input, "config", "num_long"));
  const bool & verify_all_mem (get<bool>(input, "config", "verify_all_mem"));
  const bool & _verify (get<bool>(input, "config", "verify"));
  const bool & _communicate (get<bool>(input, "config", "communicate"));

  const size_t size (num_long * sizeof (long));

  const int rank (fvmGetRank());

  if (verify_all_mem)
    {
      memset (fvmGetShmemPtr(), rank, fvmGetShmemSize());
    }
  else if (_verify)
    {
      memset (fvmGetShmemPtr(), rank, size);
    }

  if (_communicate)
    {
      waitComm (fvmGetGlobalData (handle, 0, size, 0, scratch));
    }

  if (_verify)
    {
      verify ((long *)fvmGetShmemPtr(), num_long, seed);
    }

  if (verify_all_mem && fvmGetShmemSize() > size)
    {
      MLOG ( INFO
           , "verifiying mem behind data ("
           << fvmGetShmemSize() - size
           << " bytes)"
           << " to contain the value " << rank
           );

      char * a ((char *)fvmGetShmemPtr() + size);

      for (size_t i (size); i < fvmGetShmemSize(); ++i, ++a)
        {
          if (*a != rank)
            {
              std::ostringstream s;

              s << "BUMMER! memory behind data corrupted: "
                << " expected in slot " << i << " the value " << rank
                << " but got the value " << *a
                ;

              throw std::runtime_error (s.str());
            }
        }
    }

  usleep (sleeptime);

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

// ************************************************************************* //

static void finalize ( void *
                     , const we::loader::input_t & input
                     , we::loader::output_t & output
                     )
{
  MLOG (INFO, "finalize: call_cnt = " << call_cnt);

  const fvmAllocHandle_t & handle (get<long> (input, "config", "handle"));
  const fvmAllocHandle_t & scratch  (get<long> (input, "config", "scratch"));

  if (handle)
    {
      fvmGlobalFree (handle);
    }

  if (scratch)
    {
      fvmGlobalFree (scratch);
    }

  output.bind ("trigger", pnet::type::value::value_type (we::type::literal::control()));
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (stresstest);
{
  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (run);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (stresstest);

WE_MOD_FINALIZE_START (stresstest);
{
}
WE_MOD_FINALIZE_END (stresstest);
