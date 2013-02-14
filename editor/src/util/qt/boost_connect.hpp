// bernd.loerwald@itwm.fraunhofer.de

/* Based on http://gitorious.org/qtboostintegration/qtboostintegration/
 * Copyright 2010  Benjamin K. Stuhl <bks24@cornell.edu>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#ifndef FHG_UTIL_QT_BOOST_CONNECT_HPP
#define FHG_UTIL_QT_BOOST_CONNECT_HPP

#include <QtCore/QObject>
#include <QtCore/QMetaType>

#include <boost/function.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace boost_connect_detail
      {
        class abstract_connection_adapter
        {
        public:
          virtual ~abstract_connection_adapter() { }

          virtual void invoke (void** args) = 0;
        };

        template<typename Signature>
          class connection_adapter : public abstract_connection_adapter
        { };

        bool connect_impl ( QObject* sender
                          , const char* signal
                          , QObject* receiver
                          , abstract_connection_adapter* adapter
                          , const QByteArray& fake_signature
                          , Qt::ConnectionType connection_type
                          );
      }

      template<typename Sig>
        inline bool boost_connect ( QObject* sender
                                  , const char* signal
                                  , QObject* receiver
                                  , const boost::function<Sig>& function
                                  , Qt::ConnectionType connection_type
                                  = Qt::AutoConnection
                                  )
      {
        return boost_connect_detail::connect_impl
          ( sender
          , signal
          , receiver
          , new boost_connect_detail::connection_adapter<Sig> (function)
          , boost_connect_detail::connection_adapter<Sig>::build_signature()
          , connection_type
          );
      }

      template<typename Signature>
        inline bool boost_connect ( QObject* sender
                                  , const char* signal
                                  , const boost::function<Signature>& function
                                  , Qt::ConnectionType connection_type
                                  = Qt::AutoConnection
                                  )
      {
        return boost_connect (sender, signal, NULL, function, connection_type);
      }

      bool boost_disconnect ( QObject* sender
                            , const char* signal
                            , QObject* receiver
                            );
    }
  }
}

#include <util/qt/boost_connect_specializes.hpp>

#endif
