/*
 * =====================================================================================
 *
 *       Filename:  traits.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2010 04:16:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_TRAITS_HPP
#define WE_MGMT_BITS_TRAITS_HPP 1

namespace we { namespace mgmt { namespace detail {
  template <typename IdType=unsigned long>
  struct id_traits
  {
  public:
	typedef IdType type;

	static type next()
	{
	  static type _id = zero();
	  return _id++;
	}

	static type zero()
	{
	  return 0;
	}
  };
}}}

#endif
