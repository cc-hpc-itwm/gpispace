#ifndef SDPA_ACTIVITY_HPP
#define SDPA_ACTIVITY_HPP 1

#include <string>
#include <map>
#include <istream>
#include <ostream>

#include <sdpa/util/Properties.hpp>
#include <sdpa/wf/Parameter.hpp>

namespace sdpa { namespace wf {
  typedef std::map<std::string, Parameter> parameters_t; //!< the type of our parameters @see sdpa::wf::Parameter

  /**
    This class encapsulates a method call to a generic method.

    A method is defined by a shared-object or module name in which the
    function code is implemented and a method name. The function will be
    invoked using a generic input parameter list and a generic output
    parameter list.

    @see sdpa::wf::Parameter
   */
  class Method {
    public:
      explicit
      Method(const std::string & module_at_method)
        : module_()
        , name_()
      {
        deserialize(module_at_method);
      }

      Method()
        : module_()
        , name_()
      { }

      Method(const std::string & a_module, const std::string & a_method_name)
        : module_(a_module)
        , name_(a_method_name) {}

      Method(const Method& other)
        : module_(other.module())
        , name_(other.name())
      {}

      Method &operator=(const Method &rhs)
      {
        if (this != &rhs)
        {
          module_ = rhs.module();
          name_ = rhs.name();
        }
        return *this;
      }

      const std::string & module() const { return module_; }
      std::string & module() { return module_; }
      const std::string & name() const { return name_; }
      std::string & name() { return name_; }

      std::string serialize() const
      {
        return module() + "@" + name();
      }

      void deserialize(const std::string &bytes)
      {
        const std::string::size_type pos_of_at(bytes.find_first_of('@'));
        if (pos_of_at != std::string::npos)
        {
          module_ = bytes.substr(0,           pos_of_at);
          name_   = bytes.substr(pos_of_at+1, std::string::npos);
        }
/*
        else
        {
          throw std::runtime_error("could not deserialize method: \"" + bytes + "\" did not contain @ separator");
        }
*/
      }

      void writeTo(std::ostream &os) const
      {
        os << "{" << "method" << "," << module() << "," << name()  << "}";
      }

      void readFrom(std::istream &is)
      {
        std::string tmp;
        is >> tmp;
        deserialize(tmp);
      }
    private:
      std::string module_;
      std::string name_;
  };
}}

inline std::ostream & operator<<(std::ostream & os, const sdpa::wf::Method &m)
{
  m.writeTo(os);
  return os;
}

namespace sdpa { namespace wf {
  /**
    This class describes an abstract activity to be executed.

    An activity is defined by some name, a module (shared  object  file)  and  a
    method name.  In addition to that, an activity may have an arbitrary  number
    of input parameters and predefined output parameters.
    */
  class Activity {
  public:
    typedef std::string activity_id_t;
    typedef shared_ptr<Activity> ptr_t;
    typedef sdpa::util::Properties properties_t;

    enum state_t {
      ACTIVITY_UNKNOWN
    , ACTIVITY_FINISHED
    , ACTIVITY_FAILED
    , ACTIVITY_CANCELED
    };

    /**
      Create a new activity with the given parameters.

      @param name the name of this activity
      @param method the method to be called
      @param input a generic list of input parameters
      @param output a generic list of predefined output parameters
     */
    Activity(const activity_id_t &a_name, const Method & a_method, const parameters_t & params = parameters_t())
      : name_(a_name)
      , method_(a_method)
      , params_(params)
      , state_(ACTIVITY_UNKNOWN)
    { }

    Activity()
      : name_("")
      , method_("")
      , state_(ACTIVITY_UNKNOWN)
    { }

    Activity(const Activity &other)
      : name_(other.name())
      , method_(other.method())
      , params_(other.parameters())
      , properties_(other.properties())
      , state_(other.state())
    { }

    Activity& operator=(const Activity &rhs) {
      if (this != &rhs)
      {
        name_ = rhs.name();
        method_ = rhs.method();
        params_ = rhs.parameters();
        properties_ = rhs.properties();
        state_ = rhs.state();
      }
      return *this;
    }

    ~Activity() {}
    
    inline const std::string & name() const { return name_; }
    inline std::string & name() { return name_; }

    inline const Method& method() const { return method_; }
    inline Method& method() { return method_; }

    inline const parameters_t & parameters() const { return params_; }
    inline parameters_t & parameters() { return params_; }

    inline const properties_t &properties() const { return properties_; }
    inline properties_t &properties() { return properties_; }

    inline const state_t &state() const { return state_; }
    inline state_t &state() { return state_; }

    void add_parameter(const Parameter &p)
    {
      params_.insert(std::make_pair(p.name(), p));
    }

    void writeTo(std::ostream &os) const
    {
      os << "{"
         << "act"
         << ","
         << name()
         << ","
         << method()
         << ","
         << "{" << "params"
         << ","
         << "[";
      for (parameters_t::const_iterator p(parameters().begin());;) {
        os << p->second;
        ++p;
        if (p == parameters().end())
        {
          break;
        }
        else
        {
          os << ",";
        }
      }
      os << "]"
         << "}"
         << "}";
    }
  private:
    std::string name_;
    Method method_;
    parameters_t params_;
    properties_t properties_;
    state_t state_;
  };
}}

inline std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a)
{
  a.writeTo(os);
  return os;
}

#endif
