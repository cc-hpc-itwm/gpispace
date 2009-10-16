#ifndef SDPA_ACTIVITY_HPP
#define SDPA_ACTIVITY_HPP 1

#include <string>
#include <map>
#include <istream>
#include <ostream>

#include <sdpa/Properties.hpp>
#include <sdpa/wf/Parameter.hpp>

// forward decl
namespace gwes
{
  class Activity;
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
    typedef std::map<std::string, Parameter> parameters_t; //!< the type of our parameters @see sdpa::wf::Parameter
    typedef sdpa::Properties properties_t;

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
        Method(const std::string & module_at_method)
          : module_()
          , name_()
        {
          deserialize(module_at_method);
        }

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

        void operator()(const parameters_t &) {
          // \todo{implement me}
        }

        const std::string & module() const { return module_; }
        const std::string & name() const { return name_; }

        std::string serialize() const
        {
          return module() + "@" + name();
        }

        void deserialize(const std::string &bytes)
        {
          const std::string::size_type pos_of_at(bytes.find_first_of('@'));
          module_ = bytes.substr(0,           pos_of_at);
          name_   = bytes.substr(pos_of_at+1, std::string::npos);
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

    /**
      Create a new activity with the given parameters.

      @param id the id of this activity
      @param method the method to be called
      @param input a generic list of input parameters
      @param output a generic list of predefined output parameters
     */
    Activity(const activity_id_t &id, const Method & m, const parameters_t & params);

    Activity(const Activity &);
    Activity & operator=(const Activity &);

    ~Activity() {}
    
    inline const std::string & name() const { return name_; }
    inline const Method& method() const { return method_; }

    inline parameters_t & parameters() { return params_; }
    inline const parameters_t & parameters() const { return params_; }

    inline properties_t &properties() { return properties_; }
    inline const properties_t &properties() const { return properties_; }

    void add_parameter(const Parameter &);

    void writeTo(std::ostream &) const;

    // glue code for gwes
    Activity(const gwes::Activity &);
    void storeResultsInGwesActivity(gwes::Activity &);
  private:
    std::string name_;
    Method method_;
    parameters_t params_;
    properties_t properties_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a);
extern std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity::Method &m);

#endif
