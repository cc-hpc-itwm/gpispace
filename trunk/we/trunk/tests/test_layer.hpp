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

#include <sstream>
#include <boost/bind.hpp>
#include <we/mgmt/layer.hpp>

namespace test {
namespace detail
{
  template <typename I>
  struct id_generator
  {
	explicit
	id_generator(I initial=0) : id(initial) {}

	inline I operator++() { return ++id; }
  private:
	I id;
  };

  template <>
  struct id_generator<std::string>
  {
	inline const std::string operator++()
	{
	  unsigned long id = ++number;
	  std::ostringstream sstr;
	  sstr << id;
	  return sstr.str();
	}
  private:
	id_generator<unsigned long> number;
  };
}

template <typename Layer>
struct sdpa_daemon : public we::mgmt::basic_layer<typename Layer::id_type>
{
  typedef Layer layer_type;
  typedef typename layer_type::id_type id_type;

  sdpa_daemon()
	: mgmt_layer_(*this, boost::bind(&sdpa_daemon::gen_id, this))
  {}

  id_type gen_id() { return ++id_; }

  void submit(const id_type & id, const std::string & desc)
  {
	std::cout << "submit[" << id << "] = " << desc << std::endl;
  }
  bool cancel(const id_type & id, const std::string & desc)
  {
	std::cout << "cancel[" << id << "] = " << desc << std::endl; return true;
  }
  bool finished(const id_type & id, const std::string & desc)
  {
	std::cout << "finished[" << id << "] = " << desc << std::endl; return true;
  }
  bool failed(const id_type & id, const std::string & desc)
  {
	std::cout << "finished[" << id << "] = " << desc << std::endl; return true;
  }
  bool cancelled(const id_type & id)
  {
	std::cout << "cancelled[" << id << "]" << std::endl; return true;
  }

  inline layer_type & layer() { return mgmt_layer_; }

  private:
	detail::id_generator<id_type> id_;
	layer_type mgmt_layer_;
};
}

#endif
