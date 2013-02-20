// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/manager.hpp>

#include <pnete/data/handle/function.hpp>

#include <xml/parse/type/function.hpp>

#include <fhg/util/xml.hpp>

#include <QObject>
#include <QString>

#include <fstream>
#include <stdexcept>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      manager::manager ()
        : _files()
        , _unnamed_current (0)
      {}

      manager& manager::instance()
      {
        static manager _instance;
        return _instance;
      }

      handle::function manager::load (const QString& filename)
      {
        bimap_type::left_map::iterator pos (_files.left.find (filename));

        if (pos != _files.left.end())
        {
          return handle::function (pos->second->function(), pos->second);
        }
        else
        {
          internal_type* ret (new internal_type (filename));

          _files.insert (bimap_type::value_type (filename, ret));

          return handle::function (ret->function(), ret);
        }
      }

      handle::function manager::create (const internal_type::kind& kind)
      {
        const QString filename ( QObject::tr ("unnamed-%1.xml")
                               . arg (++_unnamed_current)
                               );

        internal_type* ret (new internal_type (kind));

        _files.insert (bimap_type::value_type (filename, ret));

        return handle::function (ret->function(), ret);
      }

      void manager::save ( internal_type* data
                         , const boost::optional<const QString &> filename
                         ) const
      {
        const bimap_type::right_map::const_iterator
          pos (_files.right.find (data));

        if (pos == _files.right.end())
        {
          throw std::runtime_error ("save unmanaged data!?");
        }

        const std::string file
          ((filename ? *filename : pos->second).toStdString());
        std::ofstream fs (file.c_str());

        if (!fs.good())
        {
          throw std::runtime_error("could not open file " + file);
        }

        fhg::util::xml::xmlstream s (fs);

        ::xml::parse::type::dump::dump (s, data->function().get());
      }
    }
  }
}
