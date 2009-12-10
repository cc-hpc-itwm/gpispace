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
        typedef std::tr1::shared_ptr<SedaMessage> Ptr;

        explicit
          SedaMessage()
          : from_(address_type())
          , to_(address_type())
          , payload_(payload_type())
        {}
        SedaMessage(const address_type & a_from, const address_type & a_to, const payload_type & a_payload)
          : from_(a_from)
          , to_(a_to)
          , payload_(a_payload)
        {}

        SedaMessage(const SedaMessage &other)
          : UserEvent()
          , from_(other.from())
          , to_(other.to())
          , payload_(other.payload())
        {}

        virtual ~SedaMessage() {}

        SedaMessage &operator=(const SedaMessage &rhs) {
          if (this != &rhs) {
            from_ = rhs.from();
            to_ = rhs.to();
            payload_ = rhs.payload();
            encode_buf_ = rhs.encode_buf_;
            strrep_buf_ = rhs.strrep_buf_;
          }
          return *this;
        }

        std::string str() const;

        const address_type & from() const { return from_; }
        address_type & from() { return from_; }
        const address_type & to() const { return to_; }
        address_type & to() { return to_; }
        const payload_type & payload() const { return payload_; }
        payload_type & payload() { return payload_; }

        void from(const address_type &new_from) { from_ = new_from; reset_buffers(); }
        void to(const address_type &new_to) { to_ = new_to; reset_buffers(); }
        void payload(const payload_type &new_payload) { payload_ = new_payload; reset_buffers(); }
      private:
        void reset_buffers()
        {
          encode_buf_.clear();
          strrep_buf_.clear();
        }

        address_type from_;
        address_type to_;
        payload_type payload_;
        mutable std::string encode_buf_;
        mutable std::string strrep_buf_;
    };    
  }}

#endif
