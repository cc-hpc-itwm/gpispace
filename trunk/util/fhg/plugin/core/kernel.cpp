#include <fhg/plugin/config.hpp>
#include <fhg/plugin/core/plugin.hpp>

#include <boost/thread.hpp>

typedef boost::shared_ptr<fhg::core::plugin_t> plugin_ptr;

class kernel_impl
{
public:
  kernel_impl ();
  ~kernel_impl ();

  int load_plugin (std::string const & file);
  int unload_plugin (std::string const &name);

  //  void schedule(fhg::plugin::task_t);
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
    plugin_ptr p (fhg::core::plugin_t::create (file, false));
    {
      lock_type plugins_lock (m_mtx_plugins);
      if (m_plugins.find(p->name()) != m_plugins.end())
      {
        // TODO: print descriptor of other plugin
        throw std::runtime_error ("another plugin with the same name is already loaded: " + p->name());
      }
      m_plugins.insert (std::make_pair(p->name(), p));
    }

    // check_dependencies(p);

    // create mediator class PluginKernelMediator : public Kernel
    //    m = new PluginKernelMediator(p, this);
    //    p->start(m);
    // check return code
    //    if 0:
    //      insert m into list of plugins
    //    if 1:
    //      insert m into list of incomplete plugins
    //    else:
    //      drop plugin
  }
  catch (std::exception const & ex)
  {
    throw;
  }

  return 0;
}

int kernel_impl::unload_plugin (std::string const &name)
{
  // check reference count
  // if 0 -> unload and remove plugin, dlclose file
  // else return -EBUSY
  return 0;
}

//void kernel_impl::schedule(fhg::plugin::task_t t)
//{
  // m_task_queue.push(t);
//}

void *kernel_impl::acquire (std::string const & name)
{
  return 0;
}
