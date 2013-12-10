#pragma once

#include <cassert>
#include <ostream>

#ifdef JPN_BOOST_HASH
#include <boost/functional/hash.hpp>
#endif

#include <fhg/assert.hpp>
#include <jpn/common/Unreachable.h>

namespace jpn {

/**
 * Immutable class implementing an integer that can have an infinite value.
 *
 * \tparam SIgned integer type representing a value.
 */
template<class T>
class ExtendedInteger {
    public:

    /**
     * Kind of extended integer.
     */
    enum Kind {
        NORMAL,     ///< Normal integer.
        PLUS_INF,   ///< Plus infinity.
        MINUS_INF,  ///< Minus infinity.
        INDEFINITE, ///< Indefinite value.
        KIND_COUNT  ///< Count of Kind constants.
    };

    private:

    Kind kind_; ///< Kind of the integer.
    T value_; ///< Value, valid only for normal integer.

    public:

    /**
     * Constructor.
     *
     * \param value Integer value.
     */
    ExtendedInteger(T value): kind_(NORMAL), value_(value) {}

    /**
     * Constructor.
     *
     * \param kind Kind.
     */
    explicit
    ExtendedInteger(Kind kind): kind_(kind) { assert(kind != NORMAL); }

    /**
     * \return Kind of the integer.
     */
    Kind kind() const { return kind_; }

    /**
     * \return Value of the integer, in case it is a normal one.
     */
    T value() const { assert(isNormal()); return value_; }

    /**
     * \return True iff the integer is a normal integer.
     */
    bool isNormal() const { return kind() == NORMAL; }

    /**
     * \return True iff the integer is a plus infinity.
     */
    bool isPlusInfinity() const { return kind() == PLUS_INF; }

    /**
     * \return True iff the integer is a minus infinity.
     */
    bool isMinusIfinity() const { return kind() == MINUS_INF; }

    /**
     * \return True iff the integer is an indefinity value.
     */
    bool isIndefinite() const { return kind() == INDEFINITE; }

    /**
     * Addition operator.
     *
     * \param b Extended integer.
     * \return Sum of *this and b.
     */
    ExtendedInteger<T> operator+(const ExtendedInteger<T> &b) const {
        const ExtendedInteger<T> &a = *this;

        static const Kind additionTable[KIND_COUNT][KIND_COUNT] = {
            /* NORMAL       PLUS_INF    MINUS_INF   INDEFINITE */
            { NORMAL,       PLUS_INF,   MINUS_INF,  INDEFINITE },   /* NORMAL */
            { PLUS_INF,     PLUS_INF,   INDEFINITE, INDEFINITE },   /* PLUS_INF */
            { MINUS_INF,    INDEFINITE, MINUS_INF,  INDEFINITE },   /* MINUS_INF */
            { INDEFINITE,   INDEFINITE, INDEFINITE, INDEFINITE }    /* INDEFINITE */
        };

        Kind resultKind = additionTable[a.kind()][b.kind()];
        if (resultKind == NORMAL) {
            return ExtendedInteger<T>(a.value() + b.value());
        } else {
            return ExtendedInteger<T>(resultKind);
        }
    }

    /**
     * Subtraction operator.
     *
     * \param b Extended integer.
     * \return Difference of *this and b.
     */
    ExtendedInteger<T> operator-(const ExtendedInteger<T> &b) const {
        const ExtendedInteger<T> &a = *this;

        static const Kind subtractionTable[KIND_COUNT][KIND_COUNT] = {
            /* NORMAL       PLUS_INF    MINUS_INF   INDEFINITE */
            { NORMAL,       MINUS_INF,  PLUS_INF,   INDEFINITE },   /* NORMAL */
            { PLUS_INF,     INDEFINITE, PLUS_INF,   INDEFINITE },   /* PLUS_INF */
            { MINUS_INF,    MINUS_INF,  INDEFINITE, INDEFINITE },   /* MINUS_INF */
            { INDEFINITE,   INDEFINITE, INDEFINITE, INDEFINITE }    /* INDEFINITE */
        };

        Kind resultKind = subtractionTable[a.kind()][b.kind()];
        if (resultKind == NORMAL) {
            return ExtendedInteger<T>(a.value() - b.value());
        } else {
            return ExtendedInteger<T>(resultKind);
        }
    }

