#include "ufbmig_back.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/bind.hpp>

class UfBMigTest : FHG_PLUGIN
{
public:
  UfBMigTest ()
  {}

  FHG_PLUGIN_START ()
  {
    m_backend = fhg_kernel()->acquire<ufbmig::Backend>("ufbmig_back");
    if (m_backend)
    {
      fhg_kernel()->schedule ( "test_backend"
                             , boost::bind( &UfBMigTest::test_backend
                                          , this
                                          )
                             , 1
                             );
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (not m_backend)
    {
      m_backend = fhg_kernel()->acquire<ufbmig::Backend>(plugin);
      if (m_backend)
      {
        fhg_kernel()->schedule ( "test_backend"
                               , boost::bind( &UfBMigTest::test_backend
                                            , this
                                            )
                               , 1
                               );
      }
    }
  }
private:
  void test_backend()
  {
    try
    {
      int ec = perform_test();
      if (ec < 0)
      {
        MLOG(ERROR, "test failed: " << strerror(-ec));
      }
      else
      {
        MLOG(INFO, "test successful");
      }
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "**** BUG DETECTED: this should not happen!");
      MLOG(ERROR, "**** UNHANDLED EXCEPTION: " << ex.what());
    }

    fhg_kernel()->schedule ( "test_backend"
                           , boost::bind( &UfBMigTest::test_backend
                                        , this
                                        )
                           , 1
                           );
  }

  int perform_test()
  {
    int fd;
    int ec;
    size_t handle_size;
    size_t read_bytes;
    size_t remaining;
    char buf[314];
    const size_t chunk_size = sizeof(buf);

    assert (m_backend);

    const std::string handle_name("ufbmig.test.handle");
    fd = m_backend->open (handle_name);
    if (fd < 0)
    {
      MLOG(ERROR, "could not open handle: " << handle_name);
      return fd;
    }

    ec = m_backend->seek (fd, 0, SEEK_END, &handle_size);
    m_backend->seek(fd, 0, SEEK_SET, 0);
    if (ec < 0)
    {
      m_backend->close(fd);
      MLOG(ERROR, "could not seek");
      return ec;
    }

    remaining = handle_size;

    MLOG(INFO, "going to transfer " << remaining << " bytes");

    while (remaining)
    {
      size_t to_transfer = std::min(remaining, chunk_size);

      ec = m_backend->read(fd, buf, to_transfer, read_bytes);
      if (ec < 0)
      {
        MLOG(ERROR, "could not read: chunk_size := " << chunk_size << " to_transfer := " << to_transfer);
        m_backend->close(fd);
        return ec;
      }
      else
      {
        remaining -= read_bytes;
        MLOG(INFO, "read " << read_bytes << " bytes into buffer. " << remaining << " bytes remaing");
      }
    }

    return 0;
  }

  ufbmig::Backend *m_backend;
};


EXPORT_FHG_PLUGIN( ufbmig_test
                 , UfBMigTest
                 , ""
                 , "provides test cases for the ufbmig_backend"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "ufbmig_back"
                 , ""
                 );
