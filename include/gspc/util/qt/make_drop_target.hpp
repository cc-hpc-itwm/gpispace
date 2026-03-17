#pragma once

#include <gspc/util/qt/forward_decl.hpp>

#include <functional>

FHG_UTIL_QT_FORWARD_DECL (class QDropEvent)
FHG_UTIL_QT_FORWARD_DECL (class QMimeData)
FHG_UTIL_QT_FORWARD_DECL (class QWidget)



    namespace gspc::util::qt
    {
      void make_drop_target ( QWidget* widget
                            , std::function<bool (QMimeData const*)> check_accepted
                            , std::function<void (QDropEvent const*)> on_drop
                            );

      void make_drop_target ( QWidget* widget
                            , QString const& accepted_mime_type
                            , std::function<void (QDropEvent const*)> on_drop
                            );
    }
