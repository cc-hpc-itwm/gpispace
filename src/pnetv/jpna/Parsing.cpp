#include "Parsing.h"

#include "PetriNet.h"

#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/port.hpp>
#include <we/type/transition.hpp>

#include <fhg/util/boost/optional.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <fstream>
#include <memory>
#include <unordered_map>

namespace jpna {

namespace {

/**
 * Visitor for discovering subnets in workflows and translating them to Petri nets.
 */
class TransitionVisitor: public boost::static_visitor<void> {
    std::unique_ptr<PetriNet> petriNet_; ///< Resulting Petri net.
    boost::ptr_vector<PetriNet> &petriNets_; ///< Where to add the resulting Petri net when done.

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
        petriNet_(fhg::util::cxx14::make_unique<PetriNet> (name)), petriNets_(petriNets)
    { return; }

    void operator()(const we::type::expression_t &) { return; }

    void operator()(const we::type::module_call_t &) { return; }

    void operator()(const we::type::net_type &net) {
        typedef we::type::transition_t transition_t;

        /* Translate places. */
        for (std::size_t _ (0); _ < net.places().size(); ++_)
        {
            petriNet_->createPlace (0);
        }

        /* Translate transitions. */
        for (auto const& it : net.transitions())
        {
          const we::transition_id_type& tid (it.first);
          const transition_t& t (it.second);

            std::ostringstream condition;
            condition << t.condition();

            Transition *transition = petriNet_->createTransition
              ( t.name() + "[" + condition.str() + "]"
              , !t.condition()
              , t.priority()
              );
            transitions_[tid] = transition;

            /* If there is a limit on number of firings, implement it using an additional place. */
            if (boost::optional<const we::type::property::value_type &> limit = t.prop().get({"fhg", "pnetv", "firings_limit"})) {
              transition->addInputPlace
                (petriNet_->createPlace (boost::get<TokenCount> (*limit)));
            }
        }

        for ( const we::type::net_type::adj_pt_type::value_type& pt
            : net.place_to_transition_consume()
            )
        {
          Transition *transition = transitions_.at (pt.right);

          /* Transition consumes the token on input place. */
          transition->addInputPlace (pt.left);
        }
        for ( const we::type::net_type::adj_pt_type::value_type& pt
            : net.place_to_transition_read()
            )
        {
          Transition *transition = transitions_.at (pt.right);

          /* Transition takes a token and instantly puts it back. */
          transition->addInputPlace (pt.left);
          transition->addOutputPlace (pt.left);
        }
        for ( const we::type::net_type::adj_tp_type::value_type& tp
            : net.transition_to_place()
            )
        {
          Transition *transition = transitions_.at (tp.second);

          /* Executing the transition puts a token on output place. */
          transition->addOutputPlace (tp.first);
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
        boost::apply_visitor(*this, transition.data());

        for ( const we::type::port_t& port
            : transition.ports_input() | boost::adaptors::map_values
            )
        {
          if (port.associated_place())
          {
            petriNet_->increment_token_count (*port.associated_place());
          }
        }
        for ( const we::type::port_t& port
            : transition.ports_output() | boost::adaptors::map_values
            )
        {
          if (port.associated_place())
          {
            petriNet_->increment_token_count (*port.associated_place());
          }
        }
        for ( const we::type::port_t& port
            : transition.ports_tunnel() | boost::adaptors::map_values
            )
        {
          if (port.associated_place())
          {
            petriNet_->increment_token_count (*port.associated_place());
          }
        }

        petriNets_.push_back(petriNet_.release());
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
  parse (filename, we::type::activity_t (in), petriNets);
}

void parse ( const char* name
           , we::type::activity_t const& activity
           , boost::ptr_vector<PetriNet>& nets
           )
{
  TransitionVisitor visitor (name, nets);
  visitor (activity.transition());
}

} // namespace jpna
