// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/manager.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/types.hpp>

#include <fhg/util/xml.hpp>

#include <QString>
#include <QObject>

#include <stdexcept>

#include <pnete/data/internal.hpp>

#include <fstream>

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
        static manager* _instance_ptr (0);

        if (!_instance_ptr)
        {
          _instance_ptr = new manager();
        }
        return *_instance_ptr;
      }

      internal_type* manager::load (const QString& filename)
      {
        bimap_type::left_map::iterator pos (_files.left.find (filename));

        if (pos != _files.left.end())
        {
          return pos->second;
        }
        else
        {
          internal_type* ret (new internal_type (filename));

          _files.insert (bimap_type::value_type (filename, ret));

          return ret;
        }
      }

      internal_type* manager::create()
      {
        const QString filename ( QObject::tr ("unnamed-%1.xml")
                               . arg (++_unnamed_current)
                               );

        internal_type* ret (new internal_type ());

        _files.insert (bimap_type::value_type (filename, ret));

        return ret;
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

        xml::parse::type::dump::dump (s, data->function(), data->state());
      }
    }
  }
}
