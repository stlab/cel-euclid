// SPDX-License-Identifier: BSL-1.0
#include <array>
#include <bit>
#include <cel/euclid.hpp>
#include <cel/parser/lookup.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>

#include "registration.hpp"

namespace cel::euclid {
namespace {
using detail::add;
using detail::finite;
using detail::marker;
/// Registers a checked operation, preserving its source location in errors.
template <class R, class... A, class F>
void checked(op_lookup& lookup, std::string_view name, F operation) {
    const std::array<value_type, sizeof...(A)> inputs{native_type<A>()...};
    lookup.register_operation(
        name, inputs, native_type<R>(),
        [operation, label = std::string(name)](dyn_segment& builder, source_span span) {
            builder.op<R, A...>([operation, label, span](A... args) -> R {
                try {
                    return operation(args...);
                } catch (const std::domain_error& e) {
                    throw std::runtime_error(label + " at bytes " + std::to_string(span.begin) +
                                             ".." + std::to_string(span.end) + ": " + e.what());
                }
            });
        });
    lookup.register_function(name, marker<F>{});
}
/// Returns whether every matrix coefficient is finite.
bool finite(transform2d t) {
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 2; ++j)
            if (!std::isfinite(t.value[i][j])) return false;
    return true;
}
/// Returns the Euclid determinant.
double determinant(transform2d t) {
    auto m = t.value;
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}
/// Returns the Euclid affine image of a point.
point2d point(transform2d t, point2d p) {
    auto m = t.value;
    return {{p.value.x * m[0][0] + p.value.y * m[1][0] + m[2][0],
             p.value.x * m[0][1] + p.value.y * m[1][1] + m[2][1]}};
}
/// Returns the Euclid affine image of a vector without translation.
vector2d vector(transform2d t, vector2d p) {
    auto m = t.value;
    return {{p.value.x * m[0][0] + p.value.y * m[1][0], p.value.x * m[0][1] + p.value.y * m[1][1]}};
}
/// Returns the composition that applies a before b.
transform2d then(transform2d a, transform2d b) {
    auto x = a.value, y = b.value;
    return {
        glm::dmat3x2{x[0][0] * y[0][0] + x[0][1] * y[1][0], x[0][0] * y[0][1] + x[0][1] * y[1][1],
                     x[1][0] * y[0][0] + x[1][1] * y[1][0], x[1][0] * y[0][1] + x[1][1] * y[1][1],
                     x[2][0] * y[0][0] + x[2][1] * y[1][0] + y[2][0],
                     x[2][0] * y[0][1] + x[2][1] * y[1][1] + y[2][1]}};
}
/// A finite binary64 value factored into sign, odd significand and exponent.
struct factor {
    bool negative;
    std::uint64_t significand;
    int exponent;
};
/// Factors a finite binary64 number without rounding.
factor split(double x) {
    auto bits = std::bit_cast<std::uint64_t>(x);
    auto exponent = static_cast<int>((bits >> 52) & 0x7ff);
    auto significand =
        (bits & ((std::uint64_t{1} << 52) - 1)) | (exponent ? std::uint64_t{1} << 52 : 0);
    if (!significand) return {false, 0, 0};
    int shift = std::countr_zero(significand);
    return {std::signbit(x), significand >> shift, (exponent ? exponent : 1) + shift};
}
/// Returns whether two exact products of finite binary64 inputs are equal.
bool equal_products(double a, double b, double c, double d) {
    auto x = split(a), y = split(b), z = split(c), w = split(d);
    if (!x.significand || !y.significand) return !z.significand || !w.significand;
    if (!z.significand || !w.significand) return false;
    if ((x.negative != y.negative) != (z.negative != w.negative) ||
        x.exponent + y.exponent != z.exponent + w.exponent)
        return false;
    auto cancel = [](std::uint64_t& lhs, std::uint64_t& rhs) {
        auto g = std::gcd(lhs, rhs);
        lhs /= g;
        rhs /= g;
    };
    cancel(x.significand, z.significand);
    cancel(x.significand, w.significand);
    cancel(y.significand, z.significand);
    cancel(y.significand, w.significand);
    return x.significand == 1 && y.significand == 1 && z.significand == 1 && w.significand == 1;
}
/// Returns the inverse; rejects nonfinite or unrepresentable coefficients.
transform2d inverse(transform2d t) {
    auto m = t.value;
    auto det = determinant(t);
    if (!finite(t) || !std::isfinite(det) || det == 0.0)
        throw std::domain_error("requires finite coefficients and a nonzero finite determinant");
    const double r = 1.0 / det;
    transform2d result{glm::dmat3x2{r * m[1][1], r * (0.0 - m[0][1]), r * (0.0 - m[1][0]),
                                    r * m[0][0], r * (m[1][0] * m[2][1] - m[1][1] * m[2][0]),
                                    r * (m[2][0] * m[0][1] - m[0][0] * m[2][1])}};
    auto n = result.value;
    if (!finite(result) || (n[0][0] == 0.0 && m[1][1] != 0.0) ||
        (n[0][1] == 0.0 && m[0][1] != 0.0) || (n[1][0] == 0.0 && m[1][0] != 0.0) ||
        (n[1][1] == 0.0 && m[0][0] != 0.0) ||
        (n[2][0] == 0.0 && !equal_products(m[1][0], m[2][1], m[1][1], m[2][0])) ||
        (n[2][1] == 0.0 && !equal_products(m[2][0], m[0][1], m[0][0], m[2][1])))
        throw std::domain_error("requires representable inverse coefficients");
    return result;
}
/// Returns a normalized vector, rejecting unusable lengths and results.
vector2d normalize(vector2d v) {
    auto length = std::sqrt(v.value.x * v.value.x + v.value.y * v.value.y);
    if (!(length > 0.0) || !std::isfinite(length))
        throw std::domain_error("requires a positive finite length");
    vector2d result{v.value / length};
    if (!finite(result.value)) throw std::domain_error("requires finite components");
    return result;
}
/// Returns a projected edge magnitude or rejects overflow or complete underflow.
double edge(double coefficient, double extent) {
    double r = std::abs(coefficient) * extent;
    if (!std::isfinite(r) || (r == 0.0 && coefficient != 0.0 && extent != 0.0))
        throw std::domain_error("requires representable projected edges");
    return r;
}
/// Returns an extent whose reconstructed endpoint encloses the finite maximum.
double extent(double min, double max) {
    double result = max - min;
    if (min + result < max)
        result = std::nextafter(result, std::numeric_limits<double>::infinity());
    if (!std::isfinite(result) || !std::isfinite(min + result) || min + result < max)
        throw std::domain_error("requires representable bounds");
    return result;
}
/// Returns the axis-aligned enclosing rectangle or rejects unrepresentable geometry.
rect outer(transform2d t, rect r) {
    if (!finite(t) || !finite(r.origin.value) || !finite(r.size.value) || r.size.value.x < 0.0 ||
        r.size.value.y < 0.0)
        throw std::domain_error("requires finite inputs and nonnegative sizes");
    auto max = r.origin.value + r.size.value;
    if (!finite(max) || (r.size.value.x != 0.0 && !(max.x > r.origin.value.x)) ||
        (r.size.value.y != 0.0 && !(max.y > r.origin.value.y)))
        throw std::domain_error("requires representable source endpoints");
    auto m = t.value;
    double sx = edge(m[0][0], r.size.value.x) + edge(m[1][0], r.size.value.y);
    double sy = edge(m[0][1], r.size.value.x) + edge(m[1][1], r.size.value.y);
    if (!std::isfinite(sx) || !std::isfinite(sy)) throw std::domain_error("requires finite spans");
    std::array corners{point(t, r.origin), point(t, {{max.x, r.origin.value.y}}),
                       point(t, {{r.origin.value.x, max.y}}), point(t, {max})};
    auto lo = corners[0].value, hi = lo;
    for (auto corner : corners) {
        auto p = corner.value;
        if (!finite(p)) throw std::domain_error("requires finite mapped corners");
        if (p.x < lo.x) lo.x = p.x;
        if (p.y < lo.y) lo.y = p.y;
        if (p.x > hi.x) hi.x = p.x;
        if (p.y > hi.y) hi.y = p.y;
    }
    double width = extent(lo.x, hi.x), height = extent(lo.y, hi.y);
    if ((sx != 0.0 && width == 0.0) || (sy != 0.0 && height == 0.0))
        throw std::domain_error("requires noncollapsed projected spans");
    return {{lo}, {{width, height}}};
}
} // namespace

