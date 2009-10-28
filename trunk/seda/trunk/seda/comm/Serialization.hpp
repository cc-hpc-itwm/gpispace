/*
 * =====================================================================================
 *
 *       Filename:  Serialization.hpp
 *
 *    Description:  serialization code
 *
 *        Version:  1.0
 *        Created:  10/28/2009 01:12:12 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_SERIALIZATION_HPP
#define SEDA_COMM_SERIALIZATION_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <seda/comm/SedaMessage.hpp>

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, seda::comm::SedaMessage & msg, const unsigned int /* version */)
  {
    ar & msg.from();
    ar & msg.to();
    ar & msg.payload();
    ar & msg.type_code();
  }
}}


#endif
