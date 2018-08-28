#include <logging/tcp_endpoint.hpp>
#include <logging/tcp_endpoint_serialization.hpp>

#include <fhg/util/boost/program_options/validators.hpp>

#include <stdexcept>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      bad_host_and_port_string::bad_host_and_port_string (std::string str)
        : std::invalid_argument
            ("bad host and port '" + str + "': expected 'host:port'")
      {}
    }

    tcp_endpoint::tcp_endpoint (std::string host, unsigned short port)
      : host (std::move (host))
      , port (port)
    {}

    namespace
    {
      std::pair<std::string, unsigned short> split (std::string const& raw)
      try
      {
        auto const colon_pos (raw.find (':'));

#define REQUIRE(cond_)                                         \
        if (!(cond_)) { throw std::runtime_error (#cond_); }

        REQUIRE (colon_pos != std::string::npos);
        REQUIRE (colon_pos + 1 != raw.size());
        REQUIRE (colon_pos != 0);

        auto host (raw.substr (0, colon_pos));
        auto const port_string (raw.substr (colon_pos + 1));

        std::size_t pos (0);
        auto const port (std::stoi (port_string, &pos));

        REQUIRE (pos == port_string.size());
        REQUIRE (port >= 0);
        REQUIRE (port <= std::numeric_limits<unsigned short>::max());

#undef REQUIRE

        return {std::move (host), port};
      }
      catch (...)
      {
        std::throw_with_nested (error::bad_host_and_port_string (raw));
      }
    }

    tcp_endpoint::tcp_endpoint (std::string const& host_and_port)
      : tcp_endpoint (split (host_and_port))
    {}

    tcp_endpoint::tcp_endpoint (std::pair<std::string, unsigned short> pair)
      : tcp_endpoint (std::move (pair.first), pair.second)
    {}

    tcp_endpoint::operator std::pair<std::string, unsigned short>() const
    {
      return {host, port};
    }

    void validate ( boost::any& result
                  , std::vector<std::string> const& values
                  , tcp_endpoint*
                  , int
                  )
    {
      return util::boost::program_options::validate<tcp_endpoint>
        (result, values);
    }
  }
}
