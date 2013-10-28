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

#ifndef SEDA_COMM_MESSAGE_HPP
#define SEDA_COMM_MESSAGE_HPP 1

#include <seda/UserEvent.hpp>

#include <string>

namespace seda {
  namespace comm {
    typedef std::string payload_type;
    typedef std::string address_type;

    class From {
      public:
        explicit From(const address_type &value)
          : val(value) {}
        address_type val;
    };
    class To {
      public:
        explicit To(const address_type &value)
          : val(value) {}
        address_type val;
    };

    class SedaMessage : public seda::UserEvent {
      public:
        typedef seda::shared_ptr<SedaMessage> Ptr;
		typedef std::string payload_type;
		typedef std::string address_type;
		typedef unsigned long message_id_type;

        explicit
          SedaMessage()
          : from_(address_type())
          , to_(address_type())
          , payload_(payload_type())
		  , message_id_(message_id_type())
        {}
        SedaMessage(const address_type & a_from, const address_type & a_to, const payload_type & a_payload, const message_id_type & a_message_id)
          : from_(a_from)
          , to_(a_to)
          , payload_(a_payload)
		  , message_id_(a_message_id)
        {}

        std::string str() const;

        const address_type & from() const { return from_; }
        address_type & from() { return from_; }
        const address_type & to() const { return to_; }
        address_type & to() { return to_; }
        const payload_type & payload() const { return payload_; }
        payload_type & payload() { return payload_; }
        const message_id_type & id() const { return message_id_; }
        message_id_type & id() { return message_id_; }

        void from(const address_type &new_from) { from_ = new_from; }
        void to(const address_type &new_to) { to_ = new_to; }
        void payload(const payload_type &new_payload) { payload_ = new_payload; }
		void id(const message_id_type &new_message_id) { message_id_ = new_message_id; }
      private:
        address_type from_;
        address_type to_;
        payload_type payload_;
        message_id_type message_id_;
    };    
  }}

#endif
