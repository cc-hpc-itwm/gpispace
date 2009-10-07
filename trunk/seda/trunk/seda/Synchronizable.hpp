#ifndef SEDA_SYNCHRONIZABLE_HPP
#define SEDA_SYNCHRONIZABLE_HPP 1

namespace seda {
    class Synchronizable {
        public:
            virtual void acquire() = 0;
            virtual void release() = 0;
    };
}

#endif
