#include "progress.h"
#include "progress.hpp"
#include "kvs.hpp"

#include <errno.h>

#include <string>
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

  int set (const char *name, size_t value)
  {
    if (! name)
    {
      return -EINVAL;
    }

    if (value > 100)
      return -EOVERFLOW;

    m_kvs->put( add_prefix(name)
              , boost::lexical_cast<std::string>(value)
              );

    return 0;
  }

  int get (const char *name) const
  {
    if (! name)
    {
      return -EINVAL;
    }

    try
    {
      return std::abs( boost::lexical_cast<int>(m_kvs->get( add_prefix(name)
                                                          , "0"
                                                          )
                                               )
                     );
    }
    catch (std::exception const &ex)
    {
      return -EINVAL;
    }
  }
private:
  std::string add_prefix(std::string const &name) const
  {
    return m_prefix + "." + name;
  }

  kvs::KeyValueStore *m_kvs;
  std::string m_prefix;
};

int set_progress(const char *name, int value)
{
  return global_progress->set(name, value);
}

int get_progress(const char *name)
{
  return global_progress->get(name);
}

EXPORT_FHG_PLUGIN( progress
                 , ProgressImpl
                 , "provides access to progress values"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
