// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <we/type/literal/cpp.hpp>

#include <fhg/util/show.hpp>

#include <fhg/util/cpp.hpp>

namespace cpp_util = fhg::util::cpp;

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

namespace signature
{
  namespace cpp
  {
    template<typename Stream>
    void level (Stream& s, const unsigned int& j)
    {
      for (unsigned int i (0); i < j; ++i)
        {
          s << "  ";
        }
    }

    // ******************************************************************** //

    namespace visitor
    {
      // ****************************************************************** //

      //! \todo refactor this to avoid these this-> stuff

      template<typename Stream>
      class cpp_generic : public boost::static_visitor<Stream&>
      {
      protected:
        Stream& s;
        unsigned int l;

        void level (const unsigned int& j) const
        {
          for (unsigned int i (0); i < j; ++i)
            {
              s << "  ";
            }
        }
      public:
        cpp_generic (Stream& _s, const unsigned int& _l = 0)
          : s(_s), l(_l)
        {}
      };

      // ****************************************************************** //

      template<typename Stream>
      class cpp_from_value : public cpp_generic<Stream>
      {
      private:
        typedef cpp_generic<Stream> super;

        std::string fieldname;

        std::string
        longer (const std::string & sub) const
        {
          return fieldname.empty() ? sub : (fieldname + "." + sub);
        }

      public:
        cpp_from_value (Stream& _s, const unsigned int _l = 1)
          : super (_s, _l), fieldname ()
        {}

        cpp_from_value ( const std::string & _fieldname
                       , Stream& _s
                       , const unsigned int _l = 0
                       )
          : super (_s, _l), fieldname (_fieldname)
        {}

        Stream& operator () (const std::string & t) const
        {
          this->level (this->l+1);
          this->s << "x." << fieldname << " = ";

          if (literal::cpp::known (t))
            {
              this->s << cpp_util::access::make ("", "value", "get")
                      << "< "
                      << literal::cpp::translate (t)
                      << " >";
            }
          else
            {
              this->s << cpp_util::access::make ( cpp_util::access::type()
                                                , t
                                                , "from_value"
                                                );
            }

          this->s << " (v_" << (this->l-1) << ");" << std::endl;

          return this->s;
        }

        Stream& operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              this->level (this->l+1); this->s << "{" << std::endl;

              this->level (this->l+2);
              this->s
                << "const " << cpp_util::access::value_type()
                << " & v_" << this->l << " "
                << "(" << cpp_util::access::make ("", "value", "get_level")
                <<  "(\"" << field->first << "\"" << ", v_" << (this->l-1) << ")"
                << ");"
                << std::endl;

              boost::apply_visitor
                ( cpp_from_value (longer (field->first), this->s, this->l+1)
                , field->second
                );

              this->level (this->l+1); this->s << "}" << std::endl;
            }

          return this->s;
        }
      };

      template<typename Stream>
      class cpp_serialize : public cpp_generic<Stream>
      {
      private:
        typedef cpp_generic<Stream> super;

        std::string fieldname;

        std::string
        longer (const std::string & sub) const
        {
          return fieldname.empty() ? sub : (fieldname + "." + sub);
        }

      public:
        cpp_serialize (Stream& _s, const unsigned int _l = 1)
          : super (_s, _l), fieldname ()
        {}

        cpp_serialize ( const std::string & _fieldname
                      , Stream& _s
                      , const unsigned int _l = 0
                      )
          : super (_s, _l), fieldname (_fieldname)
        {}

        Stream& operator () (const std::string & t) const
        {
          this->level (this->l+1);

          this->s
            << "ar & BOOST_SERIALIZATION_NVP (" << fieldname << ");"
            << std::endl
            ;

          return this->s;
        }

        Stream& operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor
                ( cpp_serialize (longer (field->first), this->s, this->l)
                , field->second
                );
            }

          return this->s;
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      template<typename Stream>
      class cpp_to_value : public cpp_generic<Stream>
      {
      private:
        typedef cpp_generic<Stream> super;

        std::string fieldname;
        std::string levelname;

        std::string
        longer (const std::string & sub) const
        {
          return fieldname.empty() ? sub : (fieldname + "." + sub);
        }

      public:
        cpp_to_value (Stream& _s, const unsigned int _l = 0)
          : super (_s, _l), fieldname ()
        {}

        cpp_to_value ( const std::string & _fieldname
                     , const std::string & _levelname
                     , Stream& _s
                     , const unsigned int _l = 0
                     )
          : super (_s, _l)
          , fieldname (_fieldname)
          , levelname (_levelname)
        {}

        Stream& operator () (const std::string & t) const
        {
          this->level (this->l); this->s << "v_" << (this->l-1) << "[\"" << levelname << "\"]"
                       << " = ";

          if (literal::cpp::known (t))
            {
              this->s << "::literal::type (x." << fieldname << ")";
            }
          else
            {
              this->s << cpp_util::access::make ( cpp_util::access::type()
                                          , t, "to_value"
                                          )
                << "(x." << fieldname << ")"
                ;
            }

          this->s << ";" << std::endl;

          return this->s;
        }

