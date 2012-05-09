#include "progress.h"
#include "progress.hpp"
#include "kvs.hpp"

#include <errno.h>

#include <string>
#include <limits>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

static progress::Progress *global_progress;

class ProgressImpl : FHG_PLUGIN
                   , public progress::Progress
{
public:
  ProgressImpl () {}
  ~ProgressImpl () {}

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
    assert (name);

    size_t cur_value, max_value;
    if (0 == current(name, &cur_value, &max_value))
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
      return -EINVAL;
    }
  }

  int initialize (std::string const &name, size_t max)
  {
    assert (name);

    m_kvs->put ( get_key_for_current (name), 0);
    m_kvs->put ( get_key_for_maximum (name), max);

    return 0;
  }

  int current (std::string const &name, size_t *value, size_t *max) const
  {
    assert (name);

    if (value)
    {
      *value = m_kvs->get<size_t>(get_key_for_current (name), 0);
    }

    if (max)
    {
      *max = m_kvs->get<size_t>(get_key_for_maximum (name), 1);
    }

    return 0;
  }

  int finalize (std::string const &name)
  {
    assert (name);

    size_t max;
    if (0 == current (name, 0, &max))
    {
      return set (name, max);
    }

    return -ESRCH;
  }

  int tick (std::string const &name, size_t step)
  {
    assert (name);

    if (step > (size_t)std::numeric_limits<int>::max())
    {
      return -EOVERFLOW;
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
  if (0 == global_progress->current(name, &value, &max))
  {
    return (int)( (float)value / max) * 100;
  }
  else
  {
    return 0;
  }
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
