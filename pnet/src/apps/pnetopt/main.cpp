#include <algorithm>
#include <cstdlib> /* random() */
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

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/port.hpp>
#include <we/mgmt/type/activity.hpp>

#define foreach BOOST_FOREACH

namespace {

/*
 * Everything is C++-owned and is never deleted.
 * Except iterators: nobody refers them anyway, so we can apply reference counting to them.
 */

class Optimizer {
    typedef we::type::transition_t transition_t;

    typedef petri_net::net pnet_t;

    typedef we::type::port_t port_t;

    lua_State *L;

    class Transition;
    std::auto_ptr<Transition> transition_;

    public:

    Optimizer(transition_t &transition):
        transition_(new Transition(NULL, -1, transition))
    {
        L = lua_open();
        luaL_openlibs(L);

        // TODO: why without reregistering __eq each time it doesn't work?

        luabridge::getGlobalNamespace(L)
            .beginNamespace("pnetopt")
                .beginClass<pnetopt::Invalidatable>("Invalidatable")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("valid", &pnetopt::Invalidatable::valid)
                .endClass()
                .deriveClass<PetriNet, pnetopt::Invalidatable>("PetriNet")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &PetriNet::toString)
                    .addFunction("places", &PetriNet::placesIterator)
                    .addFunction("transitions", &PetriNet::transitionIterator)
                .endClass()
                .deriveClass<Place, pnetopt::Invalidatable>("Place")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &Place::name)
                    .addFunction("name", &Place::name)
                    .addFunction("setName", &Place::setName)
                    .addFunction("connectedPorts", &Place::connectedPortsIterator)
                    .addFunction("associatedPorts", &Place::associatedPortsIterator)
                    .addFunction("remove", &Place::remove)
                .endClass()
                .deriveClass<PlacesIterator, pnetopt::Invalidatable>("PlacesIterator")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &PlacesIterator::toString)
                    .addFunction("__call", &PlacesIterator::call)
                    .addFunction("__len", &PlacesIterator::size)
                .endClass()
                .deriveClass<ConnectedPortsIterator, pnetopt::Invalidatable>("ConnectedPortsIterator")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &ConnectedPortsIterator::toString)
                    .addFunction("__call", &ConnectedPortsIterator::call)
                    .addFunction("__len", &ConnectedPortsIterator::size)
                .endClass()
                .deriveClass<Expression, pnetopt::Invalidatable>("Expression")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &Expression::toString)
                    .addFunction("isEmpty", &Expression::isEmpty)
                .endClass()
                .deriveClass<Condition, pnetopt::Invalidatable>("Condition")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &Condition::toString)
                    .addFunction("isConstTrue", &Condition::isConstTrue)
                .endClass()
                .deriveClass<Transition, pnetopt::Invalidatable>("Transition")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &Transition::name)
                    .addFunction("name", &Transition::name)
                    .addFunction("setName", &Transition::setName)
                    .addFunction("ports", &Transition::portIterator)
                    .addFunction("subnet", &Transition::subnet)
                    .addFunction("expression", &Transition::expression)
                    .addFunction("condition", &Transition::condition)
                    .addFunction("remove", &Transition::remove)
                .endClass()
                .deriveClass<TransitionsIterator, pnetopt::Invalidatable>("TransitionsIterator")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &TransitionsIterator::toString)
                    .addFunction("__call", &TransitionsIterator::call)
                    .addFunction("__len", &TransitionsIterator::size)
                .endClass()
                .deriveClass<Port, pnetopt::Invalidatable>("Port")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &Port::name)
                    .addFunction("transition", &Port::transition)
                    .addFunction("name", &Port::name)
                    .addFunction("isInput", &Port::isInput)
                    .addFunction("isOutput", &Port::isOutput)
                    .addFunction("isTunnel", &Port::isTunnel)
                    .addFunction("connectedPlace", &Port::connectedPlace)
                    .addFunction("connect", &Port::connect)
                    .addFunction("disconnect", &Port::disconnect)
                    .addFunction("associatedPlace", &Port::associatedPlace)
                    .addFunction("associate", &Port::associate)
                    .addFunction("unassociate", &Port::unassociate)
                .endClass()
                .deriveClass<PortsIterator, pnetopt::Invalidatable>("PortsIterator")
                    .addFunction("__eq", &pnetopt::Invalidatable::equals)
                    .addFunction("__tostring", &PortsIterator::toString)
                    .addFunction("__call", &PortsIterator::call)
                    .addFunction("__len", &PortsIterator::size)
                .endClass()
        ;
        luabridge::push(L, transition_.get());
        lua_setfield(L, LUA_GLOBALSINDEX, "transition");
    }

    ~Optimizer() {
        lua_close(L);
    }

    void operator()(const char *filename) {
        if (luaL_dofile(L, filename) != 0) {
            std::runtime_error e((boost::format("%1% (Lua error)") % lua_tostring(L, -1)).str());
            lua_pop(L, 1);
            throw e;
        }
    }

    private:

    class Place;
  typedef boost::unordered_map<petri_net::place_id_type, Place *> IdPlaceMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdPlaceMap>, IdPlaceMap> Places;
    typedef pnetopt::LuaIterator<Places> PlacesIterator;
    typedef RefCountedObjectPtr<PlacesIterator> PlacesIteratorPtr;

    class Transition;
  typedef boost::unordered_map<petri_net::transition_id_type, Transition *> IdTransitionMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdTransitionMap>, IdTransitionMap> Transitions;
    typedef pnetopt::LuaIterator<Transitions> TransitionsIterator;
    typedef RefCountedObjectPtr<TransitionsIterator> TransitionsIteratorPtr;

    class PetriNet: public pnetopt::Invalidatable, boost::noncopyable {
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
          foreach ( const petri_net::place_id_type& pid
                  , pnet_.places() | boost::adaptors::map_keys
                  )
          {
            getPlace(pid);
          }
            foreach ( const petri_net::transition_id_type& tid
                    , pnet_.transitions() | boost::adaptors::map_keys
                    )
            {
                getTransition(tid);
            }
        }

        pnet_t &pnet() const { return pnet_; }

        ~PetriNet() {
            foreach(Place *place, places_) {
                delete place;
            }
        }

        const char *toString() const {
            ensureValid();

            return "PetriNet";
        }

      Place *getPlace(petri_net::place_id_type pid) {
            Place *&result = id2place_[pid];

            if (!result) {
              place::type &place = const_cast<place::type &>(pnet_.get_place(pid));

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
            ensureValid();

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

      Transition *getTransition(petri_net::transition_id_type tid) {
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
            ensureValid();

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

        void doInvalidate() {
            foreach (Place *place, places()) {
                place->invalidate();
            }
            foreach (Transition *transition, transitions()) {
                transition->invalidate();
            }
            invalidatePlacesIterators();
            invalidateTransitionsIterators();
        }
    };

    class Port;
    typedef std::vector<Port *> ConnectedPorts;
    typedef pnetopt::LuaIterator<ConnectedPorts> ConnectedPortsIterator;
    typedef RefCountedObjectPtr<ConnectedPortsIterator> ConnectedPortsIteratorPtr;

    typedef std::vector<Port *> AssociatedPorts;
    typedef pnetopt::LuaIterator<AssociatedPorts> AssociatedPortsIterator;
    typedef RefCountedObjectPtr<AssociatedPortsIterator> AssociatedPortsIteratorPtr;

    class Place: public pnetopt::Invalidatable, boost::noncopyable {
        /** Parent Petri net. */
        PetriNet *petriNet_;

        /** Place id. */
      petri_net::place_id_type pid_;

        /** Reference to the place. */
        place::type &place_;

        /** Ports to which this place is connected. */
        ConnectedPorts connectedPorts_;

        /** Valid iterators over the connectedPorts_ container. */
        std::vector<ConnectedPortsIteratorPtr> connectedPortsIterators_;

        /** Ports with which this place is associated. */
        AssociatedPorts associatedPorts_;

        /** Valid iterators over the associatedPorts_ container. */
        std::vector<ConnectedPortsIteratorPtr> associatedPortsIterators_;

        public:

      Place(PetriNet *petriNet, petri_net::place_id_type pid, place::type &place):
            petriNet_(petriNet), pid_(pid), place_(place)
        {}

        PetriNet *petriNet() const { return petriNet_; }

      petri_net::place_id_type id() const { return pid_; }

        const std::string &name() const {
            ensureValid();
            return place_.name();
        }

        void setName(const std::string &name) {
            ensureValid();
            place_.name(name);
        }

        const ConnectedPorts &connectedPorts() const { return connectedPorts_; }

        void addConnectedPort(Port *port) {
            connectedPorts_.push_back(port);
            invalidateConnectedPortsIterators();
        }

        void removeConnectedPort(Port *port) {
            connectedPorts_.erase(std::remove(connectedPorts_.begin(), connectedPorts_.end(), port), connectedPorts_.end());
            invalidateConnectedPortsIterators();
        }

        ConnectedPortsIteratorPtr connectedPortsIterator() {
            ensureValid();

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

        const AssociatedPorts &associatedPorts() const { return associatedPorts_; }

        void addAssociatedPort(Port *port) {
            associatedPorts_.push_back(port);
            invalidateAssociatedPortsIterators();
        }

        void removeAssociatedPort(Port *port) {
            associatedPorts_.erase(std::remove(associatedPorts_.begin(), associatedPorts_.end(), port), associatedPorts_.end());
            invalidateAssociatedPortsIterators();
        }

        AssociatedPortsIteratorPtr associatedPortsIterator() {
            ensureValid();

            AssociatedPortsIteratorPtr result(new AssociatedPortsIterator(associatedPorts()));
            associatedPortsIterators_.push_back(result);
            return result;
        }

        void invalidateAssociatedPortsIterators() {
            foreach(AssociatedPortsIteratorPtr &iterator, associatedPortsIterators_) {
                iterator->invalidate();
            }
            associatedPortsIterators_.clear();
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

            AssociatedPorts portsToDeassociate;
            portsToDeassociate.swap(associatedPorts_);

            foreach (Port *port, portsToDeassociate) {
                assert(port->associatedPlace() == this);
                port->unassociate();
            }

            petriNet()->pnet().delete_place(pid_);
            petriNet()->removePlace(this);

            invalidate();
        }

        void doInvalidate() {
            invalidateConnectedPortsIterators();
            invalidateAssociatedPortsIterators();
        }
    };

    class Expression: public pnetopt::Invalidatable, boost::noncopyable {
        we::type::expression_t &expression_;

        public:

        Expression(we::type::expression_t &expression):
            expression_(expression)
        {}

        const std::string &toString() const {
            ensureValid();
            return expression_.expression();
        }

        bool isEmpty() const {
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

        const std::string &toString() const {
            ensureValid();
            return condition_.expression();
        }

        bool isConstTrue() const {
            ensureValid();
            return condition_.is_const_true();
        }
    };

    class Port;
    enum PortDirection { INPUT, OUTPUT, TUNNEL };
    typedef boost::unordered_map<std::pair<petri_net::port_id_type, PortDirection>, Port *> IdPortMap;
    typedef pnetopt::RangeAdaptor<boost::select_second_const_range<IdPortMap>, IdPortMap> Ports;
    typedef pnetopt::LuaIterator<Ports> PortsIterator;
    typedef RefCountedObjectPtr<PortsIterator> PortsIteratorPtr;

    class Transition: public pnetopt::Invalidatable, boost::noncopyable {
        /** Parent Petri net. */
        PetriNet *petriNet_;

        /** Transition id. */
      petri_net::transition_id_type tid_;

        /** Reference to the transition. */
        transition_t &transition_;

        /** Map of a port id to a valid port with this id. */
        IdPortMap id2port_;

        /** All ever created ports, both valid and invalid. */
        std::vector<Port *> ports_;

        /** Iterators over the id2port_ container. */
        std::vector<PortsIteratorPtr> portIterators_;

        /** Pointer to the subnet, if any. */
        std::auto_ptr<PetriNet> subnet_;

        /** Pointer to the expression, if any. */
        std::auto_ptr<Expression> expression_;

        /** Condition. */
        std::auto_ptr<Condition> condition_;

        public:

      Transition(PetriNet *petriNet, petri_net::transition_id_type tid, transition_t &transition):
            petriNet_(petriNet), tid_(tid), transition_(transition)
        {
            for (transition_t::const_iterator i = transition_.ports_begin(); i != transition_.ports_end(); ++i) {
                petri_net::port_id_type portId = i->first;
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
            for (transition_t::inner_to_outer_t::const_iterator i = transition_.inner_to_outer_begin(); i != transition_.inner_to_outer_end(); ++i) {
                petri_net::port_id_type portId = i->first;
                petri_net::place_id_type placeId = i->second.first;

                /* Top-level transition cannot have connections. */
                assert(petriNet != NULL);

                getPort(portId, OUTPUT)->setConnectedPlace(petriNet->getPlace(placeId));
            }
            for (transition_t::outer_to_inner_t::const_iterator i = transition_.outer_to_inner_begin(); i != transition_.outer_to_inner_end(); ++i) {
              petri_net::place_id_type placeId = i->first;
                petri_net::port_id_type portId = i->second.first;

                /* Top-level transition cannot have connections. */
                assert(petriNet != NULL);

                getPort(portId, INPUT)->setConnectedPlace(petriNet->getPlace(placeId));
            }
        }

        PetriNet *petriNet() const { return petriNet_; }

      petri_net::transition_id_type id() const { return tid_; }

        transition_t &transition() const { return transition_; }

        Port *getPort(petri_net::port_id_type portId, PortDirection direction) {
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
            ensureValid();

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

                    /* Find associated places and add backward references to the respective ports. */
                    foreach (Port *port, ports()) {
                        port->associatedPlace();
                    }
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

            if (petriNet() == NULL) {
                throw std::runtime_error("cannot remove a top-level transition");
            }

            foreach (Port *port, ports()) {
                port->disconnect();
            }

            petriNet()->pnet().delete_transition(tid_);
            petriNet()->removeTransition(this);

            invalidate();
        }

        void doInvalidate() {
            invalidatePortsIterators();

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

    class Port: public pnetopt::Invalidatable, boost::noncopyable {
        friend class Transition;

        /** Parent transition. */
        Transition *transition_;

        /** Port id. */
        petri_net::port_id_type portId_;

        /** Reference to the port. */
        port_t &port_;

        /** Direction of this port. */
        PortDirection direction_;

        /** Place connected to the port. */
        Place *connectedPlace_;

        /** Place (in the subnet) associated with this port. */
        Place *associatedPlace_;

        public:

        Port(Transition *transition, petri_net::port_id_type portId, port_t &port, PortDirection direction):
            transition_(transition), portId_(portId), port_(port), direction_(direction),
            connectedPlace_(NULL), associatedPlace_(NULL)
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

        petri_net::port_id_type id() const { return portId_; }

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

        Place *connectedPlace() const {
            ensureValid();
            return connectedPlace_;
        }

        void setConnectedPlace(Place *place) {
            ensureValid();
            if (place == connectedPlace_) {
                return;
            }
            assert(place == NULL || place->petriNet() == petriNet());
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

            if (place != NULL && place->petriNet() != petriNet()) {
                throw std::runtime_error((boost::format("trying to connect a port %1% to a place %2% from a different Petri net") % name() % place->name()).str());
            }

            if (connectedPlace_) {
                /* We must remove an edge. */
                if (isInput()) {
                    petriNet()->pnet().delete_edge_in(transition()->id(), connectedPlace_->id());
                }
                if (isOutput()) {
                    petriNet()->pnet().delete_edge_out(transition()->id(), connectedPlace_->id());
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
              //! \todo How to get an read connection?
                petri_net::edge::type edgeType = isInput() ? petri_net::edge::PT : petri_net::edge::TP;

                bool tryAgain;
                do {
                    tryAgain = false;
                    try {
                        petriNet()->pnet().add_connection
                          (petri_net::connection_t ( edgeType
                                                   , transition()->id()
                                                   , connectedPlace_->id()
                                                   )
                          );
                    } catch (const we::container::exception::already_there&) {
                        tryAgain = true;
                    }
                } while (tryAgain);

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

        Place *associatedPlace() {
            ensureValid();

            if (!associatedPlace_ && port_.has_associated_place()) {
                assert(transition()->subnet());
                setAssociatedPlace(transition()->subnet()->getPlace(port_.associated_place()));
            }

            return associatedPlace_;
        }

        void setAssociatedPlace(Place *place) {
            ensureValid();

            if (place == associatedPlace_) {
                return;
            }
            assert(place == NULL || place->petriNet() == transition()->subnet());
            if (associatedPlace_) {
                associatedPlace_->removeAssociatedPort(this);
            }
            associatedPlace_ = place;
            if (associatedPlace_) {
                associatedPlace_->addAssociatedPort(this);
            }
        }

        void associate(Place *place) {
            ensureValid();

            /* Note: it is important to call associatedPlace() here. */
            if (place == associatedPlace()) {
                return;
            }

            if (place != NULL && place->petriNet() != transition()->subnet()) {
                throw std::runtime_error((boost::format("trying to associate a port %1% with a place %2% not from a direct subnet") % name() % place->name()).str());
            }

            setAssociatedPlace(place);

            if (place) {
                port_.associated_place() = place->id();
            } else {
                port_.associated_place() = petri_net::place_id_invalid();
            }
        }

        void unassociate() {
            ensureValid();
            associate(NULL);
        }
    };
};

} // anonymous namespace

void do_optimize(we::type::transition_t &transition, const char *script) {
    Optimizer optimizer(transition);
    optimizer(script);
}

int main(int argc, char **argv)
try
{
  namespace po = boost::program_options;

  std::string input("-");
  std::string output("-");
  std::string script("main.lua");

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

  we::mgmt::type::activity_t activity
    ( input == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (input))
    );

  do_optimize(activity.transition(), script.c_str());

  if (output == "-") {
    std::cout << activity.to_string();
  } else {
    std::ofstream out(output.c_str());
    if (!out) {
      std::cerr << "failed to open " << input << " for writing" << std::endl;
      return EXIT_FAILURE;
    }
    out << activity.to_string();
  }

  return 0;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}

/* vim:set et sts=4 sw=4: */
