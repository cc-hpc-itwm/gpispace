#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"

#include "Invalidatable.h"

#include <we/we.hpp>

#define foreach BOOST_FOREACH

namespace {

using namespace luabridge;

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

        getGlobalNamespace(L)
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
                    .addFunction("__tostring", &Place::__tostring)
                    .addFunction("name", &Place::name)
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
                    .addFunction("__tostring", &Transition::__tostring)
                    .addFunction("name", &Transition::name)
                .endClass()
                ;

        push(L, petriNet_);
        lua_setfield(L, LUA_GLOBALSINDEX, "pnet");
    }

    ~Optimizer() {
        lua_close(L);
    }

    void operator()(const char *filename) {
        if (luaL_dofile(L, filename) != 0) {
            std::runtime_error e(lua_tostring(L, -1));
            lua_pop(L, 1);
            throw e;
        }
    }

    private:

    class Places;
    class PlaceIterator;
    class Place;

    class Transitions;
    class TransitionIterator;
    class Transition;

    class PetriNet {
        pnet_t &pnet_;
        Places places_;
        Transitions transitions_;

        public:

        PetriNet(pnet_t &pnet): pnet_(pnet), places_(*this), transitions_(*this) {}

        pnet_t &pnet() const {
            return pnet_;
        }

        const char *__tostring() const {
            return "PetriNet";
        }

        Places &places() {
            return places_;
        }

        Transitions &transitions() {
            return transitions_;
        }
    };

    class Places {
        PetriNet &petriNet_;

        std::vector<RefCountedObjectPtr<PlaceIterator> > iterators_;
        boost::unordered_map<petri_net::tid_t, RefCountedObjectPtr<Place> > places_;

        public:

        Places(PetriNet &petriNet): petriNet_(petriNet) {}

        PetriNet &petriNet() const {
            return petriNet_;
        }

        const char *__tostring() const {
            return "Places";
        }

        int __len() const {
            return petriNet_.pnet().get_num_places();
        }

        RefCountedObjectPtr<PlaceIterator> all() {
            RefCountedObjectPtr<PlaceIterator> result(new PlaceIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        RefCountedObjectPtr<Place> getPlace(petri_net::pid_t pid) {
            RefCountedObjectPtr<Place> &result = places_[pid];
            if (!result || !result->valid()) {
                result = RefCountedObjectPtr<Place>(new Place(pid, petriNet().pnet().get_place(pid)));
            }
            return result;
        }

        void invalidateIterators() {
            foreach (RefCountedObjectPtr<PlaceIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        void invalidatePlace(petri_net::tid_t tid) {
            RefCountedObjectPtr<Place> &result = places_[tid];
            if (result) {
                result->invalidate();
            }
            places_.erase(tid);
        }
    };

    class PlaceIterator: public RefCountedObjectType<int>, public trench::Invalidatable {
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

        RefCountedObjectPtr<Place> __call() {
            ensureValid();

            RefCountedObjectPtr<Place> result;
            if (it_.has_more()) {
                result = places_.getPlace(*it_);
                ++it_;
            }
            return result;
        };
    };

    class Place: public RefCountedObjectType<int>, public trench::Invalidatable {
        petri_net::pid_t pid_;
        const place_t &place_;

        public:

        Place(typename petri_net::pid_t pid, const place_t &place):
            pid_(pid),
            place_(place)
        {}

        const char *__tostring() {
            ensureValid();

            return "Place";
        }

        const std::string &name() {
            ensureValid();

            return place_.get_name();
        }
    };

    class Transitions {
        PetriNet &petriNet_;

        std::vector<RefCountedObjectPtr<TransitionIterator> > iterators_;
        boost::unordered_map<petri_net::tid_t, RefCountedObjectPtr<Transition> > transitions_;

        public:

        Transitions(PetriNet &petriNet): petriNet_(petriNet) {}

        PetriNet &petriNet() const {
            return petriNet_;
        }

        const char *__tostring() const {
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

        RefCountedObjectPtr<Transition> getTransition(petri_net::tid_t tid) {
            RefCountedObjectPtr<Transition> &result = transitions_[tid];
            if (!result || !result->valid()) {
                result = RefCountedObjectPtr<Transition>(new Transition(tid, petriNet().pnet().get_transition(tid)));
            }
            return result;
        }

        void invalidateIterators() {
            foreach (RefCountedObjectPtr<TransitionIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        void invalidateTransition(petri_net::tid_t tid) {
            RefCountedObjectPtr<Transition> &result = transitions_[tid];
            if (result) {
                result->invalidate();
            }
            transitions_.erase(tid);
        }
    };

    class TransitionIterator: public RefCountedObjectType<int>, public trench::Invalidatable {
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

        RefCountedObjectPtr<Transition> __call() {
            ensureValid();

            RefCountedObjectPtr<Transition> result;
            if (it_.has_more()) {
                result = transitions_.getTransition(*it_);
                ++it_;
            }
            return result;
        };
    };

    class Transition: public RefCountedObjectType<int>, public trench::Invalidatable {
        petri_net::tid_t tid_;
        const transition_t &transition_;

        public:

        Transition(typename petri_net::tid_t tid, const transition_t &transition):
            tid_(tid),
            transition_(transition)
        {}

        const char *__tostring() {
            ensureValid();

            return "Transition";
        }

        const std::string &name() {
            ensureValid();

            return transition_.name();
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

        typedef we::type::transition_t<P, E, T> transition_t;
        typedef petri_net::net<P, transition_t, E, T> pnet_t;

        for (typename pnet_t::transition_const_it it = net.transitions(); it.has_more(); ++it) {
            // TODO: introduce pnet_t::transition_it and remove the cast
            (*this)(const_cast<we::type::transition_t<P, E, T> &>(net.get_transition(*it)));
        }
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
