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

#include <sdpa/daemon/nre/messages.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive &ar, sdpa::nre::worker::Message &m, const unsigned int /* version */)
  {
    ar & m.id();
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
  }
  template <class Archive>
  void serialize(Archive &ar, struct timeval &tv, const unsigned int /* version */)
  {
    ar & tv.tv_sec;
    ar & tv.tv_usec;
  }

  template <class Archive>
  void serialize(Archive &ar, sdpa::nre::worker::PingReply::usage_t &u, const unsigned int /* version */)
  {
    ar & u.ru_utime; /* user time used */
    ar & u.ru_stime; /* system time used */
    ar & u.ru_maxrss;        /* maximum resident set size */
    ar & u.ru_ixrss;         /* integral shared memory size */
    ar & u.ru_idrss;         /* integral unshared data size */
    ar & u.ru_isrss;         /* integral unshared stack size */
    ar & u.ru_minflt;        /* page reclaims */
    ar & u.ru_majflt;        /* page faults */
    ar & u.ru_nswap;         /* swaps */
    ar & u.ru_inblock;       /* block input operations */
    ar & u.ru_oublock;       /* block output operations */
    ar & u.ru_msgsnd;        /* messages sent */
    ar & u.ru_msgrcv;        /* messages received */
    ar & u.ru_nsignals;      /* signals received */
    ar & u.ru_nvcsw;         /* voluntary context switches */
    ar & u.ru_nivcsw;        /* involuntary context switches */
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::PingReply &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.pid();
    ar & rply.usage();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::InfoRequest &rqst, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Request>(rqst);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::pc_info_t &pc_info, const unsigned int /* version */)
  {
    ar & pc_info.pid();
    ar & pc_info.rank();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::InfoReply &rply, const unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.info();
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
    //ar & rply.state();
  }

  /*
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::LoadModuleRequest &rqst, const unsigned int )
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Request>(rqst);
    ar & rqst.path();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ModuleLoaded &rply, const unsigned int )
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.path();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::nre::worker::ModuleNotLoaded &rply, const unsigned int )
  {
    ar & boost::serialization::base_object<sdpa::nre::worker::Reply>(rply);
    ar & rply.path();
    ar & rply.reason();
  }*/

BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Message)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Request)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::nre::worker::Reply)

}}

#endif
