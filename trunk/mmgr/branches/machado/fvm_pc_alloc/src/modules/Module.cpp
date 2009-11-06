#include "Module.hpp"

using namespace sdpa::modules;

Module::Module(const std::string &a_name, handle_t a_handle)
  : name_(a_name)
  , handle_(a_handle)
  , call_table_()
{
}

Module::~Module()
{
}

void Module::call(const std::string &function
                , data_t &data) const throw (FunctionNotFound, BadFunctionArgument, FunctionException, std::exception) {
  call_table_t::const_iterator f = call_table_.find(function);
  if (f == call_table_.end()) {
    throw FunctionNotFound(name(), function);
  } else {
    (*(f->second))(data);
  }
}

void Module::add_function(const std::string &function, GenericFunction f) throw (DuplicateFunction, FunctionException) {
  std::pair<call_table_t::iterator, bool> insertPosition = call_table_.insert(std::make_pair(function, f));
  if (! insertPosition.second) {
    throw DuplicateFunction(name(), function);
  }
}
