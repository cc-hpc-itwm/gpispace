#ifndef COMM_SIMPLE_GLOBAL_HPP
#define COMM_SIMPLE_GLOBAL_HPP 1

#include <string>
#include <seda/comm/Global.hpp>

namespace seda {
namespace comm {
    class SimpleGlobal : public IGlobal {
        public:
            explicit
            SimpleGlobal(const std::string &server="");
            ~SimpleGlobal();

            void put(const std::string &k, const std::string &v);
            std::string get(const std::string &k);
    };
}}

#endif // ! COMM_SIMPLE_GLOBAL_HPP
