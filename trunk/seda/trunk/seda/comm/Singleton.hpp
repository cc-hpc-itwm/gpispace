#ifndef COMM_SINGLETON_HPP
#define COMM_SINGLETON_HPP 1

#include <tr1/memory>

namespace seda {
namespace comm {

    template <typename T>
    class FactoryFunction {
        public:
            typedef T class_type;
            typedef std::tr1::shared_ptr<class_type> class_ptr;

            virtual class_ptr operator()(void *) const = 0;
    };

    template <typename T>
    class Singleton {
        public:
            typedef std::tr1::shared_ptr<T> ptr_t;
            typedef FactoryFunction<T> factory_t;

            static ptr_t instance() {
                if (! instance_) {
                    instance_ = ff_();
                }
                return instance_;
            }

            static void factoryFunction(const factory_t &factory) {
                ff_ = factory;
            }
        private:
            static factory_t ff_;
            static ptr_t instance_;
    };
}}

#endif
