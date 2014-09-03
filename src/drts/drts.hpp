// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <drts/virtual_memory.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t;
    }
  }
}

namespace gspc
{
  namespace options
  {
    boost::program_options::options_description logging();
    boost::program_options::options_description installation();
    boost::program_options::options_description drts();
    boost::program_options::options_description virtual_memory();
  }

  class installation
  {
  public:
    installation (boost::program_options::variables_map const& vm);

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }

  private:
    boost::filesystem::path const _gspc_home;
  };

  // fake rif, replace by real rif when it's done
  class rif_t
  {
  public:
    struct endpoint_t
    {
      endpoint_t (const std::string& host, unsigned short port);

      const std::string host;
      const unsigned short port;
    };

    explicit rif_t (boost::filesystem::path const& root);

    void exec ( const std::list<endpoint_t>& rifs
              , const std::string& key
              , const std::vector<std::string>& command
              , const std::map<std::string, std::string>& environment
              );
    void store ( const std::list<endpoint_t>& rifs
               , const std::vector<char>& data
               , const std::string& path
               );
    void stop ( const std::list<endpoint_t>& rifs
              , const std::string& key
              );
  private:
    const boost::filesystem::path _root;

    struct child_t
    {
      explicit child_t (const pid_t);
      ~child_t ();
    private:
      const pid_t _pid;
    };

    // just keep track of local ssh processes
    std::map<std::string, std::list<std::unique_ptr<child_t>>> _processes;
  };

  class scoped_runtime_system
  {
  public:
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          );

    ~scoped_runtime_system();

    std::multimap<std::string, pnet::type::value::value_type>
      put_and_run
      ( boost::filesystem::path const& workflow
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      ) const;

    vmem_allocation alloc
      (unsigned long size, std::string const& description) const;

    unsigned long virtual_memory_total() const
    {
      return number_of_unique_nodes()
        * (*_virtual_memory_per_node - 32UL * (1UL << 20UL));
    }
    unsigned long number_of_unique_nodes() const
    {
      return _nodes_and_number_of_unique_nodes.second;
    }

    scoped_runtime_system (scoped_runtime_system const&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system const&) = delete;
    scoped_runtime_system (scoped_runtime_system&&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system&&) = delete;

  private:
    friend class vmem_allocation;

    installation const _installation;
    boost::filesystem::path const _state_directory;
    boost::filesystem::path const _nodefile;
    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    std::pair<std::list<std::string>, unsigned long> const
      _nodes_and_number_of_unique_nodes;
    std::unique_ptr<gpi::pc::client::api_t> _virtual_memory_api;
  };

  void set_gspc_home ( boost::program_options::variables_map&
                     , boost::filesystem::path const&
                     );
  void set_state_directory ( boost::program_options::variables_map&
                           , boost::filesystem::path const&
                           );
  void set_nodefile ( boost::program_options::variables_map&
                    , boost::filesystem::path const&
                    );
  void set_virtual_memory_manager ( boost::program_options::variables_map&
                                  , boost::filesystem::path const&
                                  );
  void set_virtual_memory_per_node ( boost::program_options::variables_map&
                                   , unsigned long
                                   );
  void set_virtual_memory_socket ( boost::program_options::variables_map&
                                 , boost::filesystem::path const&
                                 );
  void set_application_search_path ( boost::program_options::variables_map&
                                   , boost::filesystem::path const&
                                   );
  void set_log_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  void set_gui_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  void set_log_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
  void set_gui_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
}

#endif
