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

namespace we { namespace mgmt { namespace detail {
  template <typename Id
	, typename Net
	, typename Status
	, typename Data>
	struct descriptor
	{
	  typedef Id id_type;
	  typedef Net net_type;
	  typedef Status status_type;
	  typedef Data data_type;
	  typedef std::vector<id_type> container_type;

	  enum descriptor_category
	  {
		NET
		  , ACTIVITY
	  };

	  descriptor(const id_type & id_, descriptor_category category_)
		: id(id_)
		  , category(category_)
	  {}

	  descriptor(const descriptor &other)
	  {
		if (&other == this) return;

		id = other.id;
		category = other.category;
		parent = other.parent;

		switch (category)
		{
		  case NET:
			net = other.net;
			children = other.children;
			break;
		  case ACTIVITY:
			data = other.data;
			break;
		  default:
			assert(false);
		}
	  }

	  inline bool is_net() const { return category == NET; }
	  inline bool is_activity() const { return category == ACTIVITY; }

	  id_type id;
	  descriptor_category category;
	  id_type parent;
	  net_type net;
	  data_type data;
	  container_type children;
	};
}}}

#endif
