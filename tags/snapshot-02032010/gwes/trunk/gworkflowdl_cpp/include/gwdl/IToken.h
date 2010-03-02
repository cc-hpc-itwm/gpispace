/*
 * =====================================================================================
 *
 *       Filename:  IToken.h
 *
 *    Description:  slim interface to a token 
 *
 *        Version:  1.0
 *        Created:  11/13/2009 11:04:35 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef GWDL_I_TOKEN_HPP
#define GWDL_I_TOKEN_HPP 1

namespace gwdl
{
  class IToken
  {
  public:
    typedef long id_t;

    virtual ~IToken() {}
    
    virtual id_t getID() const = 0;
  };
}

#endif
