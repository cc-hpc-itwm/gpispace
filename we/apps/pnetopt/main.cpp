#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory> /* std::auto_ptr */

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"

#include "Invalidatable.h"
#include "LuaIterator.h"
#include "RangeAdaptor.h"

#include <we/we.hpp>

#define foreach BOOST_FOREACH

namespace {

/*
 * Everything is C++ owned and is never deleted.
 * Except iterators: nobody refers them anyway, so we can garbage-collect them.
 */

template<class P, class E, class T>
class Optimizer {
    typedef petri_net::pid_t pid_t;
    typedef P place_t;

    typedef petri_net::tid_t tid_t;
    typedef we::type::transition_t<P, E, T> transition_t;

    typedef petri_net::net<P, transition_t, E, T> pnet_t;

    typedef typename transition_t::port_id_t port_id_t;
    typedef typename transition_t::port_t port_t;

    lua_State *L;

    class PetriNet;
    PetriNet petriNet_;

    public:

    Optimizer(pnet_t &pnet):
        petriNet_(pnet)
    {
        L = lua_open();
        luaL_openlibs(L);

        luabridge::getGlobalNamespace(L)
            .beginNamespace("pnetopt")
                .template beginClass<PetriNet>("PetriNet")
                    .addFunction("__tostring", &PetriNet::toString)
                    .addFunction("places", &PetriNet::placesIterator)
                    .addFunction("transitions", &PetriNet::transitionIterator)
                .endClass()
                .template beginClass<Place>("Place")
                    .addFunction("__tostring", &Place::name)
                    .addFunction("name", &Place::name)
                    .addFunction("setName", &Place::setName)
                    .addFunction("connectedPorts", &Place::connectedPortsIterator)
                    .addFunction("remove", &Place::remove)
                .endClass()
                .template beginClass<PlacesIterator>("PlacesIterator")
                    .addFunction("__tostring", &PlacesIterator::toString)
                    .addFunction("__call", &PlacesIterator::call)
                    .addFunction("__len", &PlacesIterator::size)
                .endClass()
                .template beginClass<ConnectedPortsIterator>("ConnectedPortsIterator")
                    .addFunction("__tostring", &ConnectedPortsIterator::toString)
                    .addFunction("__call", &ConnectedPortsIterator::call)
                    .addFunction("__len", &ConnectedPortsIterator::size)
                .endClass()
                .template beginClass<Transition>("Transition")
                    .addFunction("__tostring", &Transition::name)
                    .addFunction("name", &Transition::name)
                    .addFunction("setName", &Transition::setName)
                    .addFunction("ports", &Transition::portIterator)
                    .addFunction("remove", &Transition::remove)
                .endClass()
                .template beginClass<TransitionsIterator>("TransitionsIterator")
                    .addFunction("__tostring", &TransitionsIterator::toString)
                    .addFunction("__call", &TransitionsIterator::call)
                    .addFunction("__len", &TransitionsIterator::size)
                .endClass()
                .template beginClass<Port>("Port")
                    .addFunction("__tostring", &Port::name)
                    .addFunction("transition", &Port::transition)
                    .addFunction("name", &Port::name)
                    .addFunction("isInput", &Port::isInput)
                    .addFunction("isOutput", &Port::isOutput)
                    .addFunction("isTunnel", &Port::isTunnel)
                    .addFunction("isRead", &Port::isRead)
                    .addFunction("connectedPlace", &Port::connectedPlace)
                    .addFunction("connect", &Port::connect)
                    .addFunction("disconnect", &Port::disconnect)
                .endClass()
                .template beginClass<PortsIterator>("PortsIterator")
                    .addFunction("__tostring", &PortsIterator::toString)
                    .addFunction("__call", &PortsIterator::call)
                    .addFunction("__len", &PortsIterator::size)
                .endClass()
        ;

#if 0
                .template beginClass<PetriNet>("PetriNet")
                    .addFunction("__tostring", &PetriNet::__tostring)
                    .addFunction("places", &PetriNet::places)
                    .addFunction("transitions", &PetriNet::transitions)
                .endClass()
                .template beginClass<Places>("Places")
                    .addFunction("__tostring", &Places::__tostring)
                    .addFunction("__len", &Places::__len)
                    .addFunction("all", &Places::all)
                .endClass()
                .template beginClass<PlacesIterator>("PlacesIterator")
                    .addFunction("__tostring", &PlacesIterator::__tostring)
                    .addFunction("__call", &PlacesIterator::__call)
                .endClass()
                .template beginClass<Place>("Place")
                    .addFunction("__tostring", &Place::name)
                    .addFunction("__eq", &Place::__eq)
                    .addFunction("id", &Place::id)
                    .addFunction("name", &Place::name)
                    .addFunction("setName", &Place::setName)
                    .addFunction("inConnections", &Place::in_connections)
                    .addFunction("outConnections", &Place::out_connections)
                    .addFunction("remove", &Place::remove)
                .endClass()
                .template beginClass<AdjacentPortsIterator>("AdjacentPortsIterator")
                    .addFunction("__tostring", &AdjacentPortsIterator::__tostring)
                    .addFunction("__call", &AdjacentPortsIterator::__call)
                .endClass()
                .template beginClass<Transitions>("Transitions")
                    .addFunction("__tostring", &Transitions::__tostring)
                    .addFunction("__len", &Transitions::__len)
                    .addFunction("all", &Transitions::all)
                .endClass()
                .template beginClass<TransitionsIterator>("TransitionsIterator")
                    .addFunction("__tostring", &TransitionsIterator::__tostring)
                    .addFunction("__call", &TransitionsIterator::__call)
                .endClass()
                .template beginClass<Transition>("Transition")
                    .addFunction("__tostring", &Transition::name)
                    .addFunction("name", &Transition::name)
                    .addFunction("setName", &Transition::setName)
                    .addFunction("ports", &Transition::ports)
                    .addFunction("subnet", &Transition::subnet)
                    .addFunction("expression", &Transition::expression)
                    .addFunction("condition", &Transition::condition)
                    .addFunction("remove", &Transition::remove)
                .endClass()
                .template beginClass<Expression>("Expression")
                    .addFunction("__tostring", &Expression::__tostring)
                    .addFunction("isEmpty", &Expression::isEmpty)
                .endClass()
                .template beginClass<Condition>("Condition")
                    .addFunction("__tostring", &Condition::__tostring)
                    .addFunction("isConstTrue", &Condition::isConstTrue)
                .endClass()
                .template beginClass<Ports>("Ports")
                    .addFunction("__tostring", &Ports::__tostring)
                    .addFunction("__len", &Ports::__len)
                    .addFunction("all", &Ports::all)
                .endClass()
                .template beginClass<PortsIterator>("PortsIterator")
                    .addFunction("__tostring", &PortsIterator::__tostring)
                    .addFunction("__call", &PortsIterator::__call)
                .endClass()
                .template beginClass<Port>("Port")
                    .addFunction("__tostring", &Port::name)
                    .addFunction("name", &Port::name)
                    .addFunction("transition", &Port::transition)
                    .addFunction("place", &Port::place)
                    .addFunction("associatedPlace", &Port::associatedPlace)
                    .addFunction("isInput", &Port::isInput)
                    .addFunction("isOutput", &Port::isOutput)
                    .addFunction("isTunnel", &Port::isTunnel)
                .endClass()
                ;
#endif
        luabridge::push(L, &petriNet_);
        lua_setfield(L, LUA_GLOBALSINDEX, "net");
    }

