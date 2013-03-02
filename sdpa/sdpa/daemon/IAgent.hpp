/*
 * =====================================================================================
 *
 *       Filename:  IAgent.hpp
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
#ifndef IAGENT_HPP
#define IAGENT_HPP 1

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <boost/serialization/assume_abstract.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

// Assume ids of type string
typedef std::string id_type;
typedef std::string result_type;
typedef std::string reason_type;
typedef std::string encoded_type;

inline const requirement_list_t empty_req_list()
{
  static requirement_list_t e_req_list;
  return e_req_list;
}

struct IAgent
{

  virtual void submit(const id_type & id, const encoded_type &, const requirement_list_t& = empty_req_list() ) = 0;
  virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

  virtual bool finished(const id_type & id, const result_type & result) = 0;
  virtual bool failed( const id_type& id
                       , const result_type & result
                       , int error_code
                       , std::string const & reason
                       ) = 0;

  virtual bool cancelled(const id_type & id) = 0;

  virtual bool finished(const id_type & id, const result_type & result, const id_type& ) { return false; }

  virtual ~IAgent () {}

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive&, const unsigned int ){}
};

#endif //IAGENT_HPP
