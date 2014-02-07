#ifndef GSPC_NET_IO_HPP
#define GSPC_NET_IO_HPP

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

#include <memory>

namespace gspc
{
  namespace net
  {
    struct initializer
    {
    public:
      //! \todo Remove: nthread shall be known by caller!
      initializer ();
      explicit initializer (size_t nthread);

      ~initializer ();

      operator boost::asio::io_service&();

    private:
      boost::asio::io_service _io_service;
      std::auto_ptr<boost::asio::io_service::work> _io_service_work;
      boost::thread_group _threads;
    };
  }
}

#endif
