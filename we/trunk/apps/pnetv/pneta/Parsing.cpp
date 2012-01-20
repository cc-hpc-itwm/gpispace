#include "Parsing.h"

#include <cctype>
#include <fstream>

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include <we/we.hpp>

#include "PetriNet.h"

namespace pneta {

namespace {

/**
 * \param container An associative container.
 * \param key Key.
 *
 * \return An element from the container with given key. The element must exist.
 *
 * \tparam Container Type of the associative container.
 */
template<class Container>
typename Container::mapped_type find(const Container &container, const typename Container::key_type &key) {
    typename Container::const_iterator i = container.find(key);
    assert(i != container.end());
    return i->second;
}

/**
 * Visitor for discovering subnets in workflows and translating them to Petri nets.
 */
class TransitionVisitor: public boost::static_visitor<void> {
    std::auto_ptr<PetriNet> petriNet_; ///< Resulting Petri net.
    boost::ptr_vector<PetriNet> &petriNets_; ///< Where to add the resulting Petri net when done.

    /**
     * Places present in the workflow.
     */
    boost::unordered_map<petri_net::pid_t, Place *> place2place_;

    /**
     * Helper places for implementing capacities.
     */
    boost::unordered_map<petri_net::pid_t, Place *> place2capacity_;

    /**
     * Places for storing tokens representing an activity being executed.
     */
    boost::unordered_map<petri_net::tid_t, Place *> transition2activity_;

    /**
     * Transitions present in the workflow.
     */
    boost::unordered_map<petri_net::tid_t, Transition *> transition2transition_;

    /**
     * Transitions for representing execution of an activity.
     */
    boost::unordered_map<petri_net::tid_t, Transition *> transition2execute_;

    public:

    /**
     * Constructor.
     *
     * \param[in] name of the component being visited.
     * \param[out] petriNets Where to add the result of parsing a subnet.
     */
    TransitionVisitor(const std::string &name, boost::ptr_vector<PetriNet> &petriNets):
        petriNet_(new PetriNet(name)), petriNets_(petriNets)
    { return; }

    void operator()(const we::type::expression_t & expr) { return; }

    void operator()(const we::type::module_call_t & mod_call) { return; }

    template<class P, class E, class T>
    void operator()(const petri_net::net<P, we::type::transition_t<P, E, T>, E, T> &net) {
        typedef petri_net::net<P, we::type::transition_t<P, E, T>, E, T> pnet_t;
        typedef we::type::transition_t<P, E, T> transition_t;

        /* Translate places. */
        for (typename pnet_t::place_const_it it = net.places(); it.has_more(); ++it) {
            petri_net::pid_t pid = *it;
            const P &p = net.get_place(pid);

            Place *place = petriNet_->createPlace();
            place->setName("normal_" + p.get_name());
            if (net.get_capacity(pid)) {
                place->setCapacity(*net.get_capacity(pid));
            }
            place2place_[pid] = place;

            if (place->capacity()) {
                Place *capacity = petriNet_->createPlace();
                capacity->setName("capacity_" + p.get_name());
                capacity->setCapacity(place->capacity());
                capacity->setInitialMarking(*place->capacity());
                place2capacity_[pid] = capacity;
            }
        }

        /* Translate transitions. */
        for (typename pnet_t::transition_const_it it = net.transitions(); it.has_more(); ++it) {
            petri_net::pid_t tid = *it;
            const transition_t &t = net.get_transition(tid);

            std::ostringstream condition;
            condition << t.condition();

            Transition *transition = petriNet_->createTransition();
            transition->setName("normal_" + t.name() + "[" + condition.str() + "]");
            transition->setConditionAlwaysTrue(condition.str() == "true");
            transition2transition_[tid] = transition;

            Place *activity = petriNet_->createPlace();
            activity->setName("activity_" + t.name());
            transition2activity_[tid] = activity;

            Transition *execute = petriNet_->createTransition();
            execute->setName("execute_" + t.name());
            transition2execute_[tid] = execute;

            /* When a transition is fired, an activity token is produced. */
            transition->addOutputPlace(activity);

            /* When an activity is executed, the token is consumed. */
            execute->addInputPlace(activity);
        }

        for (typename pnet_t::edge_const_it it = net.edges(); it.has_more(); ++it) {
            const typename petri_net::connection_t &connection = net.get_edge_info(*it);

            switch (connection.type) {
                case petri_net::PT: {
                    Place *place = find(place2place_, connection.pid);
                    Transition *transition = find(transition2transition_, connection.tid);

                    /* Transition consumes the token on input place. */
                    transition->addInputPlace(place);

                    if (place->capacity()) {
                        Place *capacity = find(place2capacity_, connection.pid);

                        /* Transition produces a token on capacity place. */
                        transition->addOutputPlace(capacity);
                    }
                    break;
                }
                case petri_net::PT_READ: {
                    Place *place = find(place2place_, connection.pid);
                    Transition *transition = find(transition2transition_, connection.tid);

                    /* Transition takes a token and instantly puts it back. */
                    transition->addInputPlace(place);
                    transition->addOutputPlace(place);
                    break;
                }
                case petri_net::TP: {
                    Transition *execute = find(transition2execute_, connection.tid);
                    Place *place = find(place2place_, connection.pid);

                    /* Executing the transition puts a token on output place. */
                    execute->addOutputPlace(place);

                    if (net.get_capacity(connection.pid)) {
                        Place *capacity = find(place2capacity_, connection.pid);

                        /* Transition consumes a token on capacity place. */
                        execute->addInputPlace(capacity);

                        // TODO: in fact, a place with limited capacity can receive more
                        // tokens than allowed. This is not modelled by us currently.
                        // In practice it shouldn't give much difference.
                    }
                    break;
                }
                default:
                    jpn::unreachable();
            }
        }

        for (typename pnet_t::transition_const_it it = net.transitions(); it.has_more(); ++it) {
            petri_net::pid_t id = *it;
            const transition_t &transition = net.get_transition(id);

            TransitionVisitor visitor(petriNet_->name() + "::" + transition.name(), petriNets_);
            visitor(transition);
        }
    }

    template<class P, class E, class T>
    void operator()(const we::type::transition_t<P, E, T> &transition) {
        typedef we::type::transition_t<P, E, T> transition_t;

        boost::apply_visitor(*this, transition.data());

        foreach(const typename transition_t::port_map_t::value_type &item, transition.ports()) {
            const typename transition_t::port_t &port = item.second;

            if (port.has_associated_place()) {
                petri_net::pid_t pid = port.associated_place();

                Place *place = find(place2place_, pid);
                place->setInitialMarking(place->initialMarking() + 1);

                if (place->capacity()) {
                    Place *capacity = find(place2capacity_, pid);
                    capacity->setInitialMarking(capacity->initialMarking() - 1);
                }
            }
        }

        petriNets_.push_back(petriNet_);
    }
};

} // anonymous namespace

void parse(const char *filename, boost::ptr_vector<PetriNet> &petriNets) {
    we::activity_t activity;

    {
        std::ifstream in(filename);

        if (!in) {
            throw std::runtime_error((boost::format("failed to open %1% for reading") % filename).str());
        }

        we::util::text_codec::decode(in, activity);
    }

    {
        TransitionVisitor visitor(filename, petriNets);
        visitor(activity.transition());
    }
}

} // namespace pneta

/* vim:set et sts=4 sw=4: */
