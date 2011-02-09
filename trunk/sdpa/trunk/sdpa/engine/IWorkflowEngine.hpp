/*
 * =====================================================================================
 *
 *       Filename:  IWorkflowEngine.hpp
 *
 *    Description:  Redefines the interface to gwes
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef IWORKFLOWENGINE_HPP
#define IWORKFLOWENGINE_HPP 1

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <boost/serialization/assume_abstract.hpp>
#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/we.hpp>
#include <we/mgmt/layer.hpp>

// Assume ids of type string
typedef std::string id_type;
typedef std::string result_type;
typedef std::string reason_type;
typedef std::string encoded_type;

typedef we::preference_t preference_t;
typedef we::mgmt::activity_information_t activity_information_t;
typedef we::mgmt::basic_layer<id_type, result_type, reason_type, encoded_type> IWorkflowEngine;
typedef we::mgmt::layer<id_type, we::activity_t> RealWorkflowEngine;

#endif //IWORKFLOWENGINE_HPP
