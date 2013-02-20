#include "Parsing.h"

#include <cctype>
#include <fstream>

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/port.hpp>
#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <we/mgmt/type/activity.hpp>

#include <boost/range/adaptor/map.hpp>

#include "PetriNet.h"

namespace jpna {

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
    boost::unordered_map<petri_net::place_id_type, Place *> places_;

    /**
     * Transitions present in the workflow.
     */
    boost::unordered_map<petri_net::transition_id_type, Transition *> transitions_;

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

    void operator()(const petri_net::net &net) {
        typedef petri_net::net pnet_t;
        typedef we::type::transition_t transition_t;

        /* Translate places. */
        typedef std::pair<petri_net::place_id_type, place::type> ip_type;

        BOOST_FOREACH (const ip_type& ip, net.places()) {
            const petri_net::place_id_type& pid (ip.first);
            const place::type &p (ip.second);

            Place *place = petriNet_->createPlace();
            place->setName(p.name());
            places_[pid] = place;
        }

        /* Translate transitions. */
        typedef std::pair<petri_net::transition_id_type, transition_t> it_type;

        FOREACH (const it_type& it, net.transitions())
        {
          const petri_net::transition_id_type& tid (it.first);
          const transition_t& t (it.second);

            std::ostringstream condition;
            condition << t.condition();

            Transition *transition = petriNet_->createTransition();
            transition->setName(t.name() + "[" + condition.str() + "]");
            transition->setConditionAlwaysTrue(t.condition().is_const_true());
            transition->setPriority(net.get_transition_priority(tid));
            transitions_[tid] = transition;

            /* If there is a limit on number of firings, implement it using an additional place. */
            if (boost::optional<const we::type::property::value_type &> limit = t.prop().get_maybe_val("fhg.pnetv.firings_limit")) {
                Place *place = petriNet_->createPlace();
                place->setName("limit!" + t.name());
                place->setInitialMarking(boost::lexical_cast<TokenCount>(*limit));

                transition->addInputPlace(place);
            }
        }

        FOREACH (const petri_net::connection_t& connection, net.connections()) {
            switch (connection.type) {
                case petri_net::edge::PT: {
                    Place *place = find(places_, connection.pid);
                    Transition *transition = find(transitions_, connection.tid);

                    /* Transition consumes the token on input place. */
                    transition->addInputPlace(place);

                    break;
                }
                case petri_net::edge::PT_READ: {
                    Place *place = find(places_, connection.pid);
                    Transition *transition = find(transitions_, connection.tid);

                    /* Transition takes a token and instantly puts it back. */
                    transition->addInputPlace(place);
                    transition->addOutputPlace(place);
                    break;
                }
                case petri_net::edge::TP: {
                    Transition *transition = find(transitions_, connection.tid);
                    Place *place = find(places_, connection.pid);

                    /* Executing the transition puts a token on output place. */
                    transition->addOutputPlace(place);

                    break;
                }
                default:
                    jpn::unreachable();
            }
        }

        FOREACH ( const petri_net::transition_id_type& id
                , net.transitions() | boost::adaptors::map_keys
                )
        {
            const transition_t &transition = net.get_transition(id);

            TransitionVisitor visitor(petriNet_->name() + "::" + transition.name(), petriNets_);
            visitor(transition);
        }
    }

    void operator()(const we::type::transition_t &transition) {
        typedef we::type::transition_t transition_t;

        boost::apply_visitor(*this, transition.data());

        FOREACH(const transition_t::port_map_t::value_type &item, transition.ports()) {
          const we::type::port_t &port = item.second;

            if (port.has_associated_place()) {
                petri_net::place_id_type pid = port.associated_place();

                Place *place = find(places_, pid);
                place->setInitialMarking(place->initialMarking() + 1);

            }
        }

        petriNets_.push_back(petriNet_);
    }
};

} // anonymous namespace

void parse(const char *filename, boost::ptr_vector<PetriNet> &petriNets) {
    std::ifstream in(filename);

    if (!in) {
        throw std::runtime_error((boost::format("failed to open %1% for reading") % filename).str());
    }

    parse(filename, in, petriNets);
}

void parse(const char *filename, std::istream &in, boost::ptr_vector<PetriNet> &petriNets) {
  we::mgmt::type::activity_t activity (in);

    TransitionVisitor visitor(filename, petriNets);
    visitor(activity.transition());
}

} // namespace jpna

/* vim:set et sts=4 sw=4: */
