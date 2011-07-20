#include <errno.h>

#include <algorithm>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

namespace fhg
{
  namespace core
  {
    plugin_t::plugin_t ( std::string const & my_name
                       , std::string const & my_filename
                       , fhg::plugin::Plugin *my_plugin
                       , const fhg_plugin_descriptor_t *my_desc
                       , int my_flags
                       )
      : m_name (my_name)
      , m_file_name(my_filename)
      , m_plugin (my_plugin)
      , m_descriptor (my_desc)
      , m_flags (my_flags)
    {
      assert (m_plugin != 0);
      assert (m_descriptor != 0);
    }

    plugin_t::~plugin_t ()
    {
      assert (m_used_by.empty());
    }

    std::string const & plugin_t::name () const
    {
      return m_name;
    }

    const fhg_plugin_descriptor_t *plugin_t::descriptor() const
    {
      return m_descriptor;
    }

    fhg::plugin::Plugin *plugin_t::get_plugin()
    {
      return m_plugin;
    }

    const fhg::plugin::Plugin *plugin_t::get_plugin() const
    {
      return m_plugin;
    }

    size_t plugin_t::use_count () const
    {
      return m_used_by.size();
    }

    bool plugin_t::is_used_by(const plugin_t *other) const
    {
      assert (other != 0);
      return std::find(m_used_by.begin(), m_used_by.end(), other) != m_used_by.end();
    }

    void plugin_t::used_by(const plugin_t *other)
    {
      assert (other != 0);
      if (! is_used_by(other))
        m_used_by.push_back(other);
    }

    void plugin_t::unused_by(const plugin_t *other)
    {
      assert (is_used_by(other));

      m_used_by.erase(std::find(m_used_by.begin(), m_used_by.end(), other));

      assert (! is_used_by(other));
    }

    int plugin_t::start (fhg::plugin::config_t const & cfg)
    {
      return m_plugin->fhg_plugin_start(cfg);
    }

    int plugin_t::stop ()
    {
      if (use_count())
      {
        return -EBUSY;
      }
      else
      {
        return m_plugin->fhg_plugin_stop();
      }
    }
  }
}
