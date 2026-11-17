// SPDX-License-Identifier: BSL-1.0
#ifndef CEL_EUCLID_RECORDS_HPP
#define CEL_EUCLID_RECORDS_HPP
#include <array>
#include <cel/euclid.hpp>
#include <stdexcept>

namespace cel {
namespace euclid::detail {
/// Returns a named f64 component from a canonical record.
inline double number(const value& source, const char* name) {
    return source.field(name).as<double>();
}
/// Checks a complete record descriptor before decoding.
inline void check(const value& source, const value_type& type) {
    if (!source.type().compatible_with(type)) throw std::invalid_argument("wrong geometry record");
}
} // namespace euclid::detail

#define CEL_EUCLID_VEC_ADAPTER(TYPE, NAME, FIRST, SECOND)                                         \
    value_type record_adapter<euclid::TYPE>::type() {                                             \
        static const auto t =                                                                     \
            value_type::record(NAME, std::array{record_field{FIRST, native_type<double>()},       \
                                                record_field{SECOND, native_type<double>()}});    \
        return t;                                                                                 \
    }                                                                                             \
    value record_adapter<euclid::TYPE>::to_value(const euclid::TYPE& x) {                         \
        return value::record(type(), std::array{value::from(x.value.x), value::from(x.value.y)}); \
    }                                                                                             \
    euclid::TYPE record_adapter<euclid::TYPE>::from_value(const value& x) {                       \
        euclid::detail::check(x, type());                                                         \
        return {{euclid::detail::number(x, FIRST), euclid::detail::number(x, SECOND)}};           \
    }
CEL_EUCLID_VEC_ADAPTER(point2d, "Point2D", "x", "y")
CEL_EUCLID_VEC_ADAPTER(vector2d, "Vector2D", "x", "y")
CEL_EUCLID_VEC_ADAPTER(size2d, "Size2D", "width", "height")
#undef CEL_EUCLID_VEC_ADAPTER

value_type record_adapter<euclid::rect>::type() {
    static const auto t = value_type::record(
        "Rect", std::array{record_field{"origin", native_type<euclid::point2d>()},
                           record_field{"size", native_type<euclid::size2d>()}});
    return t;
}
value record_adapter<euclid::rect>::to_value(const euclid::rect& x) {
    return value::record(type(), std::array{value::from(x.origin), value::from(x.size)});
}
euclid::rect record_adapter<euclid::rect>::from_value(const value& x) {
    euclid::detail::check(x, type());
    return {x.field("origin").as<euclid::point2d>(), x.field("size").as<euclid::size2d>()};
}
value_type record_adapter<euclid::box2d>::type() {
    static const auto t = value_type::record(
        "Box2D", std::array{record_field{"min", native_type<euclid::point2d>()},
                            record_field{"max", native_type<euclid::point2d>()}});
    return t;
}
value record_adapter<euclid::box2d>::to_value(const euclid::box2d& x) {
    return value::record(type(), std::array{value::from(x.min), value::from(x.max)});
}
euclid::box2d record_adapter<euclid::box2d>::from_value(const value& x) {
    euclid::detail::check(x, type());
    return {x.field("min").as<euclid::point2d>(), x.field("max").as<euclid::point2d>()};
}
value_type record_adapter<euclid::angle>::type() {
    static const auto t =
        value_type::record("Angle", std::array{record_field{"radians", native_type<double>()}});
    return t;
}
value record_adapter<euclid::angle>::to_value(const euclid::angle& x) {
    return value::record(type(), std::array{value::from(x.radians)});
}
euclid::angle record_adapter<euclid::angle>::from_value(const value& x) {
    euclid::detail::check(x, type());
    return {euclid::detail::number(x, "radians")};
}
value_type record_adapter<euclid::transform2d>::type() {
    static const auto f = native_type<double>();
    static const auto t = value_type::record(
        "Transform2D",
        std::array{record_field{"m11", f}, record_field{"m12", f}, record_field{"m21", f},
                   record_field{"m22", f}, record_field{"m31", f}, record_field{"m32", f}});
    return t;
}
value record_adapter<euclid::transform2d>::to_value(const euclid::transform2d& x) {
    return value::record(type(),
                         std::array{value::from(x.value[0][0]), value::from(x.value[0][1]),
                                    value::from(x.value[1][0]), value::from(x.value[1][1]),
                                    value::from(x.value[2][0]), value::from(x.value[2][1])});
}
euclid::transform2d record_adapter<euclid::transform2d>::from_value(const value& x) {
    euclid::detail::check(x, type());
    return {glm::dmat3x2{euclid::detail::number(x, "m11"), euclid::detail::number(x, "m12"),
                         euclid::detail::number(x, "m21"), euclid::detail::number(x, "m22"),
                         euclid::detail::number(x, "m31"), euclid::detail::number(x, "m32")}};
}
} // namespace cel
#endif
