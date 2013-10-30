/*
 * =====================================================================================
 *
 *       Filename:  CancelJobAckEvent.hpp
 *
 *    Description:  CancelJobAckEvent
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa { namespace events {
        class CancelJobAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<CancelJobAckEvent> Ptr;

      CancelJobAckEvent() : JobEvent("", "", "") {}

      CancelJobAckEvent(const address_t &a_from, const address_t &a_to, const sdpa::job_id_t& a_job_id)
      :  sdpa::events::JobEvent( a_from, a_to, a_job_id) {}

      std::string str() const
      {
        return "CancelJobAckEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleCancelJobAckEvent(this);
      }

      std::string const & result() const { return m_result; }
      std::string & result() { return m_result; }

      CancelJobAckEvent * set_result(std::string const &r)
      {
        m_result = r;
        return this;
      }
    private:
      std::string m_result;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
        ar & m_result;
      }
    };
}}

#endif
