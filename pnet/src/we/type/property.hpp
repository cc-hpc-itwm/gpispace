// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_HPP
#define _WE_TYPE_PROPERTY_HPP

#include <we/type/property.fwd.hpp>

#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <stack>

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef std::string key_type;
      typedef std::string value_type;

      struct type;

      typedef boost::variant < boost::recursive_wrapper<type>
                             , value_type
                             > mapped_type;

      typedef std::vector<key_type> path_type;
      typedef path_type::const_iterator path_iterator;
      typedef boost::unordered_map<key_type, mapped_type> map_type;

      //! \note Used when storing containers, to allow storing structured types.
      template <typename T> value_type to_property (const T& t);
      template <typename T> void store_value
        (type* properties, const key_type& key, const T& t);
      template <typename T> T from_property (const value_type& value);
      template <typename T>  T retrieve_value
        (const type& properties, const key_type& key);

      // ******************************************************************* //

      namespace util
      {
        path_type split (const key_type& s, const char& sep = '.');
      }

      // ******************************************************************* //

      namespace exception
      {
        class missing_binding : public fhg::util::backtracing_exception
        {
        private:
          std::string nice ( path_type::const_iterator pos
                           , const path_type::const_iterator end
                           );

        public:
          missing_binding ( path_type::const_iterator pos
                          , const path_type::const_iterator end
                          );
        };

        class empty_path : public std::runtime_error
        {
        public:
          empty_path (const std::string& pre);
        };

        class not_a_map : public std::runtime_error
        {
        public:
          not_a_map (const std::string& pre);
        };

        class not_a_val : public std::runtime_error
        {
        public:
          not_a_val (const std::string& pre);
        };
      }

      struct type
      {
      private:
        map_type map;

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(map);
        }

        // ----------------------------------------------------------------- //

        const mapped_type& get ( const path_iterator& pos
                               , const path_iterator& end
                               , const path_iterator& zero
                               ) const;

        const value_type& get_val ( const path_iterator& pos
                                  , const path_iterator& end
                                  , const path_iterator& zero
                                  ) const;

        const boost::optional<const value_type&>
          get_maybe_val ( const path_iterator& pos
                        , const path_iterator& end
                        , const path_iterator& zero
                        ) const;

        // ----------------------------------------------------------------- //

      public:
        type();

        const map_type& get_map (void) const;

        boost::optional<mapped_type> set ( const path_iterator& pos
                                         , const path_iterator& end
                                         , const value_type& val
                                         );
        boost::optional<mapped_type>
          set (const path_type& path, const value_type& val);
        boost::optional<mapped_type>
          set (const std::string& path, const value_type& val);

        template <typename C>
          void set_container (const key_type& key, const C& c)
        {
          store_value (this, key + ".size", c.size());
          std::size_t i (0);
          for ( typename C::const_iterator it (c.begin())
              ; it != c.end()
              ; ++it
              )
          {
            store_value
              (this, key + "." + boost::lexical_cast<key_type> (i), *it);
            ++i;
          }
        }

        //! \todo Sanity checking? (missing binding -> throw invalid_container)
        template <typename C>
          C get_container (const key_type& key)
        {
          typedef typename C::value_type value_type;

          C result;

          const std::size_t size
            (retrieve_value<std::size_t> (*this, key + ".size"));
          //! \todo Detect if C::reserve exists (does not with std::list)
          // result.reserve (size);
          for (std::size_t i (0); i < size; ++i)
          {
            result.insert
              ( result.end()
              , retrieve_value<value_type>
                (*this, key + "." + boost::lexical_cast<key_type> (i))
              );
          }

          return result;
        }

        const mapped_type& get
          (const path_iterator& pos, const path_iterator& end) const;
        const mapped_type& get (const path_type& path) const;
        const mapped_type& get (const std::string& path) const;

        const value_type& get_val
          (const path_iterator& pos, const path_iterator& end) const;
        const value_type& get_val (const path_type& path) const;
        const value_type& get_val (const std::string& path) const;

        const boost::optional<const value_type&> get_maybe_val
          (const path_iterator& pos, const path_iterator& end) const;
        const boost::optional<const value_type&>
          get_maybe_val (const path_type& path) const;
        const boost::optional<const value_type&>
          get_maybe_val (const std::string& path) const;

        const value_type& get_with_default ( const path_iterator& pos
                                           , const path_iterator& end
                                           , const value_type& dflt
                                           ) const;
        const value_type& get_with_default ( const path_type& path
                                           , const value_type& dflt
                                           ) const;
        const value_type& get_with_default ( const std::string& path
                                           , const value_type& dflt
                                           ) const;

        void del (const path_iterator& pos, const path_iterator& end);
        void del (const path_type& path);
        void del (const std::string& path);

        bool has (const path_iterator& pos, const path_iterator& end) const;
        bool has (const path_type& path) const;
        bool has (const std::string& path) const;
      };

      template <typename T> value_type to_property (const T& t)
      {
        return boost::lexical_cast<value_type> (t);
      }

      template <typename T> void store_value
        (type* properties, const key_type& key, const T& t)
      {
        properties->set (key, to_property (t));
      }

      template <typename T> T from_property (const value_type& value)
      {
        return boost::lexical_cast<T> (value);
      }

      template <typename T>  T retrieve_value
        (const type& properties, const key_type& key)
      {
        return from_property<T> (properties.get_val (key));
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const type&);
        void ordered_dump (::fhg::util::xml::xmlstream&, const type&);
      }

      std::ostream& operator<< (std::ostream& s, const type& t);

      // ******************************************************************* //

      namespace traverse
      {
        typedef std::pair<path_type, value_type> pair_type;
        typedef std::stack<pair_type> stack_type;

        stack_type dfs (const type& t);
        stack_type dfs ( const type& t
                       , const path_iterator pre_pos
                       , const path_iterator pre_end
                       );
        stack_type dfs (const type& t, const path_type& path);
        stack_type dfs (const type& t, const std::string& path);
      }
    }
  }
}

#endif
