/*
 * =====================================================================================
 *
 *       Filename:  Serialization.hpp
 *
 *    Description:  serialization functionalities for sdpa::wf classes
 *
 *        Version:  1.0
 *        Created:  10/17/2009 01:05:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_WF_SERIALIZATION_HPP
#define SDPA_WF_SERIALIZATION_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/util/Properties.hpp>
#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Parameter.hpp>
#include <sdpa/wf/Activity.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, sdpa::util::Properties & props, const unsigned int /* version */)
  {
    ar & props.map();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::wf::Token & token, const unsigned int /* version */)
  {
    ar & token.data();
    ar & token.properties();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::wf::Parameter & param, const unsigned int /* version */)
  {
    ar & param.name();
    ar & param.edge_type();
    ar & param.token();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::wf::Method & method, const unsigned int /* version */)
  {
    ar & method.module();
    ar & method.name();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::wf::Activity & act, const unsigned int /* version */)
  {
    ar & act.name();
    ar & act.method();
    ar & act.parameters();
    ar & act.properties();
  }
}}

#endif
