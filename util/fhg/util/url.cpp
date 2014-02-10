#include <fhg/util/url.hpp>
#include <fhg/util/split.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace
    {
      url_t parse (std::string const &input)
      {
        url_t url;

        // type, rest = split ://
        // path, args = split on ?
        // arg_k, arg_v = split = (split & args)

        typedef std::pair<std::string, std::string> split_p;
        split_p p;

        p = split (input, "://");

        if (p.first.empty ())
        {
          throw std::invalid_argument ("no type found in url: " + input);
        }

        url.type (p.first);

        if (p.second.find ("?") != std::string::npos)
        {
          p = split (p.second, "?");
          url.path (p.first);
          std::string args = p.second;

          while (args.size ())
          {
            p = split (args, "&");

            args = p.second;

            if (p.first.empty ())
              continue;
            p = split (p.first, "=");
            if (p.first.empty ())
            {
              throw std::invalid_argument ("empty parameter in url: " + input);
            }

            url.set (p.first, p.second);
          }
        }
        else if (p.second.find ("&") == std::string::npos)
        {
          url.path (p.second);
        }
        else
        {
          throw std::invalid_argument ("malformed url: & not allowed in path");
        }

        return url;
      }
    }

    url_t::url_t (std::string const& u)
    {
      *this = parse (u);
    }
  }
}
