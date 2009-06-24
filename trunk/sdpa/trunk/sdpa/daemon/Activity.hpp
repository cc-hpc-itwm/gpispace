#ifndef SDPA_ACTIVITY_HPP
#define SDPA_ACTIVITY_HPP 1

#include <string>
#include <list>
#include <ostream>

#include <sdpa/Properties.hpp>
#include <sdpa/daemon/Parameter.hpp>

namespace sdpa { namespace daemon {
  /**
    This class describes an abstract activity to be executed.

    An activity is defined by some name, a module (shared  object  file)  and  a
    method name.  In addition to that, an activity may have an arbitrary  number
    of input parameters and predefined output parameters.
    */
  class Activity : public sdpa::Properties {
  public:
    typedef std::list<Parameter> parameter_list; //!< the type of our parameters @see sdpa::daemon::Parameter

    /**
      This class encapsulates a method call to a generic method.

      A method is defined by a shared-object or module name in which the
      function code is implemented and a method name. The function will be
      invoked using a generic input parameter list and a generic output
      parameter list.

      @see sdpa::daemon::Parameter
     */
    class Method {
      public:
        Method(const std::string & module, const std::string & method_name)
          : module_(module), name_(method_name) {}

        void operator()(const parameter_list & in, parameter_list & out) {
          // \todo{implement me}
        }

        const std::string & module() const { return module_; }
        const std::string & name() const { return name_; }
      private:
        std::string module_;
        std::string name_;
    };

    /**
      Create a new activity using the given method.

      @param name the name of this activity
      @param method the method to be used
     */
    Activity(const std::string &name, const Method & m);

    /**
      Create a new activity with the given parameters.

      @param name the name of this activity
      @param method the method to be called
      @param input a generic list of input parameters
     */
    Activity(const std::string &name, const Method & m, const parameter_list & input);

    /**
      Create a new activity with the given parameters.

      @param name the name of this activity
      @param method the method to be called
      @param input a generic list of input parameters
      @param output a generic list of predefined output parameters
     */
    Activity(const std::string &name, const Method & m, const parameter_list & input, const parameter_list & output);

    Activity(const Activity &);
    const Activity & operator=(const Activity &);

    ~Activity() {}
    
    inline const std::string & name() const { return name_; }
    inline const Method& method() const { return method_; }

    inline parameter_list & input() { return input_; }
    inline parameter_list & output() { return output_; }
    inline const parameter_list & input() const { return input_; }
    inline const parameter_list & output() const { return output_; }

    void add_input(const Parameter &);
    void add_output(const Parameter &);

    void writeTo(std::ostream &) const;
  private:
    std::string name_;
    Method method_;
    parameter_list input_;
    parameter_list output_;
  };
}}

extern std::ostream & operator<<(std::ostream & os, const sdpa::daemon::Activity &a);

#endif
