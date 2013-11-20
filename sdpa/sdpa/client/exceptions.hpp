/*
 * =====================================================================================
 *
 *       Filename:  exceptions.hpp
 *
 *    Description:  exception definitions related to the client
 *
 *        Version:  1.0
 *        Created:  10/22/2009 01:10:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_CLIENT_EXCEPTIONS_HPP
#define SDPA_CLIENT_EXCEPTIONS_HPP 1

#include <string>
#include <exception>

namespace sdpa { namespace client {
  class ClientException : public std::exception
  {
  public:
    explicit
    ClientException(const std::string &a_reason)
      : reason_(a_reason)
    { }

    virtual ~ClientException() throw() { }

    virtual const char *what() const throw()
    {
      return reason_.c_str();
    }
  private:
    std::string reason_;
  };
}}

#endif
