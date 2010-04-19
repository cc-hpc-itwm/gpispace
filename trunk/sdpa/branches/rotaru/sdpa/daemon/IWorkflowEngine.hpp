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
#ifndef WE_MGMT_BASIC_LAYER_HPP
#define WE_MGMT_BASIC_LAYER_HPP 1

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

//#include <we/mgmt/basic_layer.hpp>

#include <string>

namespace we { namespace mgmt {
  template <typename id_type=unsigned long
		  , typename result_type=std::string
		  , typename reason_type=std::string
		  , typename encoded_type=std::string
  >
  struct basic_layer
  {
	virtual void submit(const id_type & id, const encoded_type & ) = 0;
	virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

	virtual bool finished(const id_type & id, const result_type & result) = 0;
	virtual bool failed(const id_type & id, const result_type & result) = 0;
	virtual bool cancelled(const id_type & id) = 0;

	template <class Archive>
	void serialize(Archive& ar, const unsigned int file_version )
	{

	}

	friend class boost::serialization::access;
  };
}}


// sdpa part

// Assume ids of type string
typedef we::mgmt::basic_layer<std::string> IWorkflowEngine;
typedef we::mgmt::basic_layer<std::string> IDaemon;

typedef std::string id_type;
typedef std::string result_type;
typedef std::string reason_type;
typedef std::string encoded_type;

#endif
