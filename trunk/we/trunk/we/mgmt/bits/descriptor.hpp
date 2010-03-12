/*
 * =====================================================================================
 *
 *       Filename:  descriptor.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/04/2010 11:49:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_DESCRIPTOR_HPP
#define WE_MGMT_LAYER_DESCRIPTOR_HPP 1

#include <boost/unordered_set.hpp>
#include <we/mgmt/bits/synch_net.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename Id
	, typename Net
	, typename Status
	, typename Data>
	struct descriptor
	{
	  typedef Id id_type;
	  typedef Net real_net_type;
	  typedef synch_net<Net> synch_net_type;

	  typedef Status status_type;
	  typedef Data data_type;
	  typedef boost::unordered_set<id_type> children_type;

	  struct
	  {
		bool cancel : 1;
		bool cancelling : 1;
		bool cancelled : 1;
		bool suspended : 1;
	  } flags;

	  enum descriptor_category
	  {
		NET
		  , ACTIVITY
	  };

	  descriptor(const id_type & id_, descriptor_category category_)
		: id(id_)
		  , category(category_)
		  , net(real_net)
	  {}

	  descriptor(const id_type & id_, real_net_type const& n)
		: id(id_)
		, category(NET)
		, real_net(n)
		, net(real_net)
	  {}

	  descriptor(descriptor const& other)
		: id(other.id)
		, parent(other.parent)
		, category(other.category)
		, real_net(other.real_net)
		, net(real_net)
		, data(other.data)
		, children(other.children)
	  {
	  }

	  inline bool is_net() const { return category == NET; }
	  inline bool is_activity() const { return category == ACTIVITY; }

	  real_net_type & get_real_net() { return real_net; }
	  id_type id;
	  id_type parent;
  private:
	  descriptor_category category;
	  real_net_type real_net;
  public:
	  synch_net_type net;
	  data_type data;
	  children_type children;
	};
}}}

#endif