    ~Optimizer() {
        lua_close(L);
    }

    void operator()(const char *filename) {
        if (luaL_dofile(L, filename) != 0) {
            std::runtime_error e((boost::format("Lua error: %1%") % lua_tostring(L, -1)).str());
            lua_pop(L, 1);
            throw e;
        }
    }

    private:

    class Place;
    typedef boost::unordered_map<pid_t, Place *> IdPlaceMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdPlaceMap>, IdPlaceMap> Places;
    typedef pnetopt::LuaIterator<Places> PlacesIterator;
    typedef RefCountedObjectPtr<PlacesIterator> PlacesIteratorPtr;

    class Transition;
    typedef boost::unordered_map<tid_t, Transition *> IdTransitionMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdTransitionMap>, IdTransitionMap> Transitions;
    typedef pnetopt::LuaIterator<Transitions> TransitionsIterator;
    typedef RefCountedObjectPtr<TransitionsIterator> TransitionsIteratorPtr;

    class PetriNet: public boost::noncopyable {
        /** Petri net reference. */
        pnet_t &pnet_;

        /** All the ever created places, both valid and invalid. */
        std::vector<Place *> places_;

        /** Map of an id to a valid place. */
        IdPlaceMap id2place_;

        /** Iterators over the id2place_ container. */
        std::vector<PlacesIteratorPtr> placesIterators_;

        /** All the ever created transitions, both valid and invalid. */
        std::vector<Transition *> transitions_;

        /** Map of an id to a valid transition. */
        IdTransitionMap id2transition_;

        /** Iterators over the id2transition_ container. */
        std::vector<TransitionsIteratorPtr> transitionIterators_;

        public:

        PetriNet(pnet_t &pnet):
            pnet_(pnet)
        {
            for (typename pnet_t::place_const_it it = pnet_.places(); it.has_more(); ++it) {
                getPlace(*it);
            }
            for (typename pnet_t::transition_const_it it = pnet_.transitions(); it.has_more(); ++it) {
                getTransition(*it);
            }
        }

        pnet_t &pnet() const { return pnet_; }

        ~PetriNet() {
            foreach(Place *place, places_) {
                delete place;
            }
        }

        const char *toString() const {
            return "PetriNet";
        }

        Place *getPlace(pid_t pid) {
            Place *&result = id2place_[pid];

            if (!result) {
                place_t &place = const_cast<place_t &>(pnet_.get_place(pid));

                places_.reserve(places_.size() + 1);
                result = new Place(this, pid, place);
                places_.push_back(result);

                invalidatePlacesIterators();
            }

            result->ensureValid();
            
            return result;
        }

        void removePlace(Place *place) {
            assert(id2place_[place->id()] == place);

            id2place_.erase(place->id());
            invalidatePlacesIterators();
        }

        Places places() const {
            return pnetopt::rangeAdaptor(id2place_ | boost::adaptors::map_values, id2place_);
        }

        PlacesIteratorPtr placesIterator() {
            PlacesIteratorPtr result(new PlacesIterator(places()));
            placesIterators_.push_back(result);
            return result;
        }

