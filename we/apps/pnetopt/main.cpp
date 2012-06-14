#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <lua.hpp>
#include "LuaBridge/LuaBridge.h"

#include <we/we.hpp>

#define foreach BOOST_FOREACH

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
                .template beginClass<TransitionsIterator>("TransitionsIterator")
                    .addFunction("__tostring", &TransitionsIterator::__tostring)
                    .addFunction("__call", &TransitionsIterator::__call)
                .endClass()
                .template beginClass<Transition>("Transition")
                    .addFunction("__tostring", &Transition::__tostring)
                    .addFunction("name", &Transition::name)
                .endClass();

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

    class Transition {
        petri_net::tid_t tid_;
        const transition_t &transition_;

        public:

        Transition(pnet_t &pnet, typename petri_net::tid_t tid):
            tid_(tid), transition_(pnet.get_transition(tid))
        {}

        const char *__tostring() {
            return "Transition";
        }

        const std::string &name() {
            return transition_.name();
        }
    };

    class TransitionsIterator {
        pnet_t &pnet_;
        typename pnet_t::transition_const_it it_;

        public:

        TransitionsIterator(pnet_t &pnet):
            pnet_(pnet), it_(pnet.transitions())
        {}

        const char *__tostring() {
            return "Transitions Iterator";
        }

        Transition *__call() {
            if (it_.has_more()) {
                // Memory leak here.
                Transition *result = new Transition(pnet_, *it_);
                ++it_;
                return result;
            } else {
                return NULL;
            }
        };
    };

    class Transitions {
        pnet_t &pnet_;

        public:

        Transitions(pnet_t &pnet): pnet_(pnet) {}

        const char *__tostring() {
            return "Petri Net Transitions";
        }

        int __len() {
            return pnet_.get_num_transitions();
        }

        TransitionsIterator all() {
            return TransitionsIterator(pnet_);
        }
    };

    class PetriNet {
        pnet_t &pnet_;
        Transitions transitions_;

        public:

        PetriNet(pnet_t &pnet): pnet_(pnet), transitions_(pnet) {}

        const char *__tostring() {
            return "Petri Net";
        }

        Transitions &transitions() {
            return transitions_;
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
