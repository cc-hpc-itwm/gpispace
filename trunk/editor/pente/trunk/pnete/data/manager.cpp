// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/manager.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/types.hpp>

#include <fhg/util/xml.hpp>

#include <QString>
#include <QObject>

#include <stdexcept>

// remove me
#include <iostream>

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

      internal_type::ptr manager::load (const QString& filename)
      {
        bimap_type::left_map::iterator pos (_files.left.find (filename));

        if (pos != _files.left.end())
          {
            return pos->second;
          }
        else
          {
            internal_type::ptr ret (new internal_type (filename));

            _files.insert (bimap_type::value_type (filename, ret));

            return ret;
          }
      }

      internal_type::ptr manager::create()
      {
        const QString filename ( QObject::tr ("unnamed-%1.xml")
                               . arg (++_unnamed_current)
                               );

        internal_type::ptr ret (internal_type::ptr (new internal_type ()));

        _files.insert (bimap_type::value_type (filename, ret));

        return ret;
      }

      void manager::save ( const internal_type::ptr & data
                         , const boost::optional<const QString &> filename
                         ) const
      {
        const bimap_type::right_map::const_iterator
          pos (_files.right.find (data));

        if (pos == _files.right.end())
          {
            throw std::runtime_error ("save unmanaged data!?");
          }

        fhg::util::xml::xmlstream s (std::cout);
        xml::parse::type::dump::dump (s, data->function(), data->state());

        if (filename)
          {
            //! \todo look if filename already present.

            // TODO
            // dump (data, *filename)
          }
        else
          {
            // TODO
            // dump (data, pos->second);
          }
      }
    }
  }
}