    /**
     * Negation operator.
     *
     * \return The negation of *this.
     */
    ExtendedInteger<T> operator-() const {
        static const Kind negationTable[KIND_COUNT] = {
            /* NORMAL PLUS_INF MINUS_INF INDEFINITE */
            NORMAL, MINUS_INF, PLUS_INF, INDEFINITE
        };

        if (isNormal()) {
            return ExtendedInteger<T>(-value());
        } else {
            return ExtendedInteger<T>(negationTable[kind()]);
        }
    }

    /**
     * \param b An extended integer.
     *
     * \return True if *this == b.
     */
    bool operator==(const ExtendedInteger<T> &b) const {
        const ExtendedInteger<T> &a = *this;

        return a.kind() == b.kind() && (!a.isNormal() || a.value() == b.value());
    }

    /**
     * \param b An extended integer.
     *
     * \return True if *this != b.
     */
    bool operator!=(const ExtendedInteger<T> &b) const {
        return !(*this == b);
    }

    /**
     * Less than operator.
     *
     * \param b Another extended integer.
     *
     * \return True iff *this < b.
     */
    bool operator<(const ExtendedInteger<T> &b) const {
        const ExtendedInteger<T> &a = *this;

        static const bool lessThanTable[KIND_COUNT][KIND_COUNT] = {
            /* NORMAL  PLUS_INF  MINUS_INF INDEFINITE */
            { false,   true,     false,    false }, /* NORMAL */
            { false,   false,    false,    false }, /* PLUS_INF */
            { true,    true,     false,    false }, /* MINUS_INF */
            { false,   false,    false,    false }  /* INDEFINITE */
        };

        if (a.isNormal() && b.isNormal()) {
            return a.value() < b.value();
        } else {
            return lessThanTable[a.kind()][b.kind()];
        }
    }

    /**
     * Less than or equal to operator.
     *
     * \param b Another extended integer.
     *
     * \return True iff *this <= b.
     */
    bool operator<=(const ExtendedInteger<T> &b) const {
        const ExtendedInteger<T> &a = *this;

        static const bool lessOrEqualTable[KIND_COUNT][KIND_COUNT] = {
            /* NORMAL  PLUS_INF  MINUS_INF INDEFINITE */
            { false,   true,     false,    false }, /* NORMAL */
            { false,   true,     false,    false }, /* PLUS_INF */
            { true,    true,     true,     false }, /* MINUS_INF */
            { false,   false,    false,    false }  /* INDEFINITE */
        };

        if (a.isNormal() && b.isNormal()) {
            return a.value() <= b.value();
        } else {
            return lessOrEqualTable[a.kind()][b.kind()];
        }
    }

    /**
     * Greater than operator.
     *
     * \param b Another extended integer.
     *
     * \return True iff *this > b.
     */
    bool operator>(const ExtendedInteger<T> &b) const {
        return b < *this;
    }

    /**
     * Less than or equal to operator.
     *
     * \param b Another extended integer.
     *
     * \return True iff *this >= b.
     */
    bool operator>=(const ExtendedInteger<T> &b) const {
        return b <= *this;
    }
};

/**
 * Prints the extended integer to a stream.
 *
 * \param out Output stream.
 * \param a An extended integer.
 *
 * \return out.
 */
template<class T>
std::ostream &operator<<(std::ostream &out, const ExtendedInteger<T> &a) {
    switch (a.kind()) {
        case ExtendedInteger<T>::NORMAL:
            return out << a.value();
        case ExtendedInteger<T>::PLUS_INF:
            return out << "inf";
        case ExtendedInteger<T>::MINUS_INF:
            return out << "-inf";
        case ExtendedInteger<T>::INDEFINITE:
            return out << "indefinite";
        default:
            unreachable();
    }

    /* Make compiler happy. */
    return out;
}

} // namespace jpn

#ifdef JPN_BOOST_HASH
namespace boost {
    template<class T>
    struct hash<jpn::ExtendedInteger<T> > {
        std::size_t operator()(const jpn::ExtendedInteger<T> &x) const {
            if (x.isNormal()) {
                return x.value();
            } else {
                return static_cast<std::size_t>(x.kind()) << 8;
            }
        }
    };
}
#endif

/* vim:set et sts=4 sw=4: */
