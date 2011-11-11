
//
// Finite state machine for the SDPA client
//

#if __GNUC__ >4 || ( __GNUC__==4 && __GNUC_MINOR__ > 1)
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wunused"
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif



#include "Client.hpp"
#include <sdpa/types.hpp>
#include <sdpa/client/types.hpp>
#include <seda/IEvent.hpp>
#include "ClientFsm_sm.h"

using namespace statemap;

namespace sdpa
{
    namespace client
    {
        // Static class declarations.
        ClientFsm_Unconfigured ClientFsm::Unconfigured("ClientFsm::Unconfigured", 0);
        ClientFsm_Configuring ClientFsm::Configuring("ClientFsm::Configuring", 1);
        ClientFsm_Configured ClientFsm::Configured("ClientFsm::Configured", 2);

        const int ClientContext::MIN_INDEX = 0;
        const int ClientContext::MAX_INDEX = 2;
        ClientState* ClientContext::_States[] = 
{
            &ClientFsm::Unconfigured,
            &ClientFsm::Configuring,
            &ClientFsm::Configured
        };

        ClientState& ClientContext::valueOf(int stateId)
        {
            if (stateId < MIN_INDEX || stateId > MAX_INDEX)
            {
                throw (
                    IndexOutOfBoundsException(
                        stateId, MIN_INDEX, MAX_INDEX));
            }

            return (static_cast<ClientState&>(*(_States[stateId])));
        }

        void ClientState::Cancel(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::CancelAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::ConfigNok(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::ConfigOk(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Delete(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::DeleteAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Error(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Query(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Results(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Retrieve(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Shutdown(ClientContext& context)
        {
            Default(context);
            return;
        }

        void ClientState::Start(ClientContext& context, const sdpa::client::config_t & cfg)
        {
            Default(context);
            return;
        }

        void ClientState::StatusReply(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Submit(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::SubmitAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Subscribe(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::SubscribeAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Unknown(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Default(context);
            return;
        }

        void ClientState::Default(ClientContext& context)
        {
            throw (
                TransitionUndefinedException(
                    context.getState().getName(),
                    context.getTransition()));

            return;
        }

        void ClientFsm_Unconfigured::Default(ClientContext& context)
        {


            return;
        }

        void ClientFsm_Unconfigured::Shutdown(ClientContext& context)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_shutdown();
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Unconfigured::Start(ClientContext& context, const sdpa::client::config_t & cfg)
        {
            Client& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_configure(cfg);
                context.setState(ClientFsm::Configuring);
            }
            catch (...)
            {
                context.setState(ClientFsm::Configuring);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void ClientFsm_Configuring::ConfigNok(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_config_nok();
                ctxt.action_store_reply(evt);
                context.setState(ClientFsm::Unconfigured);
            }
            catch (...)
            {
                context.setState(ClientFsm::Unconfigured);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void ClientFsm_Configuring::ConfigOk(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_config_ok();
                ctxt.action_store_reply(evt);
                context.setState(ClientFsm::Configured);
            }
            catch (...)
            {
                context.setState(ClientFsm::Configured);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void ClientFsm_Configuring::Default(ClientContext& context)
        {


            return;
        }

        void ClientFsm_Configured::Cancel(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_cancel(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::CancelAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Delete(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_delete(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::DeleteAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Error(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Query(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_query(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Results(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Retrieve(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_retrieve(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Shutdown(ClientContext& context)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_shutdown();
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::StatusReply(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Submit(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_submit(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::SubmitAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Subscribe(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_subscribe(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::SubscribeAck(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }

        void ClientFsm_Configured::Unknown(ClientContext& context, const seda::IEvent::Ptr & evt)
        {
            Client& ctxt(context.getOwner());

            ClientState& EndStateName = context.getState();

            context.clearState();
            try
            {
                ctxt.action_store_reply(evt);
                context.setState(EndStateName);
            }
            catch (...)
            {
                context.setState(EndStateName);
                throw;
            }

            return;
        }
    }
}
