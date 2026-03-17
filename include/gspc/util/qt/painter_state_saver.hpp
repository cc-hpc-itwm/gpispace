#pragma once

#include <QtGui/QPainter>



    namespace gspc::util::qt
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

        painter_state_saver (painter_state_saver const&) = delete;
        painter_state_saver& operator= (painter_state_saver const&) = delete;
        painter_state_saver (painter_state_saver&&) = delete;
        painter_state_saver& operator= (painter_state_saver&&) = delete;

      private:
        QPainter* const _painter;
      };
    }
