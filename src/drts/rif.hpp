#ifndef DRTS_RIF_HPP
#define DRTS_RIF_HPP

#include <boost/filesystem/path.hpp>

#include <memory>
#include <string>
#include <map>
#include <list>
#include <vector>

namespace gspc
{
  // fake rif, replace by real rif when it's done
  class rif_t
  {
  public:
    struct endpoint_t
    {
      endpoint_t (const std::string& host, unsigned short port);

      const std::string host;
      const unsigned short port;

      bool operator< (const endpoint_t& other) const;
      bool operator== (const endpoint_t& other) const;
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
      explicit child_t (const endpoint_t&, const pid_t);
      ~child_t ();
    private:
      const endpoint_t _rif;
      const pid_t _pid;
    };

    // just keep track of local ssh processes
    std::map
    < endpoint_t
    , std::map< std::string
              , std::list<std::unique_ptr<child_t>>
              >
    > _processes;
  };
}

#endif
