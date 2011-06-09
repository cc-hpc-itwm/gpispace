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

// Assume ids of type string
typedef std::string id_type;
typedef std::string result_type;
typedef std::string reason_type;
typedef std::string encoded_type;

struct IAgent
{
  IAgent ()
    : b_use_request_model (true)
  {}

    virtual void submit(const id_type & id, const encoded_type & ) = 0;
    virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

    virtual bool finished(const id_type & id, const result_type & result) = 0;
    virtual bool failed(const id_type & id, const result_type & result) = 0;
    virtual bool cancelled(const id_type & id) = 0;

    virtual ~IAgent () {}

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive&, const unsigned int ){}

    virtual void notifyAppGui(const result_type & result) {};

  bool use_request_model () const { return b_use_request_model; }
  void set_use_request_model (bool b) { b_use_request_model = b; }

private:
  bool b_use_request_model;
};

#endif //IAGENT_HPP
