#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"

#include <we/we.hpp>

#define foreach BOOST_FOREACH

namespace luabridge {

      template <class T>
      struct Stack <RefCountedObject<T> >
      {
        static void push (lua_State* L, RefCountedObject<T> p)
        {
          if (p) {
          } else {
            lua_pushnil(L);
          }
          lua_pushstring (L, s.toUTF8 ());
        }

        static juce::String get (lua_State* L, int index)
        {
          if (lua_isnil(L, index)) {
            return RefCountedObject<T>();
          } else {
            return RefCo
          }
          return juce::String (luaL_checkstring (L, index));
        }
      };
}

namespace {

using namespace luabridge;

template<class P, class E, class T>
class Optimizer {
    lua_State *L;

    public:

    typedef we::type::transition_t<P, E, T> transition_t;
    typedef petri_net::net<P, transition_t, E, T> pnet_t;

    public:

    Optimizer(pnet_t &pnet) {
        L = lua_open();
        luaL_openlibs(L);

        getGlobalNamespace(L)
            .beginNamespace("pnetopt")
                .beginClass<PetriNet>("PetriNet")
                    .addFunction("__tostring", &PetriNet::__tostring)
                    .addFunction("transitions", &PetriNet::transitions)
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

        push(L, PetriNet(pnet));
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

    class Transitions;

    class PetriNet {
        pnet_t &pnet_;
        Transitions transitions_;

        public:

        PetriNet(pnet_t &pnet): pnet_(pnet), transitions_(*this) {}

        pnet_t &pnet() const {
            return pnet_;
        }

        const char *__tostring() const {
            return "PetriNet";
        }

        Transitions &transitions() {
            return transitions_;
        }
    };

    class TransitionIterator;
    class Transition;

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
            gc();

            RefCountedObjectPtr<TransitionIterator> result(new TransitionIterator(*this));
            iterators_.push_back(result);
            return result;
        };

        RefCountedObjectPtr<Transition> getTransition(petri_net::tid_t tid) {
            gc();

            RefCountedObjectPtr<Transition> &result = transitions_[tid];
            if (!result) {
                result = RefCountedObjectPtr<Transition>(new Transition(*this, tid));
            }
            return result;
        }

        void invalidateIterators() {
            foreach (RefCountedObjectPtr<TransitionIterator> &iterator, iterators_) {
                iterator->invalidate();
            }
            iterators_.clear();
        }

        void gc() {
            // TODO
        }
    };

    class TransitionIterator: public RefCountedObjectType<int> {
        Transitions &transitions_;
        typename pnet_t::transition_const_it it_;
        bool valid_;

        public:

        TransitionIterator(Transitions &transitions):
            transitions_(transitions),
            it_(transitions.petriNet().pnet().transitions()),
            valid_(true)
        {}

        const char *__tostring() {
            ensure_valid();

            return "TransitionIterator";
        }

        RefCountedObjectPtr<Transition> __call() {
            ensure_valid();

            RefCountedObjectPtr<Transition> result;
            if (it_.has_more()) {
                result = transitions_.getTransition(*it_);
                if (!result->valid()) {
                    valid_ = false;
                    ensure_valid();
                }
                ++it_;
            }
            if (!result) {
                std::cerr << "Returning zero and..." << std::endl;
            }
            return result;
        };

        void ensure_valid() {
            if (!valid_) {
//                luaL_error(L, "Accessing invalid transition iterator");
            }
        }

        void invalidate() {
            valid_ = false;
        }
    };

    class Transition: public RefCountedObjectType<int> {
        Transitions &transitions_;
        petri_net::tid_t tid_;
        const transition_t *transition_;
        bool valid_;

        public:

        Transition(Transitions &transitions, typename petri_net::tid_t tid):
            transitions_(transitions),
            tid_(tid),
            valid_(true)
        {
            try {
                transition_ = &transitions_.petriNet().pnet().get_transition(tid);
            } catch (const bijection::exception::no_such &e) {
                valid_ = false;
            }
        }

        const char *__tostring() {
            ensure_valid();

            return "Transition";
        }

        const std::string &name() {
            ensure_valid();

            return transition_->name();
        }

        void ensure_valid() {
            if (valid_) {
                try {
                    if (transition_ != &transitions_.petriNet().pnet().get_transition(tid_)) {
                        valid_ = false;
                    }
                } catch (const bijection::exception::no_such &e) {
                    valid_ = false;
                }
            }
            if (!valid_) {
//                luaL_error(L, "Accessing a deleted transition.");
            }
        }

        bool valid() {
            return valid_;
        }

        void invalidate() {
            valid_ = false;
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
        std::cerr << e.what() << std::endl;
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
