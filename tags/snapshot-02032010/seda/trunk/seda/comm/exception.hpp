/* 
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  

*/

/*
 * =====================================================================================
 *
 *       Filename:  exception.hpp
 *
 *    Description:  seda communication exceptions
 *
 *        Version:  1.0
 *        Created:  10/23/2009 10:24:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_EXCEPTION_HPP
#define SEDA_COMM_EXCEPTION_HPP 1

#include <stdexcept>

namespace seda { namespace comm {
  class CommunicationError : public std::exception
  {
  public:
    explicit
    CommunicationError(const std::string &a_reason)
      : std::exception()
      , reason_(a_reason)
    {}

    virtual ~CommunicationError() throw() {}

    const char *what() const throw ()
    {
      return reason_.c_str();
    }
  private:
    std::string reason_;
  };
}}

#endif
