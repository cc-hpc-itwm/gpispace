/*
 * =====================================================================================
 *
 *       Filename:  Task.hpp
 *
 *    Description:
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
#ifndef TASK_HPP
#define TASK_HPP 1

#include <boost/serialization/access.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

using namespace std;

class Task
{
public:
  virtual ~Task(){}

  virtual void run() = 0;
  virtual void clear() = 0;
  virtual void print() = 0;
  virtual void print(const std::string& strFileName) = 0;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive&, const unsigned int /* file version */){}
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT( Task )

#endif //TASK_HPP
