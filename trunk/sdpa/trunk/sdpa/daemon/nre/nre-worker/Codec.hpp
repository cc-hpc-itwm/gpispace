/*
 * =====================================================================================
 *
 *       Filename:  Codec.hpp
 *
 *    Description:  codec
 *
 *        Version:  1.0
 *        Created:  11/12/2009 11:44:41 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_NRE_WORKER_CODEC_HPP
#define SDPA_DAEMON_NRE_WORKER_CODEC_HPP 1

#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sdpa/daemon/nre/nre-worker/messages.hpp>
#include <sdpa/daemon/nre/nre-worker/Serialization.hpp>

namespace sdpa { namespace nre { namespace worker {
  class Codec
  {
  public:
    Message *decode(const std::string &bytes) throw (std::exception)
    {
      Message *msg(NULL);
      std::stringstream sstr(bytes);
      boost::archive::text_iarchive ar(sstr);
      init_archive(ar);
      ar >> msg;
      return msg;
    }

    std::string encode(const Message &msg) throw (std::exception)
    {
      std::ostringstream sstr;
      boost::archive::text_oarchive ar(sstr);
      init_archive(ar);
      const Message *msg_ptr = &msg;
      ar << msg_ptr;
      return sstr.str();
    }
  private:
    template <class Archive>
    void init_archive(Archive & ar)
    {
      ar.register_type(static_cast<sdpa::nre::worker::PingRequest*>(NULL));
      ar.register_type(static_cast<sdpa::nre::worker::PingReply*>(NULL));

      ar.register_type(static_cast<sdpa::nre::worker::InfoRequest*>(NULL));
      ar.register_type(static_cast<sdpa::nre::worker::InfoReply*>(NULL));

      ar.register_type(static_cast<sdpa::nre::worker::ExecuteRequest*>(NULL));
      ar.register_type(static_cast<sdpa::nre::worker::ExecuteReply*>(NULL));

      /*ar.register_type(static_cast<sdpa::nre::worker::ModuleLoaded*>(NULL));
      ar.register_type(static_cast<sdpa::nre::worker::ModuleNotLoaded*>(NULL));
      ar.register_type(static_cast<sdpa::nre::worker::LoadModuleRequest*>(NULL)); */
    }
  };
}}}

#endif
