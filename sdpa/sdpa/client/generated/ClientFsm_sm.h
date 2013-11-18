#ifndef _H_CLIENTFSM_SM
#define _H_CLIENTFSM_SM

#define SMC_USES_IOSTREAMS

#include <statemap.h>

namespace sdpa
{
    namespace client
    {
        // Forward declarations.
        class ClientFsm;
        class ClientFsm_Configured;
        class ClientFsm_Default;
        class ClientState;
        class ClientContext;
        class Client;

        class ClientState :
            public statemap::State
        {
        public:

            ClientState(const char *name, int stateId)
            : statemap::State(name, stateId)
            {};

            virtual void Entry(ClientContext&) {};
            virtual void Exit(ClientContext&) {};

            virtual void Cancel(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void CancelAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void ConfigNok(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void ConfigOk(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Delete(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void DeleteAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Error(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Query(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Results(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Retrieve(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Start(ClientContext& context, const sdpa::client::config_t & cfg);
            virtual void StatusReply(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Submit(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void SubmitAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Subscribe(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void SubscribeAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            virtual void Unknown(ClientContext& context, const seda::IEvent::Ptr & evt);

        protected:

            virtual void Default(ClientContext& context);
        };

        class ClientFsm
        {
        public:

            static ClientFsm_Configured Configured;
        };

        class ClientFsm_Default :
            public ClientState
        {
        public:

            ClientFsm_Default(const char *name, int stateId)
            : ClientState(name, stateId)
            {};

        };

        class ClientFsm_Configured :
            public ClientFsm_Default
        {
        public:
            ClientFsm_Configured(const char *name, int stateId)
            : ClientFsm_Default(name, stateId)
            {};

            void Cancel(ClientContext& context, const seda::IEvent::Ptr & evt);
            void CancelAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Delete(ClientContext& context, const seda::IEvent::Ptr & evt);
            void DeleteAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Error(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Query(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Results(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Retrieve(ClientContext& context, const seda::IEvent::Ptr & evt);
            void StatusReply(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Submit(ClientContext& context, const seda::IEvent::Ptr & evt);
            void SubmitAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Subscribe(ClientContext& context, const seda::IEvent::Ptr & evt);
            void SubscribeAck(ClientContext& context, const seda::IEvent::Ptr & evt);
            void Unknown(ClientContext& context, const seda::IEvent::Ptr & evt);
        };

        class ClientContext :
            public statemap::FSMContext
        {
        public:

            ClientContext(Client& owner)
            : _owner(owner)
            {
                setState(ClientFsm::Configured);
                ClientFsm::Configured.Entry(*this);
            };

            Client& getOwner() const
            {
                return (_owner);
            };

            ClientState& getState() const
            {
                if (_state == NULL)
                {
                    throw statemap::StateUndefinedException();
                }

                return (dynamic_cast<ClientState&>(*_state));
            };

            void Cancel(const seda::IEvent::Ptr & evt)
            {
                (getState()).Cancel(*this, evt);
            };

            void CancelAck(const seda::IEvent::Ptr & evt)
            {
                (getState()).CancelAck(*this, evt);
            };

            void ConfigNok(const seda::IEvent::Ptr & evt)
            {
                (getState()).ConfigNok(*this, evt);
            };

            void ConfigOk(const seda::IEvent::Ptr & evt)
            {
                (getState()).ConfigOk(*this, evt);
            };

            void Delete(const seda::IEvent::Ptr & evt)
            {
                (getState()).Delete(*this, evt);
            };

            void DeleteAck(const seda::IEvent::Ptr & evt)
            {
                (getState()).DeleteAck(*this, evt);
            };

            void Error(const seda::IEvent::Ptr & evt)
            {
                (getState()).Error(*this, evt);
            };

            void Query(const seda::IEvent::Ptr & evt)
            {
                (getState()).Query(*this, evt);
            };

            void Results(const seda::IEvent::Ptr & evt)
            {
                (getState()).Results(*this, evt);
            };

            void Retrieve(const seda::IEvent::Ptr & evt)
            {
                (getState()).Retrieve(*this, evt);
            };

            void Start(const sdpa::client::config_t & cfg)
            {
                (getState()).Start(*this, cfg);
            };

            void StatusReply(const seda::IEvent::Ptr & evt)
            {
                (getState()).StatusReply(*this, evt);
            };

            void Submit(const seda::IEvent::Ptr & evt)
            {
                (getState()).Submit(*this, evt);
            };

            void SubmitAck(const seda::IEvent::Ptr & evt)
            {
                (getState()).SubmitAck(*this, evt);
            };

            void Subscribe(const seda::IEvent::Ptr & evt)
            {
                (getState()).Subscribe(*this, evt);
            };

            void SubscribeAck(const seda::IEvent::Ptr & evt)
            {
                (getState()).SubscribeAck(*this, evt);
            };

            void Unknown(const seda::IEvent::Ptr & evt)
            {
                (getState()).Unknown(*this, evt);
            };

            static ClientState& valueOf(int stateId);

        private:

            Client& _owner;

        private:

            const static int MIN_INDEX;
            const static int MAX_INDEX;
            static ClientState* _States[];
        };
    }

}

#endif // _H_CLIENTFSM_SM
