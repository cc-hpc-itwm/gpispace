/*
 * =====================================================================================
 *
 *       Filename:  test_layer.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2010 02:53:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_TESTS_TEST_LAYER_HPP
#define WE_TESTS_TEST_LAYER_HPP 1

#include <we/mgmt/layer.hpp>

template <typename Layer>
struct sdpa_daemon : public we::mgmt::basic_layer<>
{
  typedef Layer mgmt_layer_type;
  typedef unsigned long id_type;

  sdpa_daemon()
	: mgmt_layer(*this)
  {}

  mgmt_layer_type mgmt_layer;

  id_type submit(const id_type & id, const std::string & desc)
  {
	std::cout << "submit[" << id << "] = " << desc << std::endl; return id;
  }
  bool cancel(const id_type & id, const std::string & desc)
  {
	std::cout << "cancel[" << id << "] = " << desc << std::endl; return id;
  }
  bool finished(const id_type & id, const std::string & desc)
  {
	std::cout << "finished[" << id << "] = " << desc << std::endl; return id;
  }
  bool failed(const id_type & id, const std::string & desc)
  {
	std::cout << "finished[" << id << "] = " << desc << std::endl; return id;
  }
  bool cancelled(const id_type & id)
  {
	std::cout << "cancelled[" << id << "]" << std::endl; return id;
  }
};

#endif
