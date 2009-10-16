#ifndef SDPA_PROPERTIES_HPP
#define SDPA_PROPERTIES_HPP 1

#include <string>
#include <map>
#include <sdpa/SDPAException.hpp>
#include <sstream>

namespace sdpa {
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
        Properties()
          : properties_()
        {
        }
        virtual ~Properties() {}

        Properties(const Properties &other)
          : properties_(other.properties_)
        {
        }

        Properties &operator=(const Properties &rhs)
        {
          if (this != &rhs)
          {
            properties_ = rhs.properties_;
          }
          return *this;
        }

        void put(const std::string &key, const std::string &value);
        template <typename T> void put(const std::string &key, const T &value) throw(PropertyConversionFailed) {
            std::stringstream sstr;
            sstr << value;
            put(key, sstr.str());
        }
        std::size_t del(const std::string &key);

        bool has_key(const std::string &key) const;
        const std::string &get(const std::string &key) const throw(PropertyLookupFailed);
        const std::string &get(const std::string &key, const std::string &def) const throw();

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
            } catch (const PropertyLookupFailed &plf) {
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

        void clear();
        bool empty() const;

        void writeTo(std::ostream &) const;
    private:
        std::map<std::string, std::string> properties_;
    };
}

extern std::ostream & operator<<(std::ostream &, const sdpa::Properties &);

#endif // ! SDPA_PROPERTIES_HPP
