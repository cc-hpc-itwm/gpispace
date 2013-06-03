// mirko.rahn@itwm.fraunhofer.de

#include <pnete/setting.hpp>

#include <pnete/ui/editor_window.hpp>

#include <fhg/revision.hpp>

#include <QSettings>
#include <QApplication>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace setting
    {
      namespace
      {
        void maybe_set
          (QSettings& settings, const QString& key, const QVariant& value)
        {
          if (!settings.value (key).isValid())
          {
            settings.setValue (key, value);
          }
        }

        void maybe_set_default_settings()
        {
          QSettings settings;

          settings.beginGroup ("gantt");

          maybe_set (settings, "created", QColor (128, 128, 128));
          maybe_set (settings, "started", QColor (255, 255, 0));
          maybe_set (settings, "finished", QColor (0, 200, 0));
          maybe_set (settings, "failed", QColor (255, 0, 0));
          maybe_set (settings, "cancelled", QColor (165, 42, 42));

          settings.endGroup();
        }
      }

      void init ()
      {
        QApplication::setApplicationName ("pnete");
        QApplication::setApplicationVersion
          ( QString (fhg::project_version())
          .append (" - ")
          .append (fhg::project_revision())
          );
        QApplication::setOrganizationDomain ("itwm.fhg.de");
        QApplication::setOrganizationName ("Fraunhofer ITWM");

        maybe_set_default_settings();
      }

#define CONST(_type,_name,_value)                       \
      static const _type& _name()                       \
      {                                                 \
        static const _type value (_value);              \
                                                        \
        return value;                                   \
      }

      namespace splash
      {
        CONST(QString, key, "show_splash");

        void update (const bool value) { QSettings().setValue (key(), value); }
        bool show() { return QSettings().value (key()).toBool(); }
      }

      namespace template_filename
      {
        CONST(QString, key, "template_filename");

        void update (const QString& value) { QSettings().setValue (key(), value); }
        QString show() { return QSettings().value (key()).toString(); }
      }

      namespace library_transition
      {
        namespace key
        {
          CONST(QString, group, "library_transition");
          CONST(QString, path, "path");
          CONST(QString, paths_trusted, "paths_trusted");
          CONST(QString, paths_untrusted, "paths_untrusted");
        }

        namespace detail
        {
          static path_set_type read (const QString& key)
          {
            path_set_type paths;

            QSettings settings;

            settings.beginGroup (key::group());

            const int size (settings.beginReadArray (key));

            for (int i (0); i < size; ++i)
              {
                settings.setArrayIndex (i);

                paths.insert
                  (settings.value (key::path()).toString().toStdString());
              }

            settings.endArray();
            settings.endGroup();

            return paths;
          }

          static void write (const QString& key, const path_set_type& paths)
          {
            QSettings settings;

            settings.beginGroup (key::group());

            settings.beginWriteArray (key);

            int index (0);

            BOOST_FOREACH(const path_type& path, paths)
              {
                settings.setArrayIndex (index++);

                settings.setValue (key::path(), QString::fromStdString (path));
              }

            settings.endArray();
            settings.endGroup();
          }

          static void update ( const QString& key
                             , const path_list_type& paths_new
                             )
          {
            path_set_type paths_old (detail::read (key));

            paths_old.insert (paths_new.begin(), paths_new.end());

            detail::write (key, paths_old);
          }

          static void add_paths ( ui::editor_window* editor_window
                                , const QString& key
                                , const bool trusted
                                )
          {
            QSettings settings;

            settings.beginGroup (key::group());

            const int size (settings.beginReadArray (key));

            for (int i (0); i < size; ++i)
              {
                settings.setArrayIndex (i);
                editor_window->add_transition_library_path
                  (settings.value (key::path()).toString(), trusted);
              }
            settings.endArray();

            settings.endGroup ();
          }

        }

        void update ( const path_list_type& paths_trusted_new
                    , const path_list_type& paths_untrusted_new
                    )
        {
          detail::update (key::paths_trusted(), paths_trusted_new);
          detail::update (key::paths_untrusted(), paths_untrusted_new);
        }

        void update (ui::editor_window* editor_window)
        {
          detail::add_paths (editor_window, key::paths_trusted(), true);
          detail::add_paths (editor_window, key::paths_untrusted(), false);
        }
      }
#undef CONST
    }
  }
}
