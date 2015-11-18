#pragma once

#include <gpi-space/types.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace fhg
{
  namespace vmem
  {
    struct gaspi_timeout
    {
      template<typename Duration>
        gaspi_timeout (Duration timeout)
          : _end (std::chrono::steady_clock::now() + timeout)
      {}
      gpi::timeout_t operator()() const;

    private:
      std::chrono::steady_clock::time_point const _end;
    };

    class gaspi_context
    {
    public:
      gaspi_context ( gaspi_timeout&
                    , gpi::port_t gaspi_sn
                    , gpi::port_t local_communication
                    );
      ~gaspi_context();
      gaspi_context (gaspi_context const&) = delete;
      gaspi_context (gaspi_context&&) = delete;
      gaspi_context& operator= (gaspi_context const&) = delete;
      gaspi_context& operator= (gaspi_context&&) = delete;

      gpi::size_t memory_size() const;
      gpi::rank_t rank() const;
      gpi::size_t number_of_nodes() const;

      std::string const& hostname_of_rank (gpi::rank_t) const;
      gpi::port_t communication_port_of_rank (gpi::rank_t) const;

      //! \todo do not expose: used to determine count of memory
      //! management task threads, which is just bad.
      gpi::size_t number_of_queues() const;

    private:
      std::vector<std::string> m_rank_to_hostname;
      std::vector<unsigned short> _communication_port_by_rank;
    };
  }
}
