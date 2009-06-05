#include "Module.hpp"

using namespace sdpa::modules;

Module::Module(const std::string &name, handle_t handle)
  : name_(name), handle_(handle) {
}

Module::~Module() {
}

void Module::call(const std::string &function, const input_data_t &input, output_data_t &output) const throw (FunctionNotFound, BadFunctionArgument, FunctionException) {
  call_table_t::const_iterator f = call_table_.find(function);
  if (f == call_table_.end()) {
    throw FunctionNotFound(name(), function);
  } else {
    (*(f->second))(input, output);
  }
}

void Module::add_function(const std::string &function, GenericFunction f) throw (DuplicateFunction, FunctionException) {
  std::pair<call_table_t::iterator, bool> insertPosition = call_table_.insert(std::make_pair(function, f));
  if (! insertPosition.second) {
    throw DuplicateFunction(name(), function);
  }
}
