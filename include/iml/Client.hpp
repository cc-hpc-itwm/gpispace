// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/MemcpyID.hpp>
#include <iml/MemoryLocation.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/SharedMemoryAllocationHandle.hpp>
#include <iml/detail/dllexport.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace iml
{
  namespace detail
  {
    class OpenedSharedMemory;
  }

  //! A client to interact with a node-local IML server to manage
  //! segments and allocations as well as transferring data. While
  //! allocation and segment handles may come from different
  //! processes, a \c Client and a shared memory segment is always
  //! required when transferring data.
  class IML_DLLEXPORT Client
  {
  public:
    //! Create a client connecting via a socket at \a socket_path.
    Client (boost::filesystem::path const& socket_path);

    //! Create a client connecting with the information given as
    //! command line arguments parsed into the given \a vm.
    //! \see options()
    Client (boost::program_options::variables_map const& vm);

    //! Command line options for use in Boost.ProgramOptions, which
    //! can be used to produce the input for the \c vm overload of the
    //! constructor.
    //! \see set_socket()
    static boost::program_options::options_description options();
    //! Overwrite the socket to be used to connect to the server.
    static void set_socket
      (boost::program_options::variables_map&, boost::filesystem::path);

    //! Create a local shared memory segment that can be used to
    //! transfer data up to \a size bytes from global segments into
    //! the local process.
    SharedMemoryAllocationHandle create_shm_segment_and_allocate
      (MemorySize size);
    //! Delete the given \a shared_memory_allocation.
    void free_and_delete_shm_segment
      (SharedMemoryAllocationHandle shared_memory_allocation);
    //! Retrieve a pointer to the given local shared memory \a
    //! handle. The pointer can be read and written to freely, but
    //! interleaving access with transfers started by \c
    //! async_memcpy() is unspecified behavior.
    char* pointer (SharedMemoryAllocationHandle const& handle);

    //! Create a global segment with the type and parameters defined
    //! by \a description, providing \a total_size bytes for
    //! allocation.
    //! \see iml::SegmentAndAllocation
    SegmentHandle create_segment
      (SegmentDescription const& description, MemorySize total_size);
    //! Delete the given \a segment.
    void delete_segment (SegmentHandle segment);

    //! Create an allocation in the given \a segment with \a
    //! total_size allocated distributed over the nodes.
    //! \see iml::SegmentAndAllocation
    AllocationHandle create_allocation
      (SegmentHandle const& segment, MemorySize total_size);
    //! Delete the given \a allocation.
    void delete_allocation (AllocationHandle allocation);

    //! Asynchronously start a transfer of \a amount bytes from \a
    //! source to \a destination. One of the given locations shall be
    //! local, the other global, but both directions are possible. The
    //! transfer is assigned an ID which is returned and shall be
    //! waited for by this client.
    //! \note When writing, the local range shall not be overwritten
    //! until the corresponding \c wait() call returned. Modifying the
    //! local buffer during a transfer has unspecified behavior.
    //! \see wait(), memcpy()
    // \todo split into put+get
    // \todo differ between shm location and global?
    MemcpyID async_memcpy ( MemoryLocation const& destination
                          , MemoryLocation const& source
                          , MemorySize amount
                          );
    //! Wait for completion of the transfer started with the
    //! previously given \a memcpy_id.
    //! \see async_memcpy()
    void wait (MemcpyID memcpy_id);

    //! Synchronously transfer \a amount bytes from \a source to \a
    //! destination.
    //! \note Equivalent to an immediate call to \c wait() after \c
    //! async_memcpy().
    //! \see async_memcpy(), wait()
    void memcpy ( MemoryLocation const& destination
                , MemoryLocation const& source
                , MemorySize amount
                );

    //! Determine how much it would "cost" to transfer the given \a
    //! regions to each possible node.
    //! \note The scale of the costs are not defined, only that a
    //! lower value is cheaper.
    //! \note Not all segments have a sensible definition of cost and
    //! costs are provided as an estimation, i.e. may not reflect the
    //! true duration needed for the transfer.
    //! \return The cost by hostname.
    std::unordered_map<std::string, double> transfer_costs
      (std::list<MemoryRegion> regions);

    struct AllocationInformation
    {
      MemorySize size;
    };
    AllocationInformation stat (AllocationHandle allocation);

    std::unordered_set<SegmentHandle> existing_segments();
    std::unordered_set<AllocationHandle> existing_allocations (SegmentHandle);

    Client() = delete;
    Client (Client const&) = delete;
    Client (Client&&) = delete;
    Client& operator= (Client const&) = delete;
    Client& operator= (Client&&) = delete;
    ~Client();

  private:
    void stop (std::unique_lock<std::mutex> const&);

    template < typename RequestCategory
             , typename Reply
             , typename Request
             , typename ReplyCategory = RequestCategory
             >
      Reply communicate ( std::unique_lock<std::mutex> const&
                        , std::string const& what
                        , Request const&
                        );

    std::mutex _socket_and_shm_segments_guard;
    int _socket;
    std::map < SharedMemoryAllocationHandle
             , std::shared_ptr<detail::OpenedSharedMemory>
             > _shm_segments;
  };
}