        Stream& operator () (const structured_t & map) const
        {
          this->level (this->l); this->s << "{" << std::endl;

          this->level (this->l+1); this->s << cpp_util::access::make ("","value","structured_t")
                         << " v_" << this->l << ";"
                         << std::endl
                         ;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor
                ( cpp_to_value (longer (field->first), field->first, this->s, this->l+1)
                , field->second
                );
            }

          if (fieldname.empty())
            {
              this->level (this->l+1); this->s << "return v_" << this->l << ";" << std::endl;
            }
          else
            {
              this->level (this->l+1); this->s << "v_" << (this->l-1) << "[\"" << levelname << "\"]"
                             << " = " << "v_" << this->l << ";"
                             << std::endl
                             ;
            }

          this->level (this->l); this->s << "} // to_value " << levelname << std::endl;

          return this->s;
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      typedef boost::unordered_set<std::string> seen_type;

      template<typename Stream>
      class cpp_includes : public boost::static_visitor<void>
      {
      private:
        Stream& os;
        seen_type & seen;
        const boost::filesystem::path & incpath;

      public:
        cpp_includes ( Stream& _os
                     , seen_type & _seen
                     , const boost::filesystem::path& _incpath
                     )
          : os (_os), seen (_seen), incpath (_incpath)
        {}

        void operator () (const std::string & t) const
        {
          if (!literal::cpp::known (t))
            {
              if (seen.find (t) == seen.end())
                {
                  const boost::filesystem::path file
                    (incpath / cpp_util::make::hpp (t));

                  cpp_util::include (os, file);

                  seen.insert (t);
                }
            }
        }

