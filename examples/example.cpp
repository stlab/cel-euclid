// SPDX-License-Identifier: BSL-1.0
#include <cel/euclid.hpp>
#include <cel/parser.hpp>
#include <iostream>
#include <stdexcept>
#include <utility>

/// Evaluates geometry expressions and reports a checked evaluation error.
///
/// Complexity: linear in expression compilation and execution time and storage.
int main() {
    cel::op_lookup lookup;
    cel::euclid::install(lookup);
    cel::parser parser(std::move(lookup));
    cel::raw_stack stack;

    auto rectangle = parser.parse_str("Rect {size: Size2D {height: 6.0, width: 4.0}, "
                                      "origin: Point2D {y: 2.0, x: 1.0}}.center().x");
    const double center_x = rectangle.prepare<double>().call(stack);
    std::cout << "center x: " << center_x << '\n';

    auto transform =
        parser.parse_str("translation(Vector2D {x: 10.0, y: 0.0}).then(scale(2.0, 2.0))"
                         ".transform_point(Point2D {x: 1.0, y: 0.0}).x");
    const double mapped_x = transform.prepare<double>().call(stack);
    std::cout << "mapped x: " << mapped_x << '\n';

    auto invalid = parser.parse_str("normalize(Vector2D {x: 0.0, y: 0.0})");
    bool caught = false;
    try {
        invalid.execute(stack, {});
    } catch (const std::runtime_error& error) {
        caught = true;
        std::cout << "checked error: " << error.what() << '\n';
    }
    return center_x == 3.0 && mapped_x == 22.0 && caught ? 0 : 1;
}