        void invalidatePlacesIterators() {
            foreach(PlacesIteratorPtr &iterator, placesIterators_) {
                iterator->invalidate();
            }
            placesIterators_.clear();
        }

        Transition *getTransition(pid_t tid) {
            Transition *&result = id2transition_[tid];

            if (!result) {
                transition_t &transition = const_cast<transition_t &>(pnet_.get_transition(tid));

                transitions_.reserve(transitions_.size() + 1);
                result = new Transition(this, tid, transition);
                transitions_.push_back(result);

                invalidateTransitionsIterators();
            }

            result->ensureValid();
            
            return result;
        }

        void removeTransition(Transition *transition) {
            assert(id2transition_[transition->id()] == transition);

            id2transition_.erase(transition->id());
            invalidateTransitionsIterators();
        }

        Transitions transitions() const {
            return pnetopt::rangeAdaptor(id2transition_ | boost::adaptors::map_values, id2transition_);
        }

        TransitionsIteratorPtr transitionIterator() {
            TransitionsIteratorPtr result(new TransitionsIterator(transitions()));
            transitionIterators_.push_back(result);
            return result;
        }

        void invalidateTransitionsIterators() {
            foreach(TransitionsIteratorPtr &iterator, transitionIterators_) {
                iterator->invalidate();
            }
            transitionIterators_.clear();
        }
    };

    class Port;
    typedef std::vector<Port *> ConnectedPorts;
    typedef pnetopt::LuaIterator<ConnectedPorts> ConnectedPortsIterator;
    typedef RefCountedObjectPtr<ConnectedPortsIterator> ConnectedPortsIteratorPtr;

    class Place: public boost::noncopyable, public pnetopt::Invalidatable {
        /** Parent Petri net. */
        PetriNet *petriNet_;

        /** Place id. */
        pid_t pid_;

        /** Reference to the place. */
        place_t &place_;

        /** Ports to which this place is connected. */
        ConnectedPorts connectedPorts_;

        /** Valid iterators for the connectedPorts_ container. */
        std::vector<ConnectedPortsIteratorPtr> connectedPortsIterators_;

        public:

        Place(PetriNet *petriNet, pid_t pid, place_t &place):
            petriNet_(petriNet), pid_(pid), place_(place)
        {}

        PetriNet *petriNet() const { return petriNet_; }

        pid_t id() const { return pid_; }

        const std::string &name() const {
            ensureValid();
            return place_.name();
        }

        void setName(const std::string &name) {
            ensureValid();
            place_.set_name(name);
        }

        void addConnectedPort(Port *port) {
            connectedPorts_.push_back(port);
            invalidateConnectedPortsIterators();
        }

        void removeConnectedPort(Port *port) {
            connectedPorts_.erase(std::remove(connectedPorts_.begin(), connectedPorts_.end(), port), connectedPorts_.end());
            invalidateConnectedPortsIterators();
        }

        const ConnectedPorts &connectedPorts() const { return connectedPorts_; }

        ConnectedPortsIteratorPtr connectedPortsIterator() {
            ConnectedPortsIteratorPtr result(new ConnectedPortsIterator(connectedPorts()));
            connectedPortsIterators_.push_back(result);
            return result;
        }

        void invalidateConnectedPortsIterators() {
            foreach(ConnectedPortsIteratorPtr &iterator, connectedPortsIterators_) {
                iterator->invalidate();
            }
            connectedPortsIterators_.clear();
        }

        void remove() {
            ensureValid();

            /* Avoid problems with iterators and square complexity. */
            ConnectedPorts portsToDisconnect;
            portsToDisconnect.swap(connectedPorts_);

            foreach (Port *port, portsToDisconnect) {
                assert(port->connectedPlace() == this);
                port->disconnect();
            }

            petriNet()->pnet().delete_place(pid_);
            petriNet()->removePlace(this);

            invalidate();
        }

        void doInvalidate() {
            invalidateConnectedPortsIterators();
        }
    };

    class Port;
    enum PortDirection { INPUT, OUTPUT, TUNNEL };
    typedef boost::unordered_map<std::pair<tid_t, PortDirection>, Port *> IdPortMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdPortMap>, IdPortMap> Ports;
    typedef pnetopt::LuaIterator<Ports> PortsIterator;
    typedef RefCountedObjectPtr<PortsIterator> PortsIteratorPtr;

    class Transition: public boost::noncopyable, public pnetopt::Invalidatable {
        /** Parent Petri net. */
        PetriNet *petriNet_;

        /** Transition id. */
        tid_t tid_;

        /** Reference to the transition. */
        transition_t &transition_;

        /** Map of a port id to a valid port with this id. */
        IdPortMap id2port_;

        /** All ever created ports, both valid and invalid. */
        std::vector<Port *> ports_;

        /** Iterators over the id2port_ container. */
        std::vector<PortsIteratorPtr> portIterators_;

        public:

