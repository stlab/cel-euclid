// SPDX-License-Identifier: BSL-1.0
#include <cel/euclid.hpp>
#include <cel/parser/lookup.hpp>
#include <cmath>
#include <numbers>
#include <string_view>

#include "registration.hpp"

namespace cel::euclid {
namespace {
using detail::add;
using detail::finite;
/// Returns the box with the same endpoints as a rectangle.
box2d box(rect r) { return {r.origin, {r.origin.value + r.size.value}}; }
/// Returns the rectangle with the same endpoints as a box.
rect rectangle(box2d b) { return {b.min, {b.max.value - b.min.value}}; }
/// Returns whether a box has no interior, including unordered endpoints.
bool empty(box2d b) { return !(b.max.value.x > b.min.value.x && b.max.value.y > b.min.value.y); }
/// Returns whether a box contains a point, excluding maximum edges.
bool contains(box2d b, point2d p) {
    return b.min.value.x <= p.value.x && p.value.x < b.max.value.x && b.min.value.y <= p.value.y &&
           p.value.y < b.max.value.y;
}
/// Returns whether two boxes intersect according to Euclid's endpoint comparisons.
bool intersects(box2d a, box2d b) {
    return a.min.value.x < b.max.value.x && a.max.value.x > b.min.value.x &&
           a.min.value.y < b.max.value.y && a.max.value.y > b.min.value.y;
}
/// Returns whether a box contains the interior of another box.
bool contains_box(box2d a, box2d b) {
    return empty(b) || (a.min.value.x <= b.min.value.x && b.max.value.x <= a.max.value.x &&
                        a.min.value.y <= b.min.value.y && b.max.value.y <= a.max.value.y);
}
/// Returns Euclid's union, preserving empty operands and signed-zero comparisons.
box2d unite(box2d a, box2d b) {
    if (empty(b)) return a;
    if (empty(a)) return b;
    const auto min = [](double x, double y) { return x <= y ? x : y; };
    const auto max = [](double x, double y) { return x >= y ? x : y; };
    return {{{min(a.min.value.x, b.min.value.x), min(a.min.value.y, b.min.value.y)}},
            {{max(a.max.value.x, b.max.value.x), max(a.max.value.y, b.max.value.y)}}};
}
/// Registers equality and inequality for one nominal carrier.
template <class T, class F>
void equality(op_lookup& lookup, F equal) {
    add<bool, T, T>(lookup, "==", equal, false);
    add<bool, T, T>(lookup, "!=", [equal](T a, T b) { return !equal(a, b); }, false);
}
} // namespace

void install_geometry(op_lookup& l) {
    equality<point2d>(l, [](auto a, auto b) { return a.value == b.value; });
    equality<vector2d>(l, [](auto a, auto b) { return a.value == b.value; });
    equality<size2d>(l, [](auto a, auto b) { return a.value == b.value; });
    equality<rect>(l, [](auto a, auto b) {
        return a.origin.value == b.origin.value && a.size.value == b.size.value;
    });
    equality<box2d>(
        l, [](auto a, auto b) { return a.min.value == b.min.value && a.max.value == b.max.value; });
    equality<angle>(l, [](auto a, auto b) { return a.radians == b.radians; });
    equality<transform2d>(l, [](auto a, auto b) { return a.value == b.value; });
    add<bool, point2d>(l, "is_finite", [](auto a) { return finite(a.value); });
    add<bool, vector2d>(l, "is_finite", [](auto a) { return finite(a.value); });
    add<bool, size2d>(l, "is_finite", [](auto a) { return finite(a.value); });
    add<bool, rect>(l, "is_finite",
                    [](auto a) { return finite(a.origin.value) && finite(a.size.value); });
    add<bool, box2d>(l, "is_finite",
                     [](auto a) { return finite(a.min.value) && finite(a.max.value); });
    add<bool, angle>(l, "is_finite", [](auto a) { return std::isfinite(a.radians); });
    add<bool, transform2d>(l, "is_finite", [](auto a) { return finite(a); });

    add<point2d, point2d, vector2d>(
        l, "+", [](auto a, auto b) { return point2d{a.value + b.value}; }, false);
    add<vector2d, vector2d, vector2d>(
        l, "+", [](auto a, auto b) { return vector2d{a.value + b.value}; }, false);
    add<size2d, size2d, size2d>(
        l, "+", [](auto a, auto b) { return size2d{a.value + b.value}; }, false);
    add<vector2d, point2d, point2d>(
        l, "-", [](auto a, auto b) { return vector2d{a.value - b.value}; }, false);
    add<point2d, point2d, vector2d>(
        l, "-", [](auto a, auto b) { return point2d{a.value - b.value}; }, false);
    add<vector2d, vector2d, vector2d>(
        l, "-", [](auto a, auto b) { return vector2d{a.value - b.value}; }, false);
    add<size2d, size2d, size2d>(
        l, "-", [](auto a, auto b) { return size2d{a.value - b.value}; }, false);
    add<vector2d, vector2d>(l, "-", [](auto a) { return vector2d{-a.value}; }, false);
    add<vector2d, vector2d, double>(
        l, "*", [](auto a, double b) { return vector2d{a.value * b}; }, false);
    add<vector2d, vector2d, double>(
        l, "/", [](auto a, double b) { return vector2d{a.value / b}; }, false);
    add<size2d, size2d, double>(
        l, "*", [](auto a, double b) { return size2d{a.value * b}; }, false);
    add<size2d, size2d, double>(
        l, "/", [](auto a, double b) { return size2d{a.value / b}; }, false);

    add<double, point2d>(l, "x", [](auto a) { return a.value.x; });
    add<double, point2d>(l, "y", [](auto a) { return a.value.y; });
    add<double, vector2d>(l, "x", [](auto a) { return a.value.x; });
    add<double, vector2d>(l, "y", [](auto a) { return a.value.y; });
    add<double, size2d>(l, "width", [](auto a) { return a.value.x; });
    add<double, size2d>(l, "height", [](auto a) { return a.value.y; });
    add<point2d, rect>(l, "origin", [](auto a) { return a.origin; });
    add<size2d, rect>(l, "size", [](auto a) { return a.size; });
    add<size2d, box2d>(l, "size", [](auto a) { return size2d{a.max.value - a.min.value}; });
    add<double, rect>(l, "width", [](auto a) { return a.size.value.x; });
    add<double, rect>(l, "height", [](auto a) { return a.size.value.y; });
    add<double, box2d>(l, "width", [](auto a) { return a.max.value.x - a.min.value.x; });
    add<double, box2d>(l, "height", [](auto a) { return a.max.value.y - a.min.value.y; });
    add<double, rect>(l, "min_x", [](auto a) { return a.origin.value.x; });
    add<double, rect>(l, "min_y", [](auto a) { return a.origin.value.y; });
    add<double, rect>(l, "max_x", [](auto a) { return a.origin.value.x + a.size.value.x; });
    add<double, rect>(l, "max_y", [](auto a) { return a.origin.value.y + a.size.value.y; });
    add<double, box2d>(l, "min_x", [](auto a) { return a.min.value.x; });
    add<double, box2d>(l, "min_y", [](auto a) { return a.min.value.y; });
    add<double, box2d>(l, "max_x", [](auto a) { return a.max.value.x; });
    add<double, box2d>(l, "max_y", [](auto a) { return a.max.value.y; });
    add<point2d, box2d>(l, "min", [](auto a) { return a.min; });
    add<point2d, box2d>(l, "max", [](auto a) { return a.max; });
    add<point2d, rect>(l, "center",
                       [](auto a) { return point2d{a.origin.value + a.size.value / 2.0}; });
    add<point2d, box2d>(l, "center",
                        [](auto a) { return point2d{(a.min.value + a.max.value) / 2.0}; });

    add<double, vector2d, vector2d>(
        l, "dot", [](auto a, auto b) { return a.value.x * b.value.x + a.value.y * b.value.y; });
    add<double, vector2d, vector2d>(
        l, "cross", [](auto a, auto b) { return a.value.x * b.value.y - a.value.y * b.value.x; });
    add<double, vector2d>(l, "square_length",
                          [](auto a) { return a.value.x * a.value.x + a.value.y * a.value.y; });
    add<double, vector2d>(l, "length", [](auto a) {
        return std::sqrt(a.value.x * a.value.x + a.value.y * a.value.y);
    });
    add<double, point2d, point2d>(l, "distance_to", [](auto a, auto b) {
        auto d = a.value - b.value;
        return std::sqrt(d.x * d.x + d.y * d.y);
    });
    add<rect, rect, vector2d>(
        l, "translate", [](auto a, auto b) { return rect{{a.origin.value + b.value}, a.size}; });
    add<box2d, box2d, vector2d>(l, "translate", [](auto a, auto b) {
        return box2d{{a.min.value + b.value}, {a.max.value + b.value}};
    });
    add<rect, rect, double, double>(l, "inflate", [](auto a, double x, double y) {
        return rect{{a.origin.value - glm::dvec2{x, y}},
                    {{a.size.value.x + x + x, a.size.value.y + y + y}}};
    });
    add<box2d, box2d, double, double>(l, "inflate", [](auto a, double x, double y) {
        return box2d{{a.min.value - glm::dvec2{x, y}}, {a.max.value + glm::dvec2{x, y}}};
    });
    add<box2d, rect>(l, "to_box2d", [](auto a) { return box(a); });
    add<rect, box2d>(l, "to_rect", [](auto a) { return rectangle(a); });
    add<bool, rect, point2d>(l, "contains", [](auto a, auto p) { return contains(box(a), p); });
    add<bool, box2d, point2d>(l, "contains", [](auto a, auto p) { return contains(a, p); });
    add<bool, rect, rect>(l, "contains_rect", [](auto a, auto b) {
        return !(b.size.value.x > 0.0 && b.size.value.y > 0.0) ||
               (a.origin.value.x <= b.origin.value.x &&
                b.origin.value.x + b.size.value.x <= a.origin.value.x + a.size.value.x &&
                a.origin.value.y <= b.origin.value.y &&
                b.origin.value.y + b.size.value.y <= a.origin.value.y + a.size.value.y);
    });
    add<bool, box2d, box2d>(l, "contains_box", [](auto a, auto b) { return contains_box(a, b); });
    add<bool, rect, rect>(l, "intersects",
                          [](auto a, auto b) { return intersects(box(a), box(b)); });
    add<bool, box2d, box2d>(l, "intersects", [](auto a, auto b) { return intersects(a, b); });
    add<rect, rect, rect>(l, "union",
                          [](auto a, auto b) { return rectangle(unite(box(a), box(b))); });
    add<box2d, box2d, box2d>(l, "union", [](auto a, auto b) { return unite(a, b); });
    add<angle, double>(l, "degrees",
                       [](double a) { return angle{a * (std::numbers::pi / 180.0)}; });
    add<angle, double>(l, "radians", [](double a) { return angle{a}; });
    add<double, angle>(l, "degrees", [](auto a) { return a.radians * (180.0 / std::numbers::pi); });
    add<double, angle>(l, "radians", [](auto a) { return a.radians; });
}
} // namespace cel::euclid
