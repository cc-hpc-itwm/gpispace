#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <limits>
#include "sinc_mod.hpp"

namespace sinc
{
  struct sinc_data_t
  {
    sinc_data_t (float _dt = -1.f)
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
    void resize (unsigned int ntid)
    {
      for (unsigned int tid (interpolators.size()); tid < ntid; ++tid)
	{
	  interpolators.push_back (new sinc_data_t);
	}
    }

    sinc_data_t * get(unsigned int tid, float dt)
    {
      assert (interpolators.size() > tid);

      sinc_data_t * sd (interpolators[tid]);
      if (  (sd->dt < 0)
	 || (fabs(dt - sd->dt) > std::numeric_limits<float>::epsilon())
	 )
      {
        sd->dt = dt;
        sd->init();
      }

      return sd;
    }

    typedef std::vector<sinc::sinc_data_t*> interpolators_t;

    ~sinc_data_mgr() throw()
    {
      for ( interpolators_t::const_iterator pos (interpolators.begin())
	  ; pos != interpolators.end()
	  ; ++pos
	  )
	{
	  if (*pos)
	    {
	      delete (*pos);
	    }
	  else
	    {
	      assert (false /* BUMMER */);
	    }
	}
    }

    interpolators_t interpolators;
  };
}

static sinc::sinc_data_mgr sd_mgr;

SincInterpolator * getSincInterpolator (unsigned int thread_id, float dt)
{
  return &(sd_mgr.get(thread_id, dt)->s);
}

void resizeSincInterpolatorArray (unsigned int ntid)
{
  sd_mgr.resize (ntid);
}

/* ************************************************************************* */

WE_MOD_INITIALIZE_START (sinc);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (sinc)");
}
WE_MOD_INITIALIZE_END (sinc);

WE_MOD_FINALIZE_START (sinc);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (sinc)");
}
WE_MOD_FINALIZE_END (sinc);