        Transition(PetriNet *petriNet, tid_t tid, transition_t &transition):
            petriNet_(petriNet), tid_(tid), transition_(transition)
        {
            for (typename transition_t::const_iterator i = transition_.ports_begin(); i != transition_.ports_end(); ++i) {
                port_id_t portId = i->first;
                port_t &port = transition_.get_port(portId);
                if (port.is_input()) {
                    getPort(portId, INPUT);
                }
                if (port.is_output()) {
                    getPort(portId, OUTPUT);
                }
                if (port.is_tunnel()) {
                    getPort(portId, TUNNEL);
                }
            }
            for (typename transition_t::inner_to_outer_t::const_iterator i = transition_.inner_to_outer_begin(); i != transition_.inner_to_outer_end(); ++i) {
                port_id_t portId = i->first;
                pid_t placeId = i->second.first;

                getPort(portId, OUTPUT)->setConnectedPlace(petriNet->getPlace(placeId));
            }
            for (typename transition_t::outer_to_inner_t::const_iterator i = transition_.outer_to_inner_begin(); i != transition_.outer_to_inner_end(); ++i) {
                pid_t placeId = i->first;
                port_id_t portId = i->second.first;

                getPort(portId, INPUT)->setConnectedPlace(petriNet->getPlace(placeId));
            }
        }

        PetriNet *petriNet() const { return petriNet_; }

        tid_t id() const { return tid_; }

        transition_t &transition() const { return transition_; }

        Port *getPort(port_id_t portId, PortDirection direction) {
            Port *&result = id2port_[std::make_pair(portId, direction)];

            if (!result) {
                port_t &port = transition_.get_port(portId);

                ports_.reserve(ports_.size() + 1);
                result = new Port(this, portId, port, direction);
                ports_.push_back(result);
            }

            result->ensureValid();
            
            return result;
        }

        Ports ports() const {
            return pnetopt::rangeAdaptor(id2port_ | boost::adaptors::map_values, id2port_);
        }

        PortsIteratorPtr portIterator() {
            PortsIteratorPtr result(new PortsIterator(ports()));
            portIterators_.push_back(result);
            return result;
        }

        void invalidatePortsIterators() {
            foreach(PortsIteratorPtr &iterator, portIterators_) {
                iterator->invalidate();
            }
            portIterators_.clear();
        }

        const std::string &name() const {
            ensureValid();
            return transition_.name();
        }

        void setName(const std::string &name) {
            ensureValid();
            transition_.set_name(name);
        }

        void remove() {
            ensureValid();

            foreach (Port *port, ports()) {
                port->disconnect();
            }

            petriNet()->pnet().delete_transition(tid_);
            petriNet()->removeTransition(this);

            invalidate();
        }

        void doInvalidate() {
            invalidatePortsIterators();
        }
    };

    class Port: public pnetopt::Invalidatable, boost::noncopyable {
        friend class Transition;

        /** Parent transition. */
        Transition *transition_;

        /** Port id. */
        port_id_t portId_;

        /** Reference to the port. */
        port_t &port_;

        /** Direction of this port. */
        PortDirection direction_;

        /** Place connected to the port. */
        Place *connectedPlace_;

        public:

        Port(Transition *transition, port_id_t portId, port_t &port, PortDirection direction):
            transition_(transition), portId_(portId), port_(port), direction_(direction), connectedPlace_(NULL)
        {
            assert(!(direction == INPUT) || port_.is_input());
            assert(!(direction == OUTPUT) || port_.is_output());
            assert(!(direction == TUNNEL) || port_.is_tunnel());
        }

        Transition *transition() const {
            ensureValid();
            return transition_;
        }

        PetriNet *petriNet() const { return transition()->petriNet(); }

        port_id_t id() const { return portId_; }

        const std::string &name() const {
            ensureValid();
            return port_.name();
        }

        bool isInput() const {
            ensureValid();
            return direction_ == INPUT;
        }

        bool isOutput() const {
            ensureValid();
            return direction_ == OUTPUT;
        }

        bool isTunnel() const {
            ensureValid();
            return direction_ == TUNNEL;
        }

        bool isRead() const {
            ensureValid();
            return port_.direction() == we::type::PORT_READ;
        }

        Place *connectedPlace() const {
            ensureValid();
            return connectedPlace_;
        }

        void setConnectedPlace(Place *place) {
            ensureValid();
            if (place == connectedPlace_) {
                return;
            }
            if (connectedPlace_) {
                connectedPlace_->removeConnectedPort(this);
            }
            connectedPlace_ = place;
            if (connectedPlace_) {
                connectedPlace_->addConnectedPort(this);
            }
        }

        void connect(Place *place) {
            ensureValid();

            if (place == connectedPlace_) {
                return;
            }

            // TODO: I'm not sure how to handle tunnels yet.
            assert(!isTunnel());

            if (connectedPlace_) {
                /* We must remove an edge. */
                if (isInput()) {
                    petriNet()->pnet().delete_edge(petriNet()->pnet().get_eid_in(transition()->id(), connectedPlace_->id()));
                }
                if (isOutput()) {
                    petriNet()->pnet().delete_edge(petriNet()->pnet().get_eid_out(transition()->id(), connectedPlace_->id()));
                }

                /* Now we must disconnect the place on the transition's side. */
                if (isInput()) {
                    transition()->transition().disconnect_outer_from_inner(connectedPlace_->id());
                }
                if (isOutput()) {
                    transition()->transition().disconnect_inner_from_outer(id());
                }
            }

            setConnectedPlace(place);

            if (connectedPlace_) {
                /* We must add an edge. */
                petri_net::edge_type edgeType = isRead() ? petri_net::PT_READ : (isInput() ? petri_net::PT : petri_net::TP);
                petriNet()->pnet().add_edge(100500, petri_net::connection_t(edgeType, transition()->id(), connectedPlace_->id()));

                /* Now we must connect the place on the transition's side. */
                if (isInput()) {
                    transition()->transition().connect_outer_to_inner(connectedPlace_->id(), id(), we::type::property::type());
                }
                if (isOutput()) {
                    transition()->transition().connect_inner_to_outer(id(), connectedPlace_->id(), we::type::property::type());
                }
            }
        }

