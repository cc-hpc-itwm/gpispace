// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_FWD_HPP
#define _XML_PARSE_STATE_FWD_HPP

namespace xml
{
  namespace parse
  {
    namespace state
    {
      // namespace fs = boost::filesystem;

      // typedef std::vector<std::string> search_path_type;
      // typedef std::vector<fs::path> in_progress_type;
      // typedef std::set<fs::path> dependencies_type;

      // typedef std::vector<std::string> gen_param_type;

      struct key_value_t;

      // typedef std::list<key_value_t> key_values_t;

      struct type;

      namespace detail
      {
        // typedef std::pair<std::string, std::string> pair_type;

        // inline pair_type mk (const std::string & param);

        // inline pair_type reg_M (const std::string& s);
      }
    }
  }
}

#endif
