#ifndef SDPA_EVENT_HPP
#define SDPA_EVENT_HPP 1

#include <string>

#include <seda/IEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa {
	namespace events {
		class SDPAEvent : public seda::IEvent {
		public:
			typedef sdpa::shared_ptr<SDPAEvent> Ptr;

			typedef std::string address_t;

			SDPAEvent(const address_t &from, const address_t &to);
			~SDPAEvent() {}

			const address_t & from() const { return from_; }
			const address_t & to() const { return to_; }
		private:
			address_t from_;
			address_t to_;
		};
	}
}

#endif // SDPA_EVENT_HPP