        void disconnect() {
            ensureValid();
            connect(NULL);
        }
    };

#if 0
    class Places;
    class PlacesIterator;
    class Place;
    class AdjacentPortsIterator;
    class Expression;
    class Condition;

    class Transitions;
    class TransitionsIterator;
    class Transition;

    class Ports;
    class PortsIterator;
    class Port;

    /**
     * A Petri net.
     */
    class PetriNet: public pnetopt::Invalidatable, boost::noncopyable {
        pnet_t &pnet_;
        Places places_;
        Transitions transitions_;

        public:

        PetriNet(pnet_t &pnet): pnet_(pnet), places_(*this), transitions_(*this) {}

        pnet_t &pnet() const {
            return pnet_;
        }

        const char *__tostring() {
            ensureValid();
            return "PetriNet";
        }

        Places &places() {
            return places_;
        }

        Transitions &transitions() {
            return transitions_;
        }

        protected:

        virtual void doInvalidate() {
            places().invalidate();
            transitions().invalidate();
        }
    };

    /**
     * All places of a Petri net.
     */
    class Places: public pnetopt::Invalidatable, boost::noncopyable {
        PetriNet &petriNet_;

        boost::unordered_map<petri_net::pid_t, Place *> pid2place_;
        std::vector<Place *> places_;

        std::vector<RefCountedObjectPtr<PlacesIterator> > iterators_;

        public:

        Places(PetriNet &petriNet): petriNet_(petriNet) {}

        ~Places() {
            foreach (Place *place, places_) {
                delete place;
            }
        }

        PetriNet &petriNet() const {
            return petriNet_;
        }

        const char *__tostring() {
            ensureValid();
            return "Places";
        }

        int __len() {
            ensureValid();
            return petriNet_.pnet().get_num_places();
        }

