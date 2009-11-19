/*
 * =====================================================================================
 *
 *       Filename:  ITokenParameter.h
 *
 *    Description:  abstract interface for the token parameters
 *
 *        Version:  1.0
 *        Created:  10/09/2009 04:05:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef GWES_TOKEN_PARAMETER_HPP
#define GWES_TOKEN_PARAMETER_HPP 1

#include <gwes/memory.hpp>

namespace gwes
{
  class ITokenParameter
  {
  public:
    typedef shared_ptr<ITokenParameter> ptr_t;

    virtual ~ITokenParameter() {}
  };
}

#endif
