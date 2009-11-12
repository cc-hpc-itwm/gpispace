/*
 * =====================================================================================
 *
 *       Filename:  Serialization.hpp
 *
 *    Description:  serialization stuff (not to be installed!)
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:30:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_NRE_WORKER_MESSAGES_SERIALIZATION_HPP
#define SDPA_NRE_WORKER_MESSAGES_SERIALIZATION_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/wf/Serialization.hpp>
#include <sdpa/daemon/nre/messages.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive &, sdpa::nre::worker::Message &, const unsigned int /* version */)
  {
    // nothing
  }

  template <class Archive>
  void serialize(Archive &ar, sdpa::nre::worker::Reply &r, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Message>(r);
  }

  template <class Archive>
  void serialize(Archive &ar, sdpa::nre::worker::Request &r, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Message>(r);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::PingRequest &rqst, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Request>(rqst);
    ar & rqst.key();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::PingReply &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.key();
    ar & rply.pid();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ExecuteRequest &rqst, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Request>(rqst);
    ar & rqst.activity();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ExecuteReply &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.result();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::LoadModuleRequest &rqst, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Request>(rqst);
    ar & rqst.path();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ModuleLoaded &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.path();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ModuleNotLoaded &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.path();
    ar & rply.reason();
  }

BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Message)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Request)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Reply)

}}

#endif