        RefCountedObjectPtr<PlacesIterator> all() {
            ensureValid();

            RefCountedObjectPtr<PlacesIterator> result(new PlacesIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        Place *getPlace(petri_net::pid_t pid) {
            Place *&result = pid2place_[pid];
            if (!result || !result->valid()) {
                /* Eliminate possibility of vector throwing an exception. */
                places_.reserve(places_.size() + 1);

                // WTF? How to get a non-const pointer to a place?
                result = new Place(*this, pid, const_cast<place_t &>(petriNet().pnet().get_place(pid)));
                places_.push_back(result);
            }
            return result;
        }

        void invalidatePlace(petri_net::pid_t pid) {
            Place *place = places_[pid];
            if (place) {
                place->invalidate();
            }
            places_.erase(pid);
        }

        void invalidatePlaces() {
            typedef typename boost::unordered_map<petri_net::pid_t, Place *>::value_type value_type;
            foreach (const value_type &item, pid2place_) {
                item.second->invalidate();
            }
            pid2place_.clear();
        }

        void invalidateIterators() {
            foreach (const RefCountedObjectPtr<PlacesIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        protected:

        virtual void doInvalidate() {
            invalidatePlaces();
            invalidateIterators();
        }
    };

    /**
     * Iterator over all places of a Petri net.
     */
    class PlacesIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable, boost::noncopyable {
        Places &places_;
        typename pnet_t::place_const_it it_;

        public:

        PlacesIterator(Places &places):
            places_(places),
            it_(places.petriNet().pnet().places())
        {}

        const char *__tostring() {
            ensureValid();

            return "PlacesIterator";
        }

        Place *__call() {
            ensureValid();

            if (it_.has_more()) {
                Place *result = places_.getPlace(*it_);
                ++it_;
                return result;
            } else {
                return NULL;
            }
        };
    };

    /**
     * Place.
     */
    class Place: public RefCountedObjectType<int>, public pnetopt::Invalidatable, boost::noncopyable {
        Places &places_;
        petri_net::pid_t pid_;
        place_t &place_;

        std::vector<RefCountedObjectPtr<AdjacentPortsIterator> > iterators_;

        public:

        Place(Places &places, typename petri_net::pid_t pid, place_t &place):
            places_(places), pid_(pid), place_(place)
        {}

        Places &places() const { return places_; }

        bool __eq(Place *place) const {
            return this == place;
        }

        petri_net::pid_t id() {
            ensureValid();

            return pid_;
        }

        const std::string &name() {
            ensureValid();

            return place_.name();
        }

        void setName(const std::string &name) {
            ensureValid();

            place_.set_name(name);
        }

        RefCountedObjectPtr<AdjacentPortsIterator> in_connections() {
            ensureValid();

            RefCountedObjectPtr<AdjacentPortsIterator> result(
                new AdjacentPortsIterator(*this, places_.petriNet().pnet().in_to_place(pid_), true));
            iterators_.push_back(result);

            return result;
        }

        RefCountedObjectPtr<AdjacentPortsIterator> out_connections() {
            ensureValid();

            RefCountedObjectPtr<AdjacentPortsIterator> result(
                new AdjacentPortsIterator(*this, places_.petriNet().pnet().out_of_place(pid_), false));
            iterators_.push_back(result);

            return result;
        }

        void remove() {
            ensureValid();

            // Disconnect the place from all the ports it is connected to.
            //
            // delete_place() won't do it properly.
            
            std::vector<Port *> portsToDisconnect;
            
            RefCountedObjectPtr<AdjacentPortsIterator> i = in_connections();
            while (Port *port = i->__call()) {
                portsToDisconnect.push_back(port);
            }

            i = out_connections();
            while (Port *port = i->__call()) {
                portsToDisconnect.push_back(port);
            }

            foreach (Port *port, portsToDisconnect) {
                port->disconnect();
            }

            places().petriNet().pnet().delete_place(pid_);

            invalidate();
            places().invalidateIterators();
        }

        void invalidateIterators() {
            foreach (const RefCountedObjectPtr<AdjacentPortsIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        protected:

        virtual void doInvalidate() {
            invalidateIterators();
        }
    };


    /**
     * Iterator over ports that are connected to a place.
     */
    class AdjacentPortsIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable, boost::noncopyable {
        Place &place_;
        typename petri_net::adj_transition_const_it it_;
        bool lookInOutputPorts_;

        public:

        AdjacentPortsIterator(Place &place, const petri_net::adj_transition_const_it &it, bool lookInOutputPorts):
            place_(place),
            it_(it),
            lookInOutputPorts_(lookInOutputPorts)
        {}

        const char *__tostring() {
            ensureValid();

            return "AdjacentPortsIterator";
        }

        Port *__call() {
            ensureValid();

            if (it_.has_more()) {
                Transition *transition = place_.places().petriNet().transitions().getTransition(*it_);

                typename transition_t::port_id_t portId;
                if (lookInOutputPorts_) {
                    portId = transition->transition().output_port_by_pid(place_.id()).first;
                } else {
                    portId = transition->transition().input_port_by_pid(place_.id()).first;
                }

                Port *result = transition->ports()->getPort(portId);
                ++it_;
                return result;
            } else {
                return NULL;
            }
        };
    };

    /**
     * All transitions of a Petri net.
     */
    class Transitions: public pnetopt::Invalidatable, boost::noncopyable {
        PetriNet &petriNet_;

        boost::unordered_map<petri_net::tid_t, Transition *> tid2transition_;
        std::vector<Transition *> transitions_;

        std::vector<RefCountedObjectPtr<TransitionsIterator> > iterators_;

        public:

        Transitions(PetriNet &petriNet): petriNet_(petriNet) {}

        ~Transitions() {
            foreach (Transition *transition, transitions_) {
                delete transition;
            }
        }

        PetriNet &petriNet() const {
            return petriNet_;
        }

        const char *__tostring() {
            return "Transitions";
        }

        int __len() const {
            return petriNet_.pnet().get_num_transitions();
        }

        RefCountedObjectPtr<TransitionsIterator> all() {
            RefCountedObjectPtr<TransitionsIterator> result(new TransitionsIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        Transition *getTransition(petri_net::tid_t tid) {
            Transition *&result = tid2transition_[tid];
            if (!result || !result->valid()) {
                transitions_.reserve(transitions_.size() + 1);

                // WTF? I can't get a non-const reference to a transition.
                result = new Transition(*this, tid, const_cast<transition_t &>(petriNet().pnet().get_transition(tid)));
                transitions_.push_back(result);
            }
            return result;
        }

        void invalidateIterators() {
            foreach (const RefCountedObjectPtr<TransitionsIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        void invalidateTransition(petri_net::tid_t tid) {
            Transition *result = tid2transition_[tid];
            if (result) {
                result->invalidate();
            }
            tid2transition_.erase(tid);
        }
    };

    /**
     * Iterator over all transitions of a Petri net.
     */
    class TransitionsIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable, boost::noncopyable {
        Transitions &transitions_;
        typename pnet_t::transition_const_it it_;

        public:

        TransitionsIterator(Transitions &transitions):
            transitions_(transitions),
            it_(transitions.petriNet().pnet().transitions())
        {}

        const char *__tostring() {
            ensureValid();

            return "TransitionsIterator";
        }

        Transition *__call() {
            ensureValid();

            if (it_.has_more()) {
                Transition *result = transitions_.getTransition(*it_);
                ++it_;
                return result;
            } else {
                return NULL;
            }
        };
    };

    /**
     * Transition.
     */
    class Transition: public pnetopt::Invalidatable, boost::noncopyable {
        Transitions &transitions_;
        petri_net::tid_t tid_;
        transition_t &transition_;
        std::auto_ptr<Ports> ports_;
        std::auto_ptr<PetriNet> subnet_;
        std::auto_ptr<Expression> expression_;
        std::auto_ptr<Condition> condition_;

        public:

        Transition(Transitions &transitions, typename petri_net::tid_t tid, transition_t &transition):
            transitions_(transitions),
            tid_(tid),
            transition_(transition),
            ports_(NULL)
        {}

        Transitions &transitions() const { return transitions_; }

        transition_t &transition() const { return transition_; }

        const std::string &name() {
            ensureValid();

            return transition_.name();
        }

        void setName(const std::string &name) {
            ensureValid();

            transition_.set_name(name);
        }

        petri_net::tid_t id() const { return tid_; }

        Ports *ports() {
            ensureValid();

            if (!ports_.get()) {
                ports_.reset(new Ports(*this));
            }
            return ports_.get();
        }

        struct SubnetReturner: public boost::static_visitor<pnet_t *> {
            pnet_t *operator()(we::type::expression_t &expr) const { return NULL; }
            pnet_t *operator()(we::type::module_call_t &mod_call) const { return NULL; }
            pnet_t *operator()(pnet_t &net) const { return &net; } 
        };

        PetriNet *subnet() {
            ensureValid();

            if (!subnet_.get()) {
                if (pnet_t *pnet = boost::apply_visitor(SubnetReturner(), transition().data())) {
                    subnet_.reset(new PetriNet(*pnet));
                }
            }
            return subnet_.get();
        }

        struct ExpressionReturner: public boost::static_visitor<we::type::expression_t *> {
            we::type::expression_t *operator()(we::type::expression_t &expr) const { return &expr; }
            we::type::expression_t *operator()(we::type::module_call_t &mod_call) const { return NULL; }
            we::type::expression_t *operator()(pnet_t &net) const { return NULL; } 
        };

        Expression *expression() {
            ensureValid();

            if (!expression_.get()) {
                if (we::type::expression_t *expr = boost::apply_visitor(ExpressionReturner(), transition().data())) {
                    expression_.reset(new Expression(*expr));
                }
            }
            return expression_.get();
        }

        Condition *condition() {
            ensureValid();

            if (!condition_.get()) {
                condition_.reset(new Condition(transition().condition()));
            }
            return condition_.get();
        }

        void remove() {
            ensureValid();

            transitions().petriNet().pnet().delete_transition(tid_);

            invalidate();
            transitions().invalidateIterators();
        }

        protected:

        void doInvalidate() {
            if (ports_.get()) {
                ports_->invalidate();
            }
            if (subnet_.get()) {
                subnet_->invalidate();
            }
            if (expression_.get()) {
                expression_->invalidate();
            }
            if (condition_.get()) {
                condition_->invalidate();
            }
        }
    };

    class Expression: public pnetopt::Invalidatable, boost::noncopyable {
        we::type::expression_t &expression_;

        public:

        Expression(we::type::expression_t &expression):
            expression_(expression)
        {}

        std::string __tostring() {
            ensureValid();
            return expression_.expression();
        }

        bool isEmpty() {
            ensureValid();
            return expression_.is_empty();
        }
    };

    class Condition: public pnetopt::Invalidatable, boost::noncopyable {
        const condition::type &condition_;

        public:

        Condition(const condition::type &condition):
            condition_(condition)
        {}

        std::string __tostring() {
            ensureValid();
            return condition_.expression();
        }

        bool isConstTrue() {
            ensureValid();
            return condition_.is_const_true();
        }
    };

    /**
     * Ports of a transition.
     */
    class Ports: public pnetopt::Invalidatable, boost::noncopyable {
        Transition &transition_;
        std::vector<RefCountedObjectPtr<PortsIterator> > iterators_;

        boost::unordered_map<typename transition_t::port_id_t, Port *> portId2port_;
        std::vector<Port *> ports_;

        public:

        Ports(Transition &transition): transition_(transition) {}

        ~Ports() {
            foreach (Port *port, ports_) {
                delete port;
            }
        }

        Transition &transition() const { return transition_; }

        const char *__tostring() {
            ensureValid();

            return "Ports";
        }

        int __len() const {
            return transition_.transition().ports().size();
        }

        RefCountedObjectPtr<PortsIterator> all() {
            ensureValid();

            RefCountedObjectPtr<PortsIterator> result(new PortsIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        Port *getPort(typename transition_t::port_id_t portId) {
            Port *&result = portId2port_[portId];
            if (!result || !result->valid()) {
                ports_.reserve(ports_.size() + 1);
                result = new Port(*this, portId, transition().transition().get_port(portId));
                ports_.push_back(result);
            }
            return result;
        }

        protected:

        virtual void doEnsureValid() {
            if (!transition_.valid()) {
                invalidate();
            }
        }

        virtual void doInvalidate() {
            foreach (const RefCountedObjectPtr<PortsIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }
    };

    /**
     * Iterator over all ports of a transition.
     */
    class PortsIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable, boost::noncopyable {
        Ports &ports_;
        typename transition_t::const_iterator it_;
        typename transition_t::const_iterator end_;

        public:
        
        PortsIterator(Ports &ports):
            ports_(ports),
            it_(ports_.transition().transition().ports_begin()),
            end_(ports_.transition().transition().ports_end())
        {}

        const char *__tostring() {
            ensureValid();

            return "PortsIterator";
        }

        Port *__call() {
            ensureValid();

            if (it_ != end_) {
                Port *result = ports_.getPort(it_->first);
                ++it_;
                return result;
            } else {
                return NULL;
            }
        };
    };

    /**
     * Port of a transition.
     */
    class Port: public pnetopt::Invalidatable, boost::noncopyable {
        Ports &ports_;
        typename transition_t::port_id_t portId_;
        typename transition_t::port_t port_;

        public:

        Port(Ports &ports, typename transition_t::port_id_t portId, typename transition_t::port_t port):
            ports_(ports), portId_(portId), port_(port)
        {}

        /**
         * \return Port name.
         */
        const std::string &name() {
            ensureValid();

            return port_.name();
        }

        /**
         * \return Place connected to this port, or nil if there is no such place.
         */
        Place *place() {
            ensureValid();

            try {
                if (isInput()) {
                    return ports_.transition().transitions().petriNet().places().getPlace(ports_.transition().transition().gen_outer_to_inner(portId_).first);
                }
                if (isOutput()) {
                    return ports_.transition().transitions().petriNet().places().getPlace(ports_.transition().transition().gen_inner_to_outer(portId_).first);
                }
            } catch (const we::type::exception::not_connected<typename transition_t::port_id_t> &e) {
                // WTF? Why can't I check connectedness without using exceptions?
            }
            return NULL;
        }

        /**
         * \return Place from the subnet of the transition which is associated with this port.
         */
        Place *associatedPlace() {
            ensureValid();

            if (port_.has_associated_place()) {
                PetriNet *subnet = ports_.transition().subnet();
                assert(subnet != NULL);

                return subnet->places().getPlace(port_.associated_place());
            } else {
                return NULL;
            }
        }

        /**
         * \return Transition this port belongs to.
         */
        Transition &transition() {
            ensureValid();
            return ports_.transition();
        }

        bool isInput() {
            ensureValid();
            return port_.is_input();
        }

        bool isOutput() {
            ensureValid();
            return port_.is_output();
        }

        bool isTunnel() {
            ensureValid();
            return port_.is_tunnel();
        }

        /**
         * Disconnects the port from the place.
         */
        void disconnect() {
            ensureValid();

            if (Place *place = this->place()) {
                // Presumably, we should remove an edge between the place and the transition.
                if (isInput()) {
                    ports_.transition().transitions().petriNet().pnet().delete_edge(
                        ports_.transition().transitions().petriNet().pnet().get_eid_in(ports_.transition().id(), place->id()));
                }
                if (isOutput()) {
                    ports_.transition().transitions().petriNet().pnet().delete_edge(
                        ports_.transition().transitions().petriNet().pnet().get_eid_out(ports_.transition().id(), place->id()));
                }

                // Now we must disconnect the place on the transition's side.
                ports_.transition().transition().disconnect_inner_from_outer(portId_);

                // And don't forget to invalidate adjacency iterators of the disconnected place.
                place->invalidateIterators();

                // TODO: That's all?
            }
        }
    };
#endif
};

class Visitor: public boost::static_visitor<void> {
    std::string script_;

    public:

    Visitor(const std::string &script): script_(script) {}

    const std::string &script() const { return script_; }
    void operator()(we::type::expression_t & expr) { return; }
    void operator()(we::type::module_call_t & mod_call) { return; }

    template<class P, class E, class T>
    void operator()(petri_net::net<P, we::type::transition_t<P, E, T>, E, T> &net) {
        Optimizer<P, E, T> optimizer(net);
        optimizer(script().c_str());
    }

    template<class P, class E, class T>
    void operator()(we::type::transition_t<P, E, T> &transition) {
        boost::apply_visitor(*this, transition.data());
    }
};

} // anonymous namespace

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string input("-");
    std::string output("-");
    std::string script("main.lua");
    bool xml = false;

    po::options_description options("options");

    options.add_options()
        ("help,h", "this message")
        ("input,i"
        , po::value<std::string>(&input)->default_value(input)
        , "input file name, - for stdin"
        )
        ("output,o"
        , po::value<std::string>(&output)->default_value(output)
        , "output file name, - for stdout"
        )
        ("script,s"
        , po::value<std::string>(&script)->default_value(script)
        , "script file name"
        )
        ( "xml,x"
        , po::bool_switch(&xml)->default_value(xml)
        , "write xml instead of text format"
        )
        ;

    po::positional_options_description positional;
    positional.add("input", -1);

    po::variables_map vm;

    try {
        po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), vm);
        po::notify(vm);
    } catch (const std::exception &e) {
        std::cerr << "invalid argument: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help")) {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    we::activity_t activity;

    if (input == "-") {
        we::util::text_codec::decode(std::cin, activity);
    } else {
        std::ifstream in(input.c_str());
        if (!in) {
            std::cerr << "failed to open " << input << "for reading" << std::endl;
            return EXIT_FAILURE;
        }
        we::util::text_codec::decode(in, activity);
    }

    try {
        Visitor visitor(script);
        visitor(activity.transition());
    } catch (const std::exception &e) {
        std::cerr << "pnetopt: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (output == "-") {
        std::cout << (xml ? we::util::xml_codec::encode(activity) : we::util::text_codec::encode(activity));
    } else {
        std::ofstream out(output.c_str());
        if (!out) {
            std::cerr << "failed to open " << input << "for writing" << std::endl;
            return EXIT_FAILURE;
        }
        out << (xml ? we::util::xml_codec::encode(activity) : we::util::text_codec::encode(activity));
    }

    return 0;
}

/* vim:set et sts=4 sw=4: */
