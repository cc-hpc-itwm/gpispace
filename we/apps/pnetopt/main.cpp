#include <iostream>
#include <fstream>
#include <memory> /* std::auto_ptr */

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"

#include "Invalidatable.h"

#include <we/we.hpp>

#define foreach BOOST_FOREACH

namespace {

/*
 * Everything is C++ owned and is never deleted.
 * Except iterators: nobody refers them anyway, so we can garbage-collect them.
 *
 * Another option: everything is Lua-owned and deletes references to itself on destruction.
 * Doesn't work. For example, a Ports has a reference to Transition which is not used
 * anymore in Lua. But the Transition can't be deleted, since it will invalidate Ports
 * object (it can't access anything anymore).
 *
 */

template<class P, class E, class T>
class Optimizer {
    class PetriNet;

    lua_State *L;
    PetriNet petriNet_;

    public:

    typedef P place_t;
    typedef we::type::transition_t<P, E, T> transition_t;
    typedef petri_net::net<P, transition_t, E, T> pnet_t;

    public:

    Optimizer(pnet_t &pnet):
        petriNet_(pnet)
    {
        L = lua_open();
        luaL_openlibs(L);

        luabridge::getGlobalNamespace(L)
            .beginNamespace("pnetopt")
                .beginClass<PetriNet>("PetriNet")
                    .addFunction("__tostring", &PetriNet::__tostring)
                    .addFunction("places", &PetriNet::places)
                    .addFunction("transitions", &PetriNet::transitions)
                .endClass()
                .template beginClass<Places>("Places")
                    .addFunction("__tostring", &Places::__tostring)
                    .addFunction("__len", &Places::__len)
                    .addFunction("all", &Places::all)
                .endClass()
                .template beginClass<PlaceIterator>("PlaceIterator")
                    .addFunction("__tostring", &PlaceIterator::__tostring)
                    .addFunction("__call", &PlaceIterator::__call)
                .endClass()
                .template beginClass<Place>("Place")
                    .addFunction("__tostring", &Place::name)
                    .addFunction("id", &Place::id)
                    .addFunction("name", &Place::name)
                    .addFunction("in_connections", &Place::in_connections)
                    .addFunction("out_connections", &Place::out_connections)
                .endClass()
                .template beginClass<AdjacentPortIterator>("AdjacentPortIterator")
                    .addFunction("__tostring", &AdjacentPortIterator::__tostring)
                    .addFunction("__call", &AdjacentPortIterator::__call)
                .endClass()
                .template beginClass<Transitions>("Transitions")
                    .addFunction("__tostring", &Transitions::__tostring)
                    .addFunction("__len", &Transitions::__len)
                    .addFunction("all", &Transitions::all)
                .endClass()
                .template beginClass<TransitionIterator>("TransitionIterator")
                    .addFunction("__tostring", &TransitionIterator::__tostring)
                    .addFunction("__call", &TransitionIterator::__call)
                .endClass()
                .template beginClass<Transition>("Transition")
                    .addFunction("__tostring", &Transition::name)
                    .addFunction("name", &Transition::name)
                    .addFunction("ports", &Transition::ports)
                    .addFunction("subnet", &Transition::subnet)
                .endClass()
                .template beginClass<Ports>("Ports")
                    .addFunction("__tostring", &Ports::__tostring)
                    .addFunction("__len", &Ports::__len)
                    .addFunction("all", &Ports::all)
                .endClass()
                .template beginClass<PortIterator>("PortIterator")
                    .addFunction("__tostring", &PortIterator::__tostring)
                    .addFunction("__call", &PortIterator::__call)
                .endClass()
                .template beginClass<Port>("Port")
                    .addFunction("__tostring", &Port::name)
                    .addFunction("name", &Port::name)
                    .addFunction("place", &Port::place)
                    .addFunction("transition", &Port::transition)
                    .addFunction("isInput", &Port::isInput)
                    .addFunction("isOutput", &Port::isOutput)
                    .addFunction("isTunnel", &Port::isTunnel)
                .endClass()
                ;

        luabridge::push(L, petriNet_);
        lua_setfield(L, LUA_GLOBALSINDEX, "pnet");
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

    class Places;
    class PlaceIterator;
    class Place;
    class AdjacentPortIterator;

    class Transitions;
    class TransitionIterator;
    class Transition;

    class Ports;
    class PortIterator;
    class Port;

    /**
     * A Petri net.
     */
    class PetriNet: public pnetopt::Invalidatable {
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
    class Places: public pnetopt::Invalidatable {
        PetriNet &petriNet_;

        boost::unordered_map<petri_net::pid_t, Place *> pid2place_;
        std::vector<Place *> places_;

        std::vector<RefCountedObjectPtr<PlaceIterator> > iterators_;

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

        RefCountedObjectPtr<PlaceIterator> all() {
            ensureValid();

            RefCountedObjectPtr<PlaceIterator> result(new PlaceIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        Place *getPlace(petri_net::pid_t pid) {
            Place *&result = pid2place_[pid];
            if (!result || !result->valid()) {
                /* Eliminate possibility of vector throwing an exception. */
                places_.reserve(places_.size() + 1);

                result = new Place(*this, pid, petriNet().pnet().get_place(pid));
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
            foreach (const RefCountedObjectPtr<PlaceIterator> &iterator, iterators_) {
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
    class PlaceIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable {
        Places &places_;
        typename pnet_t::place_const_it it_;

        public:

        PlaceIterator(Places &places):
            places_(places),
            it_(places.petriNet().pnet().places())
        {}

        const char *__tostring() {
            ensureValid();

            return "PlaceIterator";
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
    class Place: public RefCountedObjectType<int>, public pnetopt::Invalidatable {
        Places &places_;
        petri_net::pid_t pid_;
        const place_t &place_;

        std::vector<RefCountedObjectPtr<AdjacentPortIterator> > iterators_;

        public:

        Place(Places &places, typename petri_net::pid_t pid, const place_t &place):
            places_(places), pid_(pid), place_(place)
        {}

        Places &places() const { return places_; }

        petri_net::pid_t id() {
            ensureValid();

            return pid_;
        }

        const std::string &name() {
            ensureValid();

            return place_.get_name();
        }

        RefCountedObjectPtr<AdjacentPortIterator> in_connections() {
            ensureValid();

            RefCountedObjectPtr<AdjacentPortIterator> result(
                new AdjacentPortIterator(*this, places_.petriNet().pnet().in_to_place(pid_), true));
            iterators_.push_back(result);

            return result;
        }

        RefCountedObjectPtr<AdjacentPortIterator> out_connections() {
            ensureValid();

            RefCountedObjectPtr<AdjacentPortIterator> result(
                new AdjacentPortIterator(*this, places_.petriNet().pnet().out_of_place(pid_), false));
            iterators_.push_back(result);

            return result;
        }

        void invalidateIterators() {
            foreach (const RefCountedObjectPtr<AdjacentPortIterator> &iterator, iterators_) {
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
    class AdjacentPortIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable {
        Place &place_;
        typename petri_net::adj_transition_const_it it_;
        bool lookInOutputPorts_;

        public:

        AdjacentPortIterator(Place &place, const petri_net::adj_transition_const_it &it, bool lookInOutputPorts):
            place_(place),
            it_(it),
            lookInOutputPorts_(lookInOutputPorts)
        {}

        const char *__tostring() {
            ensureValid();

            return "AdjacentPortIterator";
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
    class Transitions: public pnetopt::Invalidatable {
        PetriNet &petriNet_;

        boost::unordered_map<petri_net::tid_t, Transition *> tid2transition_;
        std::vector<Transition *> transitions_;

        std::vector<RefCountedObjectPtr<TransitionIterator> > iterators_;

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

        RefCountedObjectPtr<TransitionIterator> all() {
            RefCountedObjectPtr<TransitionIterator> result(new TransitionIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        Transition *getTransition(petri_net::tid_t tid) {
            Transition *&result = tid2transition_[tid];
            if (!result || !result->valid()) {
                transitions_.reserve(transitions_.size() + 1);

                // WTF? I can't get non-const reference to a transition.
                result = new Transition(*this, tid, const_cast<transition_t &>(petriNet().pnet().get_transition(tid)));
                transitions_.push_back(result);
            }
            return result;
        }

        void invalidateIterators() {
            foreach (const RefCountedObjectPtr<TransitionIterator> &iterator, iterators_) {
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
    class TransitionIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable {
        Transitions &transitions_;
        typename pnet_t::transition_const_it it_;

        public:

        TransitionIterator(Transitions &transitions):
            transitions_(transitions),
            it_(transitions.petriNet().pnet().transitions())
        {}

        const char *__tostring() {
            ensureValid();

            return "TransitionIterator";
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
    class Transition: public pnetopt::Invalidatable {
        Transitions &transitions_;
        petri_net::tid_t tid_;
        transition_t &transition_;
        std::auto_ptr<Ports> ports_;
        std::auto_ptr<PetriNet> subnet_;

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

        protected:

        void doInvalidate() {
            if (ports_.get()) {
                ports_->invalidate();
            }
            if (subnet_.get()) {
                subnet_->invalidate();
            }
        }
    };

    /**
     * Ports of a transition.
     */
    class Ports: public pnetopt::Invalidatable {
        Transition &transition_;
        std::vector<RefCountedObjectPtr<PortIterator> > iterators_;

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

        RefCountedObjectPtr<PortIterator> all() {
            ensureValid();

            RefCountedObjectPtr<PortIterator> result(new PortIterator(*this));
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
            foreach (const RefCountedObjectPtr<PortIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }
    };

    /**
     * Iterator over all ports of a transition.
     */
    class PortIterator: public RefCountedObjectType<int>, public pnetopt::Invalidatable {
        Ports &ports_;
        typename transition_t::const_iterator it_;
        typename transition_t::const_iterator end_;

        public:
        
        PortIterator(Ports &ports):
            ports_(ports),
            it_(ports_.transition().transition().ports_begin()),
            end_(ports_.transition().transition().ports_end())
        {}

        const char *__tostring() {
            ensureValid();

            return "PortIterator";
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
    class Port: public pnetopt::Invalidatable {
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
                return ports_.transition().transitions().petriNet().places().getPlace(ports_.transition().transition().gen_inner_to_outer(portId_).first);
            } catch (const we::type::exception::not_connected<typename transition_t::port_id_t> &e) {
                // WTF? Why can't I check connectedness without using exceptions?
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
    };
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
    std::string script("pnetopt.lua");
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
