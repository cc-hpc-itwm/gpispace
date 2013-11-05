#ifndef SDPA_UTIL_PROPERTIES_HPP
#define SDPA_UTIL_PROPERTIES_HPP 1

#include <string>
#include <sdpa/SDPAException.hpp>
#include <sstream>


#include <boost/unordered_map.hpp>

namespace sdpa { namespace util {
    class PropertyLookupFailed : public sdpa::SDPAException {
    public:
        PropertyLookupFailed(const std::string &a_key)
            : sdpa::SDPAException(std::string("property not found: ") + a_key), _key(a_key) {}
        virtual ~PropertyLookupFailed() throw() {}

        const std::string &key() const { return _key; }
    private:
        std::string _key;
    };

    class PropertyConversionFailed : public sdpa::SDPAException {
    public:
        PropertyConversionFailed(const std::string &a_key, const std::string &a_value)
            : sdpa::SDPAException(std::string("property {"+a_key+","+a_value+"} could not be converted!")), _key(a_key), _value(a_value) {}
        virtual ~PropertyConversionFailed() throw() {}

        const std::string &key() const { return _key; }
        const std::string &value() const { return _value; }
    private:
        std::string _key;
        std::string _value;
    };

    /**
     * Holds a key-value store.
     *
     * warn: this class is not thread-safe.
     */
    class Properties {
    public:
        typedef boost::unordered_map<std::string, std::string> map_t;
        typedef map_t::const_iterator const_iterator;
        typedef map_t::iterator iterator;

        void put(const std::string &key, const std::string &val)
        {
          del(key);
          properties_.insert(std::make_pair(key, val));
        }
        template <typename T> void put(const std::string &key, const T &value) throw(PropertyConversionFailed) {
            std::stringstream sstr;
            sstr << value;
            put(key, sstr.str());
        }
        std::size_t del(const std::string &key)
        {
          return properties_.erase(key);
        }

        bool has_key(const std::string &key) const throw()
        {
          map_t::const_iterator it(properties_.find(key));
          return (it != properties_.end());
        }
        const std::string &get(const std::string &key) const throw(PropertyLookupFailed)
        {
          map_t::const_iterator it(properties_.find(key));
          if (it != properties_.end()) {
              return it->second;
          } else {
              throw PropertyLookupFailed(key);
          }
        }
        const std::string &get(const std::string &key, const std::string &def) const throw()
        {
          try {
              return get(key);
          } catch(...) {
              return def;
          }
        }

        template <typename T> const T get(const std::string &key) const throw(PropertyConversionFailed, PropertyLookupFailed) {
            const std::string val(get(key));
            std::stringstream sstr(val);
            T ret;
            sstr >> ret;
            if (!sstr) {
                throw PropertyConversionFailed(key, val);
            } else {
                return ret;
            }
        }
        template <typename T> const T get(const std::string &key, const T &def) const throw(PropertyConversionFailed) {
            std::string val;
            try {
                val = get(key);
            } catch (const PropertyLookupFailed&) {
                return def;
            }

            std::stringstream sstr(val);
            T ret;
            sstr >> ret;
            if (!sstr) {
                throw PropertyConversionFailed(key, val);
            } else {
                return ret;
            }
        }

        void clear()
        {
          properties_.clear();
        }
        bool empty() const
        {
          return properties_.empty();
        }

        void writeTo(std::ostream &os) const
        {
          os << "[";
          map_t::const_iterator it(properties_.begin());
          for (;;)
          {
            os << "{"
               << it->first
               << ", "
               << it->second
               << "}";

            ++it;
            if (it == properties_.end())
            {
              break;
            }
            else
            {
              os << ", ";
            }
          }
          os << "]";
        }

        const map_t & map() const { return properties_; }
        map_t & map() { return properties_; }

        const_iterator begin() const { return properties_.begin(); }
        iterator begin() { return properties_.begin(); }

        const_iterator end() const { return properties_.end(); }
        iterator end() { return properties_.end(); }

    private:
        map_t properties_;
    };

}}

inline std::ostream &operator<<(std::ostream &os, const sdpa::util::Properties &props)
{
  props.writeTo(os);
  return os;
}

#endif // ! SDPA_UTIL_PROPERTIES_HPP
