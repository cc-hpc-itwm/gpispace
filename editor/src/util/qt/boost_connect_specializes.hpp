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

#include <boost/preprocessor/iteration.hpp>
#include <boost/preprocessor/control.hpp>

#if !defined BOOST_PP_IS_ITERATING || !BOOST_PP_IS_ITERATING
#ifndef QTBOOSTINTEGRATION_MAX_ARGUMENTS
#define QTBOOSTINTEGRATION_MAX_ARGUMENTS 5
#endif

namespace
{
  template<typename T> struct remove_const_reference
  {
    typedef T type;
  };
  template<typename T> struct remove_const_reference<const T&>
  {
    typedef T type;
  };
}

#define BOOST_PP_ITERATION_LIMITS (0, QTBOOSTINTEGRATION_MAX_ARGUMENTS)
#define BOOST_PP_FILENAME_1 "util/qt/boost_connect_specializes.hpp"
#include BOOST_PP_ITERATE()

#else

#define QTBI_ITERATION BOOST_PP_ITERATION()

#if QTBI_ITERATION == 0
#include <boost/preprocessor/repetition.hpp>

#define QTBI_PARTIAL_SPEC void (void)
#else
#define QTBI_PARTIAL_SPEC void (BOOST_PP_ENUM_PARAMS(QTBI_ITERATION, T))
#endif

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace boost_connect_detail
      {
        template<BOOST_PP_ENUM_PARAMS(QTBI_ITERATION, typename T)>
          class connection_adapter<QTBI_PARTIAL_SPEC>
          : public abstract_connection_adapter
        {
        public:
          explicit connection_adapter
            (const boost::function<QTBI_PARTIAL_SPEC>& f)
            : m_function (f)
          { }

          static QByteArray build_signature()
          {
#if QTBI_ITERATION == 0
            return QByteArray ("fake()");
#else
            return QByteArray ("fake(")
#define QTBI_STORE_METATYPE(z, N, arg)                                          \
              .append (QMetaType::typeName (qMetaTypeId<typename remove_const_reference<BOOST_PP_CAT(T, N)>::type>())) \
              .append (N != QTBI_ITERATION - 1 ? "," : ")")

            BOOST_PP_REPEAT(QTBI_ITERATION, QTBI_STORE_METATYPE, ~);
#undef QTBI_STORE_METATYPE
#endif
          }

        private:
          virtual void invoke (void** args)
          {
#if QTBI_ITERATION == 0
            Q_UNUSED(args);
#endif

#define QTBI_PARAM(z, N, arg) *reinterpret_cast<typename remove_const_reference<BOOST_PP_CAT(T, N)>::type *> (args[N+1])
            m_function(BOOST_PP_ENUM(QTBI_ITERATION, QTBI_PARAM, ~));
#undef QTBI_PARAM
          }

          boost::function<QTBI_PARTIAL_SPEC> m_function;
        };
      }
    }
  }
}

#undef QTBI_ITERATION
#undef QTBI_PARTIAL_SPEC

#endif
