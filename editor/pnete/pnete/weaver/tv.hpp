// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_WEAVER_TV_HPP
#define _FHG_PNETE_WEAVER_TV_HPP 1

class QStandardItem;
class QString;

#include <string>
#include <stack>

#include <pnete/weaver/weaver.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      class tv
      {
      public:
        explicit tv (QStandardItem * root);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        std::stack<QStandardItem *> _stack;

        inline void assert_nonempty () const;
        inline void assert_empty () const;

        void push (QStandardItem * x);
        void pop ();
        QStandardItem * top ();

        void set_text (const QString & str);
        void set_text (const std::string & str);
        void add_something (const std::string & sep, const std::string & what);
        void add_type (const std::string & type);
        void add_value(const std::string & value);
        template<typename T>
        void append_key_value ( const std::string & key
                              , const std::string & fmt
                              , const T & val
                              );
        template<typename T>
        void append_maybe_key_value ( const std::string & key
                                    , const std::string & fmt
                                    , const boost::optional<T> & val
                                    );
        void append_maybe_bool ( const std::string & key
                               , const boost::optional<bool> & val
                               );
        void append_maybe ( const std::string & key
                          , const boost::optional<std::string> & val
                          );

        template<typename IT>
          void append_list (const QString& name, IT pos, const IT& end);
        template<typename C>
          void append_list (const QString& name, const C& collection);

        template<typename T> QStandardItem * append (const T & x);
        template<typename U, typename V>
          QStandardItem* append (const std::pair<U,V>& x);

        template<typename IT>
        void xs
        ( const QString & header
        , IT pos
        , const IT & end
        , void (*fun) ( tv *
                      , const typename std::iterator_traits<IT>::value_type &
                      )
        );

        template<typename Coll>
        void xs
        ( const std::string & header
        , const Coll & coll
        , void (*fun) ( tv *
                      , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                      )
        );
      };
    }
  }
}

#endif
