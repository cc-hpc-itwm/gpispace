// bernd.loerwald@itwm.fraunhofer.de

#ifndef PREFIX_PARSE_HPP
#define PREFIX_PARSE_HPP

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
      prefix::require::list (pos, boost::bind (f, _1, label (pos)));
    }

    void list_of_named_lists
      ( fhg::util::parse::position& pos
      , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
      )
    {
      prefix::require::list (pos, boost::bind (named_list, _1, f));
    }
  }
}

#endif
