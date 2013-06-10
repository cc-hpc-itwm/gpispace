// bernd.loerwald@itwm.fraunhofer.de

#ifndef PREFIX_PARSE_HPP
#define PREFIX_PARSE_HPP

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/num.hpp>

#include <boost/bind.hpp>

#include <QColor>
#include <QString>

namespace prefix
{
  namespace require
  {
    void token (fhg::util::parse::position& pos, const std::string& what)
    {
      pos.skip_spaces();
      pos.require (what);
    }

    QString qstring (fhg::util::parse::position& pos)
    {
      token (pos, "\"");
      return QString::fromStdString (pos.until ('"'));
    }

    QString label (fhg::util::parse::position& pos)
    {
      const QString key (qstring (pos));
      token (pos, ":");
      return key;
    }

    QColor qcolor (fhg::util::parse::position& pos)
    {
      pos.skip_spaces();
      return QColor (fhg::util::read_uint (pos));
    }

    char character (fhg::util::parse::position& pos)
    {
      pos.skip_spaces();
      pos.require ("'");
      const char ch (pos.character());
      pos.require ("'");
      return ch;
    }

    bool boolean (fhg::util::parse::position& pos)
    {
      pos.skip_spaces();

      if ( pos.end() || ( *pos != '0' && *pos != '1' && *pos != 'f' && *pos != 'n'
                       && *pos != 'o' && *pos != 't' && *pos != 'y'
                        )
         )
      {
        throw fhg::util::parse::error::expected
          ("0' or '1' or 'false' or 'no' or 'off' or 'on' or 'true' or 'yes", pos);
      }

      switch (*pos)
      {
      case '0':
        ++pos;
        return false;

      case '1':
        ++pos;
        return true;

      case 'f':
        ++pos;
        pos.require ("alse");
        return false;

      case 'n':
        ++pos;
        pos.require ("o");
        return false;

      case 'o':
        ++pos;

        if (pos.end() || (*pos != 'f' && *pos != 'n'))
        {
          throw fhg::util::parse::error::expected ("ff' or 'n", pos);
        }

        switch (*pos)
        {
        case 'f':
          ++pos;
          pos.require ("f");
          return false;

        case 'n':
          ++pos;
          return true;
        }

      case 't':
        ++pos;
        pos.require ("rue");
        return true;

      case 'y':
        ++pos;
        pos.require ("es");
        return true;

      default:
        return false; // never happens, as throw above, but compilers warn.
      }
    }

    void list ( fhg::util::parse::position& pos
              , const boost::function<void (fhg::util::parse::position&)>& f
              )
    {
      pos.list ('[', ',', ']', f);
    }

    void named_list
      ( fhg::util::parse::position& pos
      , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
      )
    {
      require::list (pos, boost::bind (f, _1, label (pos)));
    }

    void list_of_named_lists
      ( fhg::util::parse::position& pos
      , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
      )
    {
      require::list (pos, boost::bind (named_list, _1, f));
    }
  }
}

#endif
