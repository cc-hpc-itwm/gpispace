#ifndef COMM_GLOBAL_HPP
#define COMM_GLOBAL_HPP 1

#include <string>
#include <assert.h>
#include <tr1/memory>

namespace seda {
namespace comm {
    class IGlobal {
        public:
            virtual void put(const std::string &key, const std::string &value) = 0;
            virtual std::string get(const std::string &key) = 0;
    };

    class Global {
        public:
            typedef std::tr1::shared_ptr<IGlobal> Ptr;

            static void setInstance(IGlobal* impl) { instance_.reset(impl); }
            static void put(const std::string &key, const std::string &value) {
                assert(instance_); instance_->put(key, value);
            }
            static std::string get(const std::string &key) {
                assert(instance_); return instance_->get(key);
            }
        private:
            static Ptr instance_;
    };
}}

#endif // ! COMM_GLOBAL_HPP
