#include <fhg/plugin/kernel.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/plugin.hpp>

#include <boost/thread.hpp>

typedef boost::shared_ptr<fhg::core::plugin_t> plugin_ptr;

class kernel_impl : public fhg::plugin::kernel_t
{
public:
  kernel_impl ();
  ~kernel_impl ();

  fhg::plugin::plugin_info_list_t list_plugins () const;

  int load_plugin (std::string const & file);
  int unload_plugin (std::string const &name);

  void schedule(fhg::plugin::task_t);
protected:
  void * acquire(std::string const & name);
  void * acquire(std::string const & name, std::string const &version);
  void   release(void *);
private:
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  mutable mutex_type m_mtx_plugins;
  /* loaded plugins, interdependencies, reference counts */
  std::map<std::string, plugin_ptr> m_plugins;

  /* task queue */
  /* thread */
};

kernel_impl::kernel_impl ()
{}

kernel_impl::~kernel_impl ()
{
  // unload all non-static plugins according to dependency graph
}

int kernel_impl::load_plugin(std::string const & file)
{
  try
  {
    lock_type plugins_lock (m_mtx_plugins);

    plugin_ptr p (fhg::core::plugin_t::create (file));
    if (m_plugins.find(p->name()) != m_plugins.end())
    {
      throw std::runtime_error ("already loaded: " + p->name());
    }

    p->check_dependencies();

    // get global config here and filter by name
    fhg::plugin::config_t cfg;
    p->configure(cfg);

    p->start ();

    m_plugins.insert (std::make_pair(p->name(), p));

    // update_dependencies(p);
  }
  catch (std::exception const & ex)
  {
    //
  }

  // dlopen file

  // check/compare build information string

  // check name
  //    plugin already registered with that name

  // get plugin descriptor
  //    check dependencies

  // instantiate plugin
  //    use count = 0
  //    used by = []

  return 0;
}

int kernel_impl::unload_plugin (std::string const &name)
{
  // check reference count
  // if 0 -> unload and remove plugin, dlclose file
  // else return -EBUSY
  return 0;
}

void kernel_impl::schedule(fhg::plugin::task_t t)
{
  // m_task_queue.push(t);
}

void *kernel_impl::acquire (std::string const & name)
{
  return 0;
}

fhg::plugin::plugin_info_list_t kernel_impl::list_plugins () const
{
  return fhg::plugin::plugin_info_list_t();
}
