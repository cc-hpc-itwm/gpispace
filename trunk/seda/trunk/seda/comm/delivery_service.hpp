/*
 * =====================================================================================
 *
 *       Filename:  delivery_service.hpp
 *
 *    Description:  implements a service responsible for resending messages and
 *					handling ackknowledges
 *
 *        Version:  1.0
 *        Created:  12/09/2009 10:56:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_DELIVERY_SERVICE_HPP
#define SEDA_COMM_DELIVERY_SERVICE_HPP 1

#include <deque>
#include <fhglog/fhglog.hpp>
#include <boost/thread.hpp>

namespace seda { namespace comm {
  template <typename MessageType, typename MessageIdType>
	struct msg_info
	{
	  typedef MessageType message_type;
	  typedef MessageIdType message_id_type;
	  typedef unsigned long time_type;
	  typedef std::size_t size_type;

	  msg_info(const message_type &m
			 , const message_id_type &id
			 , size_type retries
			 , time_type last_send
			 , time_type deliver_timeout)
		: msg_id(id)
		, msg(m)
		, retry_counter(retries)
		, tstamp_of_last_send(last_send)
		, timeout(deliver_timeout)
	  { }

	  message_id_type msg_id;			//! the corresponding message id
	  message_type msg;				//! the message that must be acknowledged
	  size_type retry_counter;		//! how often should the message be resent
	  time_type tstamp_of_last_send;	//! how much time may pass between two resends
	  time_type timeout;				//! how much time may pass between two resends
	};

  template <typename MessageType, typename MessageIdType>
	class delivery_service
	{
	  private:
		typedef msg_info<MessageType, MessageIdType> msg_info_type;
		typedef std::deque<msg_info_type> msg_info_list;
		typedef typename msg_info_list::iterator iterator;

	  public:
		typedef MessageType message_type;
		typedef MessageIdType message_id_type;
		typedef typename msg_info_type::time_type time_type;
		typedef typename msg_info_type::size_type size_type;

		delivery_service(const seda::Stage::Ptr next_stage, boost::asio::io_service &asio_service)
		  : output_stage_(next_stage)
		  , io_service(asio_service)
		{}

		void start()
		{
		  // register first deadline timer
		}

		void stop()
		{
		  // cancel timer
		}

		bool acknowledge(const message_id_type &message_id)
		{
		  boost::unique_lock<boost::recursive_mutex> lock(mtx_);

		  for (iterator it(pending_messages_.begin()); it != pending_messages_.end(); ++it)
		  {
			if (it->msg_id == message_id)
			{
			  DLOG(TRACE, "acknowledged message: " << message_id);
			  pending_messages_.erase(it);
			  return true;
			}
		  }
		  return false;
		}

		bool cancel(const message_id_type &message_id)
		{
		  boost::unique_lock<boost::recursive_mutex> lock(mtx_);
		  for (iterator it(pending_messages_.begin()); it != pending_messages_.end(); ++it)
		  {
			if (it->msg_id == message_id)
			{
			  DLOG(TRACE, "cancelled message: " << message_id);
			  pending_messages_.erase(it);
			  return true;
			}
		  }
		  return false;
		}

		const message_id_type &
		send(const message_type &msg
		   , const message_id_type &msg_id
		   , time_type timeout
		   , size_type retries = std::numeric_limits<size_type>::max())
		{
		  output_stage_->send(msg);

		  if (retries)
		  {
			boost::unique_lock<boost::recursive_mutex> lock(mtx_);
			msg_info_type m_info(msg, msg_id, retries, time(NULL), timeout);
			pending_messages_.push_back(m_info);
			DLOG(TRACE, "enqued message for retransmission: " << msg_id);
		  }

		  return msg_id;
		}

	  private:
		seda::Stage::Ptr output_stage_;
		boost::asio::io_service &io_service_;

		boost::recursive_mutex mtx_;
		boost::condition_variable_any recv_cond_;
		msg_info_list pending_messages_;
	};
}}

#endif