        void operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor ( cpp_includes (os, seen, incpath)
                                   , field->second
                                   );
            }
        }
      };
    }

    // ********************************************************************** //

    namespace visitor
    {
      template<typename Stream>
      class cpp_struct : public cpp_generic<Stream>
      {
      private:
        const boost::optional<const std::string &> name;
        const boost::optional<const type &> sig;

      public:
        cpp_struct ( const type & _sig
                   , const std::string & _name
                   , Stream& _s
                   , const unsigned int _l = 0
                   )
          : cpp_generic<Stream> (_s, _l), name (_name), sig (_sig)
        {}

        cpp_struct ( Stream& _s
                   , const unsigned int _l = 0
                   )
          : cpp_generic<Stream> (_s, _l), name (boost::none), sig (boost::none)
        {}

        Stream& operator () (const std::string & t) const
        {
          if (literal::cpp::known (t))
            {
              this->s << literal::cpp::translate (t);
            }
          else
            {
              this->s << cpp_util::access::make (cpp_util::access::type(), t, t);
            }

          return this->s;
        }

        Stream& operator () (const structured_t & map) const
        {
          this->s << "struct ";

          if (name)
            {
              this->s << *name << " ";
            }

          this->s << "{" << std::endl;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              this->level(this->l+1);

              boost::apply_visitor (cpp_struct (this->s, this->l+1), field->second);

              this->s << " " << fhg::util::show (field->first) << ";" << std::endl;
            }

          if (sig)
            {
              this->level (this->l+1); this->s << "friend class boost::serialization::access;" << std::endl;
              this->level (this->l+1); this->s << "template<typename Archive>" << std::endl;
              this->level (this->l+1); this->s << "void serialize (Archive & ar, const unsigned int)" << std::endl;
              this->level (this->l+1); this->s << "{" << std::endl;

              boost::apply_visitor ( visitor::cpp_serialize<Stream> (this->s, this->l+1)
                                   , (*sig).desc()
                                   );

              this->level (this->l+1); this->s << "} // serialize " << (name ? *name : "UNKNOWN") << std::endl;
            }

          this->level (this->l); this->s << "}";

          return this->s;
        }
      };
    }

    template<typename Stream>
    void cpp_struct ( Stream& os
                    , const type & s
                    , const std::string & n
                    , const unsigned int & l = 1
                    )
    {
      boost::apply_visitor ( visitor::cpp_struct<Stream> (s, n, os, l)
                           , s.desc()
                           );

      os << "; // struct " << n << std::endl;
      os << std::endl;

      level (os, l); os << "inline " << n << " from_value (const "
                        << cpp_util::access::value_type()
                        << "& v_" << (l-1) << ")"
                        << std::endl;
      level (os, l); os << "{" << std::endl;
      level (os, l); os << "  " << n << " x;" << std::endl;

      boost::apply_visitor ( visitor::cpp_from_value<Stream> (os, l)
                           , s.desc()
                           );

      level (os, l); os << "  return x;" << std::endl;
      level (os, l); os << "} // from_value " << std::endl;
      os << std::endl;

      level (os, l); os << "inline " << cpp_util::access::value_type()
                        << " to_value (const " << n << " & x)"
                        << std::endl;

      boost::apply_visitor (visitor::cpp_to_value<Stream> (os, l), s.desc());
    }

    template<typename Stream>
    void cpp_struct (Stream& os, const type & s)
    {
      cpp_struct (os, s, s.nice());
    }

    // ********************************************************************** //

    namespace visitor
    {
      template<typename Stream>
      class cpp_show : public cpp_generic<Stream>
      {
      private:
        const std::string & field_local;
        const std::string & field_global;

        const unsigned int l0;
        const bool _is_toplevel;

      public:
        cpp_show ( Stream& _s
                 , const std::string & _field_local
                 , const std::string & _field_global
                 , const bool is_toplevel
                 , const unsigned int & _l0 = 0
                 , const unsigned int & _l = 0
                 )
          : cpp_generic<Stream> (_s, _l)
          , field_local (_field_local)
          , field_global (_field_global)
          , l0 (_l0)
          , _is_toplevel (is_toplevel)
        {}

        Stream& operator () (const std::string & t) const
        {
          this->level (l0);

          this->s << "s << \"" << field_local << " = \" << ";

          if (literal::cpp::known (t))
            {
              this->s << cpp_util::access::make ("","literal","show")
                << "(" << cpp_util::access::make ("", "literal", "type")
                << "(x" << fhg::util::show (field_global) << ")"
                << ")" << std::endl;
                ;
            }
          else
            {
              this->s << "x" << fhg::util::show (field_global);
            }

          this->s << ";" << std::endl;

          return this->s;
        }

        Stream& operator () (const structured_t & map) const
        {
          this->level (l0);

          this->s << "s << \"";
          if (!_is_toplevel)
          {
            this->s << field_local << " = ";
          }
          this->s << "{\";" << std::endl;

          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              if (field != map.begin())
              {
                this->level (l0);
                this->s << "s << \", \";" << std::endl;
              }

              boost::apply_visitor
                ( cpp_show ( this->s
                           , field->first
                           , field_global + "." + field->first
                           , false
                           , l0
                           , this->l+1
                           )
                , field->second
                );
            }

          this->level (l0);

          this->s << "s << \"}\";" << std::endl;

          return this->s;
        }
      };
    }

    template<typename Stream>
    void cpp_show ( Stream& os
                  , const type & s
                  , const std::string & n
                  , const unsigned int & l = 0
                  )
    {
      os << std::endl;

      level (os, l);

      os << "inline std::ostream & operator << (std::ostream & s, const "
         << n << " & x)"                          << std::endl;

      level (os, l);

      os << "{"                                   << std::endl;

      boost::apply_visitor ( visitor::cpp_show<Stream> (os, "", "", true, l + 1)
                           , s.desc()
                           );

      os                                          << std::endl;

      level (os, l);

      os << "  return s;"                         << std::endl;

      level (os, l);

      os << "}"                                   << std::endl;
    }

    template<typename Stream>
    void cpp_show ( Stream& os
                  , const type & s
                  , const unsigned int & l = 0
                  )
    {
      cpp_show (os, s, s.nice(), l);
    }

    // ********************************************************************* //

    template<typename Stream>
    void cpp_header ( Stream& os
                    , const type & s
                    , const std::string & n
                    , const boost::filesystem::path & defpath
                    , const boost::filesystem::path & incpath
                    )
    {
      cpp_util::header_gen_full (os);
      cpp_util::include_guard_begin (os, "PNETC_TYPE_" + n);

      cpp_util::include (os, "we/type/value.hpp");
      cpp_util::include (os, "we/type/value/cpp/get.hpp");

      // for serialization
      cpp_util::include (os, "boost/serialization/nvp.hpp");

      // for operator <<
      cpp_util::include (os, "we/type/literal.hpp");
      cpp_util::include (os, "we/type/literal/show.hpp");
      cpp_util::include (os, "iostream");

      os << std::endl;

      visitor::seen_type seen;

      boost::apply_visitor ( visitor::cpp_includes<Stream> (os, seen, incpath)
                           , s.desc()
                           );

      os << "namespace pnetc"                                    << std::endl;
      os << "{"                                                  << std::endl;
      os << "  namespace type"                                   << std::endl;
      os << "  {"                                                << std::endl;
      os                                                         << std::endl;
      os << "    // defined in " << defpath                      << std::endl;
      os << "    namespace " << n                                << std::endl;
      os << "    {"                                              << std::endl;

      os << "      "; cpp_struct<Stream> (os, s, n, 3);

      os                                                         << std::endl;

      os << "      "; cpp_show<Stream> (os, s, n, 3);

      os << "    } // namespace " << n                           << std::endl;
      os << "  } // namespace type"                              << std::endl;
      os << "} // namespace pnetc"                               << std::endl;
      os                                                         << std::endl;

      cpp_util::include_guard_end (os, "PNETC_TYPE_" + n);
    }

    template<typename Stream>
    void cpp_header (Stream& os, const type & s)
    {
      cpp_header<Stream> ( os, s, s.nice()
                         , boost::filesystem::path ("<unknown>")
                         , boost::filesystem::path ("")
                         );
    }
  }
}

#endif
