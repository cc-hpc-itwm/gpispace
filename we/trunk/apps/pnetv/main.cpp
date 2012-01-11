#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <we/we.hpp>

namespace {

std::string escape_tcl_string(const std::string &string) {
	std::string result;
	result.reserve(string.size());

	BOOST_FOREACH(char ch, string) {
		switch (ch) {
		case '$':
			result += "\\$";
			break;
		case '[':
			result += "\\[";
			break;
		case ']':
			result += "\\]";
			break;
		case '"':
			result += "\\\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		default:
			result.push_back(ch);
			break;
		}
	}

	return result;
}

std::string escape_filename(const std::string &string) {
	std::string result;
	result.reserve(string.size());

	BOOST_FOREACH(char ch, string) {
		if (isalnum(ch)) {
			result.push_back(ch);
		} else {
			result.push_back('_');
		}
	}

	return result;
}

class transition_visitor_eklmn: public boost::static_visitor<void> {
	std::string prefix_;
	std::ofstream out_;

	public:

	transition_visitor_eklmn(const std::string &prefix):
		prefix_(prefix),
		out_()
	{
		std::string filename = prefix + ".tcl";
		out_.open(filename.c_str());
		if (!out_) {
			throw std::runtime_error ("failed to open " + filename + " for writing");
		}
	}

	const std::string &prefix() const { return prefix_; }

	std::ostream &out() { return out_; }

        void operator()(const we::type::expression_t & expr) {
		out() << "# An expression." << std::endl;
        }

        void operator()(const we::type::module_call_t & mod_call) {
		out() << "# A module call." << std::endl;
        }

	template<class Place, class Edge, class Token>
	void operator()(const petri_net::net<Place, we::type::transition_t<Place, Edge, Token>, Edge, Token> &net) {
		typedef petri_net::net<Place, we::type::transition_t<Place, Edge, Token>, Edge, Token> pnet_t;
		typedef we::type::transition_t<Place, Edge, Token> transition_t;

		out() << "# A Petri Net." << std::endl;

		for (typename pnet_t::place_const_it it = net.places(); it.has_more(); ++it) {
			petri_net::pid_t id = *it;
			const Place &place = net.get_place(id);

			out() << "add_place $pnet p" << id << std::endl;
			out() << "set_node_attr $pnet p" << id << " label \"" << escape_tcl_string(place.get_name()) << "\"" << std::endl;

			if (boost::optional<petri_net::capacity_t> capacity = net.get_capacity(id)) {
				out() << "add_place $pnet cap" << id << std::endl;
				out() << "set_node_attr $pnet cap" << id << " label {Capacity " << *capacity << "}" << std::endl;
				out() << "set_place_marking $pnet cap" << id << " initial " << *capacity << std::endl;
			}
		}

		for (typename pnet_t::transition_const_it it = net.transitions(); it.has_more(); ++it) {
			petri_net::pid_t id = *it;
			const transition_t &transition = net.get_transition(id);

			std::ostringstream condition;
			condition << transition.condition();

			out() << "add_transition $pnet t" << id << std::endl;
			out() << "set_node_attr $pnet t" << id << " label \"" << escape_tcl_string(transition.name()) << "\"" << std::endl;
			out() << "set_node_attr $pnet t" << id << " condition \"" << escape_tcl_string(condition.str()) << "\"" << std::endl;

			out() << "add_place $pnet act" << id << std::endl;
			out() << "add_arc $pnet t" << id << " act" << id << std::endl;

			out() << "add_transition $pnet do" << id << std::endl;
			out() << "add_arc $pnet act" << id << " do" << id << std::endl;
		}

		for (typename pnet_t::edge_const_it it = net.edges(); it.has_more(); ++it) {
			const typename petri_net::connection_t &connection = net.get_edge_info(*it);

			switch (connection.type) {
				case petri_net::PT: {
					out() << "add_arc $pnet p" << connection.pid << " t" << connection.tid << std::endl;

					if (net.get_capacity(connection.pid)) {
						out() << "add_arc $pnet do" << connection.tid << " cap" << connection.pid << std::endl;
					}
					break;
				}
				case petri_net::PT_READ: {
					out() << "add_arc $pnet p" << connection.pid << " t" << connection.tid << std::endl;
					out() << "add_arc $pnet t" << connection.tid << " p" << connection.pid << std::endl;
					break;
				}
				case petri_net::TP: {
					out() << "add_arc $pnet do" << connection.tid << " p" << connection.pid << std::endl;

					if (net.get_capacity(connection.pid)) {
						out() << "add_arc $pnet cap" << connection.pid << " t" << connection.tid << std::endl;
					}
					break;
				}
				default:
					assert(!"NEVER REACHED");
			}
		}

		for (typename pnet_t::transition_const_it it = net.transitions(); it.has_more(); ++it) {
			petri_net::pid_t id = *it;
			const transition_t &transition = net.get_transition(id);

			std::cout << "Recursing..." << std::endl;

			transition_visitor_eklmn visitor((boost::format("%1%_%2%_%3%") % prefix() % id % escape_filename(transition.name())).str());
			visitor(transition);

			std::cout << "Coming back from recursion..." << std::endl;
		}
	}

	template<class Place, class Edge, class Token>
	void operator()(const we::type::transition_t<Place, Edge, Token> &transition) {
		typedef we::type::transition_t<Place, Edge, Token> transition_t;

		boost::apply_visitor(*this, transition.data());

		BOOST_FOREACH(const typename transition_t::port_map_t::value_type &item, transition.ports()) {
			const typename transition_t::port_t &port = item.second;
			if (port.has_associated_place()) {
				petri_net::pid_t id = port.associated_place();
				out() << "set_place_marking $pnet p" << id << " initial 1" << std::endl;
				out() << "if {[has_node $pnet cap" << id << "]} { set_place_marking $pnet cap" << id
				      << " initial [expr {[get_place_marking $pnet cap" << id << "]} - 1]}" << std::endl;
			}
		}
	}
};

} // anonymous namespace

int main(int argc, char *argv[]) {
	std::string input;
	std::string output_prefix;

	namespace po = boost::program_options;

	po::options_description desc("options");

	desc.add_options()
	( "help,h", "this message")
	( "input,i",
	  po::value<std::string>(&input)->default_value("-"),
	  "input file name, - for stdin"
	)
	( "output-prefix,o",
	  po::value<std::string>(&output_prefix)->default_value("net_"),
	  "output file name prefix"
	);

	po::positional_options_description p;
	p.add("input", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return EXIT_SUCCESS;
	}

	if (input == "-") {
		input = "/dev/stdin";
	}

	we::activity_t activity;

	{
		std::ifstream in(input.c_str());

		if (!in) {
			throw std::runtime_error ("failed to open " + input + " for reading");
		}

		we::util::text_codec::decode(in, activity);
	}

	{
		transition_visitor_eklmn visitor(output_prefix);
		visitor(activity.transition());
	}

	return EXIT_SUCCESS;
}
