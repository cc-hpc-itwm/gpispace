/*
 * =============================================================================
 *
 *       Filename:  id_generator.hpp
 *
 *    Description:  generates message ids
 *
 *        Version:  1.0
 *        Created:  12/13/2009 08:27:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =============================================================================
 */

#ifndef SDPA_ID_GENERATOR_HPP
#define SDPA_ID_GENERATOR_HPP 1

#include <sys/types.h> // pid_t
#include <unistd.h> // gethostname, getpid
#include <string.h> // memset
#include <time.h>   // time
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace sdpa {
  namespace detail
  {
    struct dflt_tag
    {
      static const char *name ()
      {
        return "id";
      }
    };
  }

  template <class Tag>
  class id_generator
  {
  private:
    typedef boost::mutex mutex_type;
    typedef boost::lock_guard<mutex_type> lock_type;

  public:

	static id_generator& instance()
	{
	  static id_generator<Tag> gen;
	  return gen;
	}

	std::string next()
	{
          size_t id;
          {
            lock_type lock (mtx_);
            id = m_count++;
          }

          std::ostringstream os;
          os << m_id_prefix << "." << id;
          return os.str ();
        }
  private:
    id_generator()
      : mtx_ ()
      , m_count (0)
    {
      char hbuf [1024];
      memset (hbuf, 0, sizeof(hbuf));
      gethostname (hbuf, sizeof(hbuf)-1);

      std::ostringstream sstr;
      sstr << hbuf << "." << Tag::name () << "." << time (0) << "." << getpid();

      m_id_prefix = sstr.str ();
    }

    mutable mutex_type mtx_;
    size_t      m_count;
    std::string m_id_prefix;
  };

  typedef id_generator<detail::dflt_tag> dflt_id_generator;
}

#endif
