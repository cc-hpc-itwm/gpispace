/* -*- mode: c++ -*- */

#include "manager.hpp"

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      manager_t::~manager_t ()
      {
        try
        {
          stop();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "error withing ~manager_t: " << ex.what());
        }
      }

      void manager_t::start ()
      {
        m_connector.start ();
      }

      void manager_t::stop ()
      {
        m_connector.stop ();
      }
    }
  }
}
