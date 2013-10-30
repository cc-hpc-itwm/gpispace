#include "progress.h"
#include "progress.hpp"
#include "kvs.hpp"

#include <errno.h>

#include <string>
#include <limits>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>
#include <fhg/plugin/plugin.hpp>

static progress::Progress *global_progress;

class ProgressImpl : FHG_PLUGIN
                   , public progress::Progress
{
public:
  FHG_PLUGIN_START()
  {
    m_kvs = fhg_kernel()->acquire<kvs::KeyValueStore>("kvs");
    if (not m_kvs)
    {
      MLOG(ERROR, "could not get access to the key value store!");
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    m_prefix = fhg_kernel()->get("prefix", "progress");

    global_progress = this;
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int set (std::string const &name, size_t value)
  {
    assert (not name.empty());

    size_t cur_value, max_value;
    if (0 == get (name, &cur_value, &max_value))
    {
      if (value <= max_value)
      {
        m_kvs->put ( get_key_for_current (name)
                   , value
                   );
        return 0;
      }
      else
      {
        return -EINVAL;
      }
    }
    else
    {
      return -ESRCH;
    }
  }

  int reset ( std::string const &name
            , std::string const &phase
            , size_t max
            )
  {
    assert (not name.empty());

    m_kvs->put ( get_key_for_current (name), 0);
    m_kvs->put ( get_key_for_maximum (name), max);
    m_kvs->put ( get_key_for_phase   (name), phase);

    return 0;
  }

  int get (std::string const &name, size_t *value, size_t *max) const
  {
    assert (not name.empty());

    if (value)
    {
      *value = m_kvs->get<size_t>(get_key_for_current (name), 0);
    }

    if (max)
    {
      *max = m_kvs->get<size_t>(get_key_for_maximum (name), 100);
    }

    return 0;
  }

  int finalize (std::string const &name)
  {
    assert (not name.empty());

    size_t max;
    if (0 == get (name, 0, &max))
    {
      return set (name, max);
    }

    return -ESRCH;
  }

  int tick (std::string const &name, int step)
  {
    assert (not name.empty());

    size_t cur, max;
    get (name, &cur, &max);

    if (max && (cur + step) > max)
    {
      m_kvs->put (get_key_for_current (name), max);
    }
    else
    {
      m_kvs->inc (get_key_for_current (name), (int)step);
    }

    return 0;
  }
private:
  std::string add_prefix(std::string const &name) const
  {
    return m_prefix + "." + name;
  }

  std::string get_key_for_current (std::string const &name) const
  {
    return m_prefix + "." + name + "." + "current";
  }

  std::string get_key_for_maximum (std::string const &name) const
  {
    return m_prefix + "." + name + "." + "maximum";
  }

  std::string get_key_for_phase (std::string const &name) const
  {
    return m_prefix + "." + name + "." + "phase";
  }

  kvs::KeyValueStore *m_kvs;
  std::string m_prefix;
};

size_t set_progress(const char *name, size_t value)
{
  if (0 == global_progress->set(name, value))
  {
    return value;
  }
  else
  {
    return 0;
  }
}

size_t get_progress(const char *name)
{
  size_t value; size_t max;
  if (0 == global_progress->get (name, &value, &max))
  {
    return (int)( (float)value / max) * 100;
  }
  else
  {
    return 0;
  }
}

/**
   Resets a progress counter.

   The current value will be set to 0 and the maximum number of expected steps
   will be set to 'max'.

   @param name the name of the progress counter
   @param max the expected maximum value
*/
int progress_reset ( const char *name
                   , const char *phase
                   , size_t max
                   )
{
  assert (global_progress);
  return global_progress->reset (name, phase, max);
}

/**
   Get the current value of the progress counter.

   @param name the name of the progress counter
   @param value store the current value in *value
   @return -ESRCH when not found, -EINVAL when not a progress counter
*/
int progress_get (const char *name, size_t * value)
{
  assert (global_progress);
  return global_progress->get (name, value, 0);
}

/**
   Count one tick on the given progress counter.

   The current value of the counter will be increased by one.

   @param name the name of the progress counter
   @return -EINVAL when not a counter
*/
int progress_tick (const char *name)
{
  return progress_tick_n (name, 1);
}

/**
   Count 'inc' ticks on the given progress counter.

   The current  value of the counter  will be increased by  'inc'. The counter
   does not check for overflow, it may happen that it over-counts.

   @param name the name of the progress counter
   @return -EINVAL when not a counter
*/
int progress_tick_n (const char *name, int inc)
{
  assert (global_progress);
  return global_progress->tick (name, inc);
}

/**
   Set a progress counter to its maximum value.

   @return -EINVAL when not a counter
*/
int progress_finalize (const char *name)
{
  assert (global_progress);
  return global_progress->finalize (name);
}


EXPORT_FHG_PLUGIN( progress
                 , ProgressImpl
                 , "progress"
                 , "provides access to progress values"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
