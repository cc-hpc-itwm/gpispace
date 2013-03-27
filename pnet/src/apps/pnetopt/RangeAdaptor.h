#pragma once

namespace pnetopt {

template<class Parent, class Container>
class RangeAdaptor: public Parent {
    const Container &container_;

    public:

    RangeAdaptor(const Parent &parent, const Container &container):
        Parent(parent), container_(container)
    {}

    std::size_t size() const { return container_.size(); }
};

template<class Base, class Container>
RangeAdaptor<Base, Container> rangeAdaptor(const Base &base, const Container &container) {
    return RangeAdaptor<Base, Container>(base, container);
}

} // namespace pnetopt

/* vim:set et sts=4 sw=4: */
