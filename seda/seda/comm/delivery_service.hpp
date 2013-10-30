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
#include <boost/date_time/posix_time/posix_time.hpp>

namespace seda { namespace comm {
  template <typename MessageType, typename MessageIdType, typename Destination>
	struct msg_info
	{
	  typedef MessageType message_type;
	  typedef MessageIdType message_id_type;
	  typedef Destination destination_type;
	  typedef unsigned long time_type;
	  typedef std::size_t size_type;

	  msg_info(message_type m
			 , message_id_type id
			 , size_type retries
			 , time_type last_send
			 , time_type deliver_timeout
			 , destination_type *dest)
		: msg_id(id)
		, msg(m)
		, retry_counter(retries)
		, tstamp_of_last_send(last_send)
		, timeout(deliver_timeout)
		, destination(dest)
	  {
	  }

	  message_id_type msg_id;			//! the corresponding message id
	  message_type msg;				//! the message that must be acknowledged
	  size_type retry_counter;		//! how often should the message be resent
	  time_type tstamp_of_last_send;	//! how much time may pass between two resends
	  time_type timeout;				//! how much time may pass between two resends
	  destination_type *destination;    //! where to deliver the events (must provide send() method)
	};

  template <typename MessageType, typename MessageIdType, typename StageType>
	class delivery_service
	{
	  private:
		typedef msg_info<MessageType, MessageIdType, StageType> msg_info_type;
		typedef std::deque<msg_info_type> msg_info_list;
		typedef typename msg_info_list::iterator iterator;

	  public:
		typedef StageType stage_type;
		typedef MessageType message_type;
		typedef MessageIdType message_id_type;
		typedef typename msg_info_type::time_type time_type;
		typedef typename msg_info_type::size_type size_type;
		typedef boost::function<void (message_type)> callback_handler;

		delivery_service(boost::asio::io_service &asio_service, time_type interval_milliseconds = 500)
		  : io_service_(asio_service)
		  , timer_(asio_service)
		  , timer_timeout_(interval_milliseconds)
		{ }

		~delivery_service()
		{
		  try
		  {
			stop();
		  }
		  catch (...)
		  {
			LOG(ERROR, "error during delivery-service shutdown!");
		  }
		}

		void register_callback_handler(callback_handler h)
		{
		  handler_ = h;
		}

		void start()
		{
		  start_timer();
		}

		void stop()
		{
		  // cancel timer
		  timer_.cancel();
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
		send( stage_type *destination
                    , message_type msg
                    , const message_id_type &msg_id
                    , time_type timeout
                    , size_type retries = std::numeric_limits<size_type>::max()
                    )
		{
                  assert ( destination != NULL );

		  destination->send(msg);

		  if (retries)
		  {
			msg_info_type m_info(msg, msg_id, retries, time(NULL), timeout, destination);
			boost::unique_lock<boost::recursive_mutex> lock(mtx_);
			pending_messages_.push_back(m_info);
			DLOG(TRACE, "enqued message for retransmission: " << msg_id);
		  }

		  return msg_id;
		}
	  private:
		void start_timer()
		{
		  timer_.expires_from_now(boost::posix_time::milliseconds(timer_timeout_));
		  timer_.async_wait(boost::bind(&delivery_service::timer_timedout, this, boost::asio::placeholders::error));
		  DLOG(TRACE, "timer started, expires in " << timer_.expires_from_now());
		}

		void timer_timedout(const boost::system::error_code &error)
		{
		  if (! error)
		  {
			resend_messages();
			// reschedule next timer
			start_timer();
		  }
		  else
		  {
			// cancelled
		  }
		}

		void resend_messages()
		{
		  const time_type now(time(NULL));

		  boost::unique_lock<boost::recursive_mutex> lock(mtx_);

                  iterator m (pending_messages_.begin());
                  while ( m != pending_messages_.end() )
                  {
			if (m->retry_counter > 0)
			{
			  if (must_be_resent(*m, now))
                          {
                            resend_message(*m, now);
                            m->retry_counter--;
                          }

                          ++m;
			}
			else
			{
			  try
			  {
				handler_(m->msg);
			  }
			  catch (const std::exception &ex)
			  {
				LOG(ERROR, "callback handler could not be executed: " << ex.what());
			  }

			  // remove it
			  LOG(ERROR, "delivery of message " << m->msg_id << " failed!");
			  m = pending_messages_.erase(m);
			}
		  }
		}

		void resend_message(msg_info_type & m_info, time_type tstamp)
		{
		  LOG(WARN, "resending message: " << m_info.msg_id);
		  m_info.destination->send(m_info.msg);
		  m_info.tstamp_of_last_send = tstamp;
		}

		bool must_be_resent(const msg_info_type &m_info, const time_type current_time)
		{
		  if (m_info.retry_counter > 0
		   && (m_info.tstamp_of_last_send + m_info.timeout < current_time))
		  {
			return true;
		  }

		  return false;
		}

		boost::asio::io_service &io_service_;
		boost::asio::deadline_timer timer_;
		time_type timer_timeout_; // milli seconds

		boost::recursive_mutex mtx_;
		msg_info_list pending_messages_;
		callback_handler handler_;
	};
}}

#endif
