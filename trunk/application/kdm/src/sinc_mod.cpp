#include <we/loader/macros.hpp>
#include <limits>
#include "sinc_mod.hpp"

namespace sinc
{
  struct sinc_data_t
  {
    sinc_data_t (float _dt = std::numeric_limits<float>::quiet_NaN())
      : dt(_dt)
    { }

    void init()
    {
      s.init(dt);
    }

    // generic part ends here
    SincInterpolator s;

    // params
    float dt;
  };

  struct sinc_data_mgr
  {
    sinc_data_t * get(unsigned int tid, float dt)
    {
      sinc_data_t * sd = interpolators[tid];
      if (sd == NULL)
      {
        sd = new sinc_data_t(dt);
        interpolators[tid] = sd;

        sd->init();
      }
      else if ((dt - sd->dt) > std::numeric_limits<float>::epsilon())
      {
        sd->dt = dt;
        sd->init();
      }

      return sd;
    }

    ~sinc_data_mgr() throw()
    {
      while (! interpolators.empty())
      {
        sinc_data_t * sd = interpolators.begin()->second;
        interpolators.erase (interpolators.begin());

        if (sd) delete sd;
      }
    }
    std::map<unsigned int, sinc::sinc_data_t*> interpolators;
  };
}

SincInterpolator * getSincInterpolator (unsigned int thread_id, float dt)
{
  static sinc::sinc_data_mgr sd_mgr;
  return &(sd_mgr.get(thread_id, dt)->s);
}

static SincInterpolator ** sinc_int_array (0);
static unsigned int NThread (0);

SincInterpolator ** SincIntArray ()
{
  return sinc_int_array;
}

void initSincIntArray (const unsigned int num_threads, float dt)
{
  if (sinc_int_array)
  {
    for (unsigned int i (0); i < NThread; ++i)
    {
      delete sinc_int_array[i];
    }

    delete [] sinc_int_array;
    sinc_int_array = 0;
  }

  sinc_int_array = new SincInterpolator*[num_threads];

  if (0 == sinc_int_array)
  {
    throw std::runtime_error ("sinc: could not allocate memory for SincInterpolator");
  }

  for (unsigned int i (0); i < num_threads; ++i)
  {
    sinc_int_array[i] = new SincInterpolator;

    if (0 == sinc_int_array[i])
    {
      throw std::runtime_error ("sinc: could not allocate memory for SincInterpolator");
    }

    sinc_int_array[i]->init (dt);
  }

  NThread = num_threads;
}


WE_MOD_INITIALIZE_START (sinc);
{
  //  LOG(INFO, "WE_MOD_INITIALIZE_START (sinc)");
  std::cout << "WE_MOD_INITIALIZE_START (sinc)" << std::endl;
}
WE_MOD_INITIALIZE_END (sinc);

WE_MOD_FINALIZE_START (sinc);
{
  //  LOG(INFO, "WE_MOD_FINALIZE_START (sinc)");
  std::cout << "WE_MOD_FINALIZE_START (sinc)" << std::endl;
  if (sinc_int_array)
  {
    for (unsigned int i (0); i < NThread; ++i)
    {
      delete sinc_int_array[i];
    }

    delete [] sinc_int_array;
    sinc_int_array = 0;
  }
}
WE_MOD_FINALIZE_END (sinc);

