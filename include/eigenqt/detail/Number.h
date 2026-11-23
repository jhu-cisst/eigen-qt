// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 EigenQt contributors
#pragma once

#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>
#include <variant>
#include <vector>

namespace EigenQt::detail {
using Number = std::variant<bool, std::int64_t, std::uint64_t, double>;

template <class T> Number number(T value)
{
    static_assert(std::is_arithmetic_v<T>, "Numeric widgets require arithmetic values");
    static_assert(sizeof(T) <= sizeof(double),
                  "Extended precision values need an explicit conversion");
    if constexpr (std::is_same_v<T, bool>) {
        return value;
    } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<double>(value);
    } else if constexpr (std::is_signed_v<T>) {
        return static_cast<std::int64_t>(value);
    } else {
        return static_cast<std::uint64_t>(value);
    }
}

// Integer conversions are checked before casting. In particular, doubles at
// 2^63/2^64 must not be cast to their corresponding integral types.
template <class To, class From> bool convert(From value, To &output)
{
    static_assert(std::is_arithmetic_v<To> && std::is_arithmetic_v<From>);
    if constexpr (std::is_same_v<To, bool>) {
        if (value != From(0) && value != From(1)) {
            return false;
        }
    } else if constexpr (std::is_integral_v<To>) {
        if constexpr (std::is_floating_point_v<From>) {
            const double upper = std::ldexp(1.0, std::numeric_limits<To>::digits);
            const double lower = std::is_signed_v<To> ? -upper : 0.0;
            if (!std::isfinite(value) || std::trunc(value) != value || value < lower ||
                value >= upper) {
                return false;
            }
        } else if constexpr (std::is_signed_v<From>) {
            if (value < 0) {
                if constexpr (!std::is_signed_v<To>) {
                    return false;
                } else if (static_cast<std::intmax_t>(value) < std::numeric_limits<To>::lowest()) {
                    return false;
                }
            } else if (static_cast<std::uintmax_t>(value) >
                       static_cast<std::uintmax_t>(std::numeric_limits<To>::max())) {
                return false;
            }
        } else if (static_cast<std::uintmax_t>(value) >
                   static_cast<std::uintmax_t>(std::numeric_limits<To>::max())) {
            return false;
        }
    } else {
        if (!std::isfinite(static_cast<long double>(value)) ||
            static_cast<long double>(value) < std::numeric_limits<To>::lowest() ||
            static_cast<long double>(value) > std::numeric_limits<To>::max()) {
            return false;
        }
        if constexpr (std::is_integral_v<From>) {
            From restored{};
            if (!convert(static_cast<To>(value), restored) || restored != value) {
                return false;
            }
        }
    }
    output = static_cast<To>(value);
    return true;
}

template <class T> bool convert(const Number &value, T &output)
{
    return std::visit([&](auto scalar) { return convert(scalar, output); }, value);
}

template <class T> constexpr std::size_t numberIndex()
{
    if constexpr (std::is_same_v<T, bool>) {
        return 0;
    } else if constexpr (std::is_floating_point_v<T>) {
        return 3;
    } else if constexpr (std::is_signed_v<T>) {
        return 1;
    } else {
        return 2;
    }
}

template <class Iterator> bool collect(Iterator first, Iterator last, std::vector<Number> &output)
{
    using T = typename std::iterator_traits<Iterator>::value_type;
    static_assert(std::is_arithmetic_v<T>, "Numeric widgets require arithmetic iterator values");
    for (; first != last; ++first) {
        if (output.size() == static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            return false;
        }
        output.push_back(number(static_cast<T>(*first)));
    }
    return true;
}

template <class Iterator>
bool copyChecked(const std::vector<Number> &input, Iterator first, Iterator last)
{
    using T = typename std::iterator_traits<Iterator>::value_type;
    static_assert(std::is_base_of_v<std::forward_iterator_tag,
                                    typename std::iterator_traits<Iterator>::iterator_category>,
                  "Numeric output requires writable forward iterators");
    if (std::distance(first, last) !=
        static_cast<typename std::iterator_traits<Iterator>::difference_type>(input.size())) {
        return false;
    }
    std::vector<T> converted(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        T value{};
        if (!convert(input[i], value)) {
            return false;
        }
        converted[i] = value;
    }
    // Materialize bools rather than assuming an iterator yields T&.
    for (const T value : converted) {
        *first++ = value;
    }
    return true;
}

template <class M, class = void> struct MatrixScalar
{
    using type = std::decay_t<decltype(std::declval<const M &>()(0, 0))>;
};
template <class M> struct MatrixScalar<M, std::void_t<typename M::value_type>>
{
    using type = std::remove_const_t<typename M::value_type>;
};
} // namespace EigenQt::detail
