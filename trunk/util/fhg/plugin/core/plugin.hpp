#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhg/plugin/build.hpp>
#include <fhg/plugin/kernel.hpp>
#include <fhg/plugin/plugin_base.hpp>

namespace fhg
{
  namespace core
  {
    class plugin_t
    {
    public:
      typedef boost::shared_ptr<plugin_t> ptr_t;

      ~plugin_t ();

      static ptr_t create (std::string const & filename, bool force);

      std::string const & name () const;

      const fhg_plugin_descriptor_t * descriptor() const;
      fhg::plugin::Plugin * get_plugin();
      const fhg::plugin::Plugin * get_plugin() const;

      bool is_depending_on(const ptr_t &) const;
      bool is_in_use() const { return use_count() > 0; }
      size_t use_count() const;
      void add_dependency (const ptr_t &);
      void del_dependency (const ptr_t &);

      int init ();
      int start (fhg::plugin::Kernel*);
      int stop  ();

      template <typename T>
      bool implements()
      {
        return this->as<T>() != 0;
      }

      template <typename T>
      T* as ()
      {
        return dynamic_cast<T*>(get_plugin());
      }

      template <typename T>
      const T* as () const
      {
        return dynamic_cast<const T*>(get_plugin());
      }
    private:
      typedef std::list<ptr_t> dependency_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      void check_dependencies();

      void inc_refcount ();
      void dec_refcount ();

      plugin_t ( std::string const & name
               , std::string const & filename
               , const fhg_plugin_descriptor_t *
               , int flags
               , void * handle
               );

      mutable mutex_type m_refcount_mtx;
      mutable mutex_type m_dependencies_mtx;

      std::string m_name;
      std::string m_file_name;
      fhg::plugin::Plugin *m_plugin;
      const fhg_plugin_descriptor_t *m_descriptor;
      int m_flags;
      void *m_handle;
      fhg::plugin::Kernel *m_kernel;

      bool m_started;
      dependency_list_t m_dependencies;
      std::size_t m_refcount;
    };
  }
}

#endif
