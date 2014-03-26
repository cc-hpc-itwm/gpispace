#include "Parsing.h"

#include <cctype>
#include <fstream>

#include <boost/format.hpp>

#include <pnetv/jpn/common/Unreachable.h>

#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/port.hpp>
#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <we/type/activity.hpp>

#include <boost/range/adaptor/map.hpp>

#include "PetriNet.h"

#include <unordered_map>

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
    std::unordered_map<we::place_id_type, Place *> places_;

    /**
     * Transitions present in the workflow.
     */
    std::unordered_map<we::transition_id_type, Transition *> transitions_;

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

    void operator()(const we::type::expression_t &) { return; }

    void operator()(const we::type::module_call_t &) { return; }

    void operator()(const we::type::net_type &net) {
        typedef we::type::transition_t transition_t;

        /* Translate places. */
        typedef std::pair<we::place_id_type, place::type> ip_type;

        for (const ip_type& ip : net.places()) {
            const we::place_id_type& pid (ip.first);
            const place::type &p (ip.second);

            Place *place = petriNet_->createPlace();
            place->setName(p.name());
            places_[pid] = place;
        }

        /* Translate transitions. */
        for ( const std::pair<we::transition_id_type, transition_t>& it
            : net.transitions()
            )
        {
          const we::transition_id_type& tid (it.first);
          const transition_t& t (it.second);

            std::ostringstream condition;
            condition << t.condition();

            Transition *transition = petriNet_->createTransition();
            transition->setName(t.name() + "[" + condition.str() + "]");
            transition->setConditionAlwaysTrue(!t.condition());
            transition->setPriority (t.priority());
            transitions_[tid] = transition;

            /* If there is a limit on number of firings, implement it using an additional place. */
            if (boost::optional<const we::type::property::value_type &> limit = t.prop().get("fhg.pnetv.firings_limit")) {
                Place *place = petriNet_->createPlace();
                place->setName("limit!" + t.name());
                place->setInitialMarking(boost::lexical_cast<TokenCount>(*limit));

                transition->addInputPlace(place);
            }
        }

        for ( const we::type::net_type::adj_pt_type::value_type& pt
            : net.place_to_transition_consume()
            )
        {
          Place *place = ::jpna::find(places_, pt.left);
          Transition *transition = ::jpna::find(transitions_, pt.right);

          /* Transition consumes the token on input place. */
          transition->addInputPlace(place);
        }
        for ( const we::type::net_type::adj_pt_type::value_type& pt
            : net.place_to_transition_read()
            )
        {
          Place *place = ::jpna::find(places_, pt.left);
          Transition *transition = ::jpna::find(transitions_, pt.right);

          /* Transition takes a token and instantly puts it back. */
          transition->addInputPlace(place);
          transition->addOutputPlace(place);
        }
        for ( const we::type::net_type::adj_tp_type::value_type& tp
            : net.transition_to_place()
            )
        {
          Transition *transition = ::jpna::find(transitions_, tp.left);
          Place *place = ::jpna::find(places_, tp.right);

          /* Executing the transition puts a token on output place. */
          transition->addOutputPlace(place);
        }

        for ( std::pair<we::transition_id_type, we::type::transition_t> const
               id_and_transition
            : net.transitions()
            )
        {
            TransitionVisitor visitor(petriNet_->name() + "::" + id_and_transition.second.name(), petriNets_);
            visitor(id_and_transition.second);
        }
    }

    void operator()(const we::type::transition_t &transition) {
        typedef we::type::transition_t transition_t;

        boost::apply_visitor(*this, transition.data());

        for (const transition_t::port_map_t::value_type &item : transition.ports_input()) {
          const we::type::port_t &port = item.second;

            if (port.associated_place()) {
                we::place_id_type pid = *port.associated_place();

                Place *place = ::jpna::find(places_, pid);
                place->setInitialMarking(place->initialMarking() + 1);

            }
        }
        for (const transition_t::port_map_t::value_type &item : transition.ports_output()) {
          const we::type::port_t &port = item.second;

            if (port.associated_place()) {
                we::place_id_type pid = *port.associated_place();

                Place *place = ::jpna::find(places_, pid);
                place->setInitialMarking(place->initialMarking() + 1);

            }
        }
        for (const transition_t::port_map_t::value_type &item : transition.ports_tunnel()) {
          const we::type::port_t &port = item.second;

            if (port.associated_place()) {
                we::place_id_type pid = *port.associated_place();

                Place *place = ::jpna::find(places_, pid);
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
  we::type::activity_t activity (in);

    TransitionVisitor visitor(filename, petriNets);
    visitor(activity.transition());
}

} // namespace jpna
