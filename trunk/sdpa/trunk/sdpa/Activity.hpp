#ifndef SDPA_ACTIVITY_HPP
#define SDPA_ACTIVITY_HPP 1

#include <string>
#include <list>
#include <ostream>

#include <sdpa/Properties.hpp>
#include <sdpa/Parameter.hpp>

namespace sdpa {
  class Activity : public sdpa::Properties {
  public:
    typedef std::list<Parameter> parameter_list;

    Activity(const std::string &name, const std::string &module_name, const std::string &method_name);
    Activity(const std::string &name, const std::string &module_name, const std::string &method_name, const parameter_list & input);
    Activity(const std::string &name, const std::string &module_name, const std::string &method_name, const parameter_list & input, const parameter_list & output);

    Activity(const Activity &);
    const Activity & operator=(const Activity &);

    ~Activity() {}
    
    inline const std::string & name() const { return name_; }
    inline const std::string & method_name() const { return method_name_; }
    inline const std::string & module_name() const { return module_name_; }

    inline parameter_list & input() { return input_; }
    inline parameter_list & output() { return output_; }
    inline const parameter_list & input() const { return input_; }
    inline const parameter_list & output() const { return output_; }

    void add_input(const Parameter &);
    void add_output(const Parameter &);

    void writeTo(std::ostream &) const;
  private:
    std::string name_;
    std::string module_name_;
    std::string method_name_;
    parameter_list input_;
    parameter_list output_;
  };
}

std::ostream & operator<<(std::ostream & os, const sdpa::Activity &a);

#endif
