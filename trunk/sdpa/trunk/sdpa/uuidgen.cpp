#include "uuidgen.hpp"
#include <uuid/uuid.h>

using namespace sdpa;

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/random_generator.hpp>

void uuidgen::generate(sdpa::uuid &uid)
{
	boost::uuids::random_generator gen;
	uid = gen();

	/*boost::mt19937 ran;
	ran.seed(time(NULL)); // one should likely seed in a better way
	boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
	uid = gen();*/
}
