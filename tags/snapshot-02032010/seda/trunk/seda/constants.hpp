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

#ifndef SEDA_CONSTANTS_HPP
#define SEDA_CONSTANTS_HPP 1

#include <string>

namespace seda {
    extern std::size_t SEDA_MAX_QUEUE_SIZE;
    extern unsigned long SEDA_DEFAULT_TIMEOUT;
    extern const std::string SEDA_SYSTEM_STAGE_NAME;
}

#endif // !SEDA_CONSTANTS_HPP
