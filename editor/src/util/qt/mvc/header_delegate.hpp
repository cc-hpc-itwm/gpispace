// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_HEADER_DELEGATE_HPP
#define FHG_UTIL_QT_HEADER_DELEGATE_HPP

#include <util/qt/mvc/header_delegate.fwd.hpp>
#include <util/qt/mvc/section_index.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class header_delegate
        {
        public:
          virtual ~header_delegate() { }

          virtual void paint
            (QPainter*, const QRect&, const section_index&) = 0;
          virtual QWidget* create_editor
            (const QRect&, delegating_header_view*, const section_index&) = 0;
          virtual void release_editor (const section_index&, QWidget* editor) = 0;
          virtual void update_editor (section_index, QWidget* editor) = 0;
          virtual bool can_edit_section (section_index) const = 0;
          virtual QMenu* menu_for_section (section_index) const = 0;
        };
      }
    }
  }
}

#endif
