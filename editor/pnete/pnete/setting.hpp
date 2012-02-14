// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_SETTING_HPP
#define _FHG_PNETE_SETTING_HPP 1

#include <QString>

#include <string>
#include <vector>
#include <set>

namespace fhg
{
  namespace pnete
  {
    namespace ui { class editor_window; }

    namespace setting
    {
      void init ();

      namespace splash
      {
        void update (const bool);
        bool show();
      }

      typedef std::string path_type;
      typedef std::set<std::string> path_set_type;
      typedef std::vector<std::string> path_list_type;

      namespace library_transition
      {
        typedef path_list_type paths_trusted_type;
        typedef path_list_type paths_untrusted_type;

        void update (const paths_trusted_type&, const paths_untrusted_type&);
        void update (ui::editor_window*);
      }
    }
  }
}

#endif
