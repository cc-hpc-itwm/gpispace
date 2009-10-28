/*
 * =====================================================================================
 *
 *       Filename:  Serialization.hpp
 *
 *    Description:  implements serialization/deserialization
 *
 *        Version:  1.0
 *        Created:  10/27/2009 12:08:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_EVENTS_SERIALIZATION_HPP
#define SDPA_EVENTS_SERIALIZATION_HPP 1

#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SDPAEvent & e, unsigned int /* version */)
  {
    ar & e.from();
    ar & e.to();
  }

  // ***********
  // Mgmt events
  // ***********
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::MgmtEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::SDPAEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ErrorEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
    ar & e.error_code();
    ar & e.reason();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ConfigReplyEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ConfigRequestEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
  }

  // ***********
  // Job events
  // ***********

  template <class Archive>
  void serialize(Archive & ar, sdpa::job_id_t & job_id, unsigned int /* version */)
  {
    ar & job_id.str();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::SDPAEvent>(e);
    ar & e.job_id();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SubmitJobEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
    ar & e.description();
    ar & e.parent_id();
  }
}}

#endif
