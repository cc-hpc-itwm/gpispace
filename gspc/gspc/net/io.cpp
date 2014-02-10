#include <gspc/net/io.hpp>

#include <fhg/util/get_cpucount.h>

#include <boost/bind.hpp>

namespace gspc
{
  namespace net
  {
    namespace
    {
      size_t get_thread_count ()
      {
        int ncpu = fhg_get_cpucount();
        if (ncpu < 0)
          ncpu = 1;
        return ncpu;
      }
    }

    initializer::initializer()
      : _io_service()
      , _io_service_work (new boost::asio::io_service::work (_io_service))
    {
      size_t nthread (get_thread_count());

      while (nthread --> 0)
      {
        _threads.create_thread
          (boost::bind (&boost::asio::io_service::run, &_io_service));
      }
    }

    initializer::initializer (size_t nthread)
      : _io_service()
      , _io_service_work (new boost::asio::io_service::work (_io_service))
    {
      while (nthread --> 0)
      {
        _threads.create_thread
          (boost::bind (&boost::asio::io_service::run, &_io_service));
      }
    }

    initializer::~initializer()
    {
      _io_service_work.reset();
      _io_service.stop();

      _threads.join_all();
    }

    initializer::operator boost::asio::io_service&()
    {
      return _io_service;
    }
  }
}