/// Installs checked geometry and affine operations.
void install_transforms(op_lookup& l) {
    checked<vector2d, vector2d>(l, "normalize", normalize);
    add<double, transform2d>(l, "m11", [](auto t) { return t.value[0][0]; });
    add<double, transform2d>(l, "m12", [](auto t) { return t.value[0][1]; });
    add<double, transform2d>(l, "m21", [](auto t) { return t.value[1][0]; });
    add<double, transform2d>(l, "m22", [](auto t) { return t.value[1][1]; });
    add<double, transform2d>(l, "m31", [](auto t) { return t.value[2][0]; });
    add<double, transform2d>(l, "m32", [](auto t) { return t.value[2][1]; });
    add<transform2d>(l, "identity_transform", [] { return transform2d{glm::dmat3x2{1.0}}; });
    add<transform2d, vector2d>(l, "translation", [](auto v) {
        return transform2d{glm::dmat3x2{1.0, 0.0, 0.0, 1.0, v.value.x, v.value.y}};
    });
    add<transform2d, double, double>(l, "scale", [](double x, double y) {
        return transform2d{glm::dmat3x2{x, 0.0, 0.0, y, 0.0, 0.0}};
    });
    add<transform2d, angle>(l, "rotation", [](auto a) {
        double c = std::cos(a.radians), s = std::sin(a.radians);
        return transform2d{glm::dmat3x2{c, s, 0.0 - s, c, 0.0, 0.0}};
    });
    add<transform2d, transform2d, transform2d>(l, "then", then);
    add<double, transform2d>(l, "determinant", determinant);
    checked<transform2d, transform2d>(l, "inverse", inverse);
    add<point2d, transform2d, point2d>(l, "transform_point", point);
    add<vector2d, transform2d, vector2d>(l, "transform_vector", vector);
    checked<rect, transform2d, rect>(l, "outer_transformed_rect", outer);
}
} // namespace cel::euclid
