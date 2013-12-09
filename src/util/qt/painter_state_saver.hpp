// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_PAINTER_STATE_SAVER_HPP
#define FHG_UTIL_QT_PAINTER_STATE_SAVER_HPP

#include <QPainter>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      struct painter_state_saver
      {
        painter_state_saver (QPainter* const painter)
          : _painter (painter)
        {
          _painter->save();
        }
        ~painter_state_saver()
        {
          _painter->restore();
        }

      private:
        QPainter* const _painter;
      };
    }
  }
}

#endif
