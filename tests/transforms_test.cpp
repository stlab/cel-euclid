// SPDX-License-Identifier: BSL-1.0
#include <cel/euclid.hpp>
#include <cel/parser/compiler.hpp>
#include <cel/std.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

namespace {
/// Compiles and evaluates an expression with the complete geometry library.
cel::value evaluate(std::string_view expression) {
    cel::op_lookup lookup;
    cel::stdlib::install(lookup);
    cel::euclid::install(lookup);
    cel::parser parser(std::move(lookup));
    cel::raw_stack stack;
    return parser.parse_str(expression).execute(stack, {});
}
/// Returns whether a compiled expression reports a source-located evaluation error.
bool evaluation_error(std::string_view expression, std::string_view operation) {
    cel::op_lookup lookup;
    cel::stdlib::install(lookup);
    cel::euclid::install(lookup);
    cel::parser parser(std::move(lookup));
    auto program = parser.parse_str(expression);
    cel::raw_stack stack;
    try {
        program.execute(stack, {});
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        return message.find(operation) != std::string::npos &&
               message.find("bytes ") != std::string::npos;
    }
    return false;
}
} // namespace

TEST_CASE("transform_then_order_is_observable") {
    const auto p = "Point2D {x: 1.0, y: 0.0}";
    CHECK(
        evaluate(
            std::string(
                "translation(Vector2D {x: 10.0, y: 0.0}).then(scale(2.0, 2.0)).transform_point(") +
            p + ").x")
            .as<double>() == 22.0);
    CHECK(
        evaluate(
            std::string(
                "scale(2.0, 2.0).then(translation(Vector2D {x: 10.0, y: 0.0})).transform_point(") +
            p + ").x")
            .as<double>() == 12.0);
}

TEST_CASE("checked_normalization_rejects_invalid_lengths") {
    CHECK(evaluate("normalize(Vector2D {x: 3.0, y: 4.0}).x").as<double>() == 0.6);
    CHECK(evaluation_error("normalize(Vector2D {x: 0.0, y: 0.0})", "normalize"));
    CHECK(evaluation_error("normalize(Vector2D {x: 1e200, y: 1.0})", "normalize"));
    CHECK(evaluation_error("normalize(Vector2D {x: 1e-200, y: 0.0})", "normalize"));
    CHECK(evaluation_error("normalize(Vector2D {x: 0.0 / 0.0, y: 1.0})", "normalize"));
    CHECK(evaluation_error("normalize(Vector2D {x: 1.0 / 0.0, y: 1.0})", "normalize"));
}

TEST_CASE("matrix_fields_match_euclid_mapping") {
    const std::string m =
        "Transform2D {m11: 2.0, m12: 1.0, m21: 3.0, m22: 4.0, m31: 5.0, m32: 6.0}";
    CHECK(evaluate(m + ".transform_point(Point2D {x: 1.0, y: 0.0}).x").as<double>() == 7.0);
    CHECK(evaluate(m + ".transform_point(Point2D {x: 1.0, y: 0.0}).y").as<double>() == 7.0);
    CHECK(evaluate(m + ".determinant()").as<double>() == 5.0);
    CHECK(evaluate(m + ".m31()").as<double>() == 5.0);
}

TEST_CASE("singular_or_nonfinite_inverse_errors") {
    CHECK(evaluation_error("scale(0.0, 1.0).inverse()", "inverse"));
    CHECK(evaluation_error("scale(1e200, 1e200).inverse()", "inverse"));
    CHECK(evaluation_error("translation(Vector2D {x: 1.0 / 0.0, y: 0.0}).inverse()", "inverse"));
    const std::string matrix =
        "Transform2D {m11: 1.0, m12: 2.0, m21: 3.0, m22: 4.0, m31: 5.0, m32: 6.0}";
    for (const auto* bad : {"(0.0 / 0.0)", "(1.0 / 0.0)", "(-1.0 / 0.0)"}) {
        for (int i = 1; i <= 6; ++i) {
            auto input = matrix;
            auto number = std::to_string(i) + ".0";
            input.replace(input.find(number), number.size(), bad);
            CHECK(evaluation_error("inverse(" + input + ")", "inverse"));
        }
    }
}

TEST_CASE("inverse_preserves_exact_zeros_and_representable_subnormals") {
    auto inv = evaluate("identity_transform().inverse()").as<cel::euclid::transform2d>();
    CHECK_FALSE(std::signbit(inv.value[0][1]));
    CHECK_FALSE(std::signbit(inv.value[1][0]));
    CHECK(evaluate("scale(0.1, 0.1).inverse().m11").as<double>() == 10.0);
    CHECK(
        evaluate(
            "Transform2D {m11: 1.0, m12: 5e-324, m21: 0.0, m22: 1.0, m31: 0.0, m32: 0.0}.inverse().m12")
            .as<double>() == -std::numeric_limits<double>::denorm_min());
}

TEST_CASE("inverse_rejects_complete_coefficient_underflow") {
    CHECK(evaluation_error(
        "Transform2D {m11: 1e150, m12: 1e-100, m21: 0.0, m22: 1e150, m31: 0.0, m32: 0.0}.inverse()",
        "inverse"));
    CHECK(evaluation_error(
        "Transform2D {m11: 1.0, m12: 0.0, m21: 0.0, m22: 1e-200, m31: 1e-200, m32: 0.0}.inverse()",
        "inverse"));
    for (
        const auto* input :
        {"Transform2D {m11: 1e150, m12: 0.0, m21: 1e-100, m22: 1e150, m31: 0.0, m32: 0.0}",
         "Transform2D {m11: 1.0, m12: 1e150, m21: -1e150, m22: 1e-100, m31: 0.0, m32: 0.0}",
         "Transform2D {m11: 1e-100, m12: 1e150, m21: -1e150, m22: 1.0, m31: 0.0, m32: 0.0}",
         "Transform2D {m11: 1e150, m12: 0.0, m21: 0.0, m22: 1e150, m31: 1e-250, m32: 0.0}",
         "Transform2D {m11: 1e150, m12: 0.0, m21: 0.0, m22: 1e150, m31: 0.0, m32: 1e-250}",
         "Transform2D {m11: 1e-200, m12: 0.0, m21: 0.0, m22: 1.0, m31: 0.0, m32: 1e-200}",
         "Transform2D {m11: 2.0, m12: 0.0, m21: 1.0000000000000002, m22: 1.0, m31: 1.0, m32: 0.9999999999999998}",
         "Transform2D {m11: 1.0, m12: 1.0000000000000002, m21: 0.0, m22: 2.0, m31: 0.9999999999999998, m32: 1.0}"}) {
        CHECK(evaluation_error(std::string("inverse(") + input + ")", "inverse"));
    }
    CHECK(
        evaluate(
            "inverse(Transform2D {m11: 2.0, m12: 0.0, m21: 1e-200, m22: 1e-200, m31: 1e-200, m32: 1e-200}).m31")
            .as<double>() == 0.0);
}

TEST_CASE("ordinary_transform_operations_do_not_gain_finite_checks") {
    CHECK(
        evaluate(
            "Transform2D {m11: 2.0, m12: 1.0, m21: 3.0, m22: 4.0, m31: 1.0 / 0.0, m32: 1.0 / 0.0}.transform_vector(Vector2D {x: 1.0, y: 0.0}).x")
            .as<double>() == 2.0);
    auto rot = evaluate("rotation(radians(0.0))").as<cel::euclid::transform2d>();
    CHECK_FALSE(std::signbit(rot.value[0][1]));
    CHECK_FALSE(std::signbit(rot.value[1][0]));
    const auto rounded =
        evaluate(
            "Transform2D {m11: 1.000000007450580596923828125, m12: 0.0, m21: 1.0, m22: 1.0, m31: 0.0, m32: 0.0}.transform_vector(Vector2D {x: 0.999999992549419403076171875, y: -1.0}).x")
            .as<double>();
    CHECK(rounded == 0.0);
    CHECK_FALSE(std::signbit(rounded));
}

TEST_CASE("checked_outer_bounds_errors") {
    CHECK(evaluation_error(
        "identity_transform().outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: -1.0, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "translation(Vector2D {x: 1e100, y: 0.0}).outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "scale(1e-200, 1.0).outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1e-200, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "identity_transform().outer_transformed_rect(Rect {origin: Point2D {x: 1e100, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "identity_transform().outer_transformed_rect(Rect {origin: Point2D {x: 1e308, y: 0.0}, size: Size2D {width: 1e308, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "scale(1e308, 1.0).outer_transformed_rect(Rect {origin: Point2D {x: 2.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}})",
        "outer_transformed_rect"));
    CHECK(evaluation_error(
        "identity_transform().outer_transformed_rect(Rect {origin: Point2D {x: 0.0 / 0.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}})",
        "outer_transformed_rect"));
    const std::string matrix =
        "Transform2D {m11: 1.0, m12: 2.0, m21: 3.0, m22: 4.0, m31: 5.0, m32: 6.0}";
    const std::string rectangle =
        "Rect {origin: Point2D {x: 1.0, y: 2.0}, size: Size2D {width: 3.0, height: 4.0}}";
    for (const auto* bad : {"(0.0 / 0.0)", "(1.0 / 0.0)", "(-1.0 / 0.0)"}) {
        for (int i = 1; i <= 6; ++i) {
            auto input = matrix;
            auto number = std::to_string(i) + ".0";
            input.replace(input.find(number), number.size(), bad);
            CHECK(evaluation_error("outer_transformed_rect(" + input + ", " + rectangle + ")",
                                   "outer_transformed_rect"));
        }
        for (int i = 1; i <= 4; ++i) {
            auto input = rectangle;
            auto number = std::to_string(i) + ".0";
            input.replace(input.find(number), number.size(), bad);
            CHECK(evaluation_error("outer_transformed_rect(identity_transform(), " + input + ")",
                                   "outer_transformed_rect"));
        }
    }
}

TEST_CASE("outer_bounds_accept_tiny_and_degenerate_geometry") {
    CHECK(
        evaluate(
            "scale(1e-150, 1e-150).outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1e-150, height: 1e-150}}).width")
            .as<double>() == 1e-300);
    CHECK(
        evaluate(
            "scale(0.0, 1.0).outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}}).width")
            .as<double>() == 0.0);
    auto w =
        evaluate(
            "Transform2D {m11: -1.0, m12: 0.0, m21: 0.0, m22: 1.0, m31: -0.0, m32: 0.0}.outer_transformed_rect(Rect {origin: Point2D {x: -0.0, y: -0.0}, size: Size2D {width: 0.0, height: -0.0}}).width")
            .as<double>();
    CHECK(w == 0.0);
    CHECK_FALSE(std::signbit(w));
}

TEST_CASE("rotation_preserves_anchor") {
    const auto value =
        evaluate(
            "translation(Vector2D {x: -3.0, y: -7.0}).then(rotation(degrees(37.0))).then(translation(Vector2D {x: 3.0, y: 7.0})).transform_point(Point2D {x: 3.0, y: 7.0})")
            .as<cel::euclid::point2d>();
    CHECK(value.value.x == doctest::Approx(3.0));
    CHECK(value.value.y == doctest::Approx(7.0));
}

TEST_CASE("inverse_round_trips_points") {
    for (const auto* matrix :
         {"rotation(degrees(37.0))",
          "Transform2D {m11: -2.0, m12: 0.5, m21: 1.25, m22: 3.0, m31: 17.0, m32: -9.0}",
          "scale(1e-150, 1e-150)", "scale(1e150, 1e150)"}) {
        std::string m = matrix;
        auto value = evaluate("(" + m + ").inverse().transform_point((" + m +
                              ").transform_point(Point2D {x: 1.0, y: 0.0}))")
                         .as<cel::euclid::point2d>();
        CHECK(value.value.x == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(std::abs(value.value.y) <= 1e-12);
    }
}

TEST_CASE("inverse_crop_bounds_enclose_all_corners") {
    for (const auto* matrix :
         {"rotation(degrees(37.0))",
          "Transform2D {m11: -2.0, m12: 0.5, m21: 1.25, m22: 3.0, m31: 17.0, m32: -9.0}",
          "Transform2D {m11: 1.0, m12: 0.0, m21: 2.0, m22: 1.0, m31: -3.0, m32: 7.0}"}) {
        const std::string inverse = "(" + std::string(matrix) + ").inverse()";
        const auto bounds =
            evaluate(
                inverse +
                ".outer_transformed_rect(Rect {origin: Point2D {x: -7.0, y: 3.0}, size: Size2D {width: 13.0, height: 11.0}})")
                .as<cel::euclid::rect>();
        for (const auto* corner : {"Point2D {x: -7.0, y: 3.0}", "Point2D {x: 6.0, y: 3.0}",
                                   "Point2D {x: -7.0, y: 14.0}", "Point2D {x: 6.0, y: 14.0}"}) {
            const auto p =
                evaluate(inverse + ".transform_point(" + corner + ")").as<cel::euclid::point2d>();
            CHECK(p.value.x >= bounds.origin.value.x);
            CHECK(p.value.x <= bounds.origin.value.x + bounds.size.value.x);
            CHECK(p.value.y >= bounds.origin.value.y);
            CHECK(p.value.y <= bounds.origin.value.y + bounds.size.value.y);
        }
    }
}

TEST_CASE("transformed_image_bounds_are_not_an_interior_test") {
    const std::string matrix = "rotation(degrees(45.0))";
    const auto b =
        evaluate(
            matrix +
            ".outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 100.0, height: 100.0}})")
            .as<cel::euclid::rect>();
    CHECK(b.size.value.x > 100.0);
    const auto p =
        evaluate(
            matrix + ".inverse().transform_point(" + matrix +
            ".outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 100.0, height: 100.0}}).origin + Vector2D {x: 1.0, y: 1.0})")
            .as<cel::euclid::point2d>();
    CHECK((p.value.x < 0.0 || p.value.y < 0.0 || p.value.x >= 100.0 || p.value.y >= 100.0));
}

TEST_CASE("outer_bounds_round_outward_when_extent_subtraction_rounds_inward") {
    const std::string m =
        "Transform2D {m11: -2.8245766933675043, m12: 7.683856543964339, m21: 9.154624079279824, m22: -6.981581884177821, m31: -6.475645430192594, m32: -5.360862663609285}";
    auto b =
        evaluate(
            m +
            ".outer_transformed_rect(Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}})")
            .as<cel::euclid::rect>();
    for (const auto* c : {"Point2D {x: 0.0, y: 0.0}", "Point2D {x: 1.0, y: 0.0}",
                          "Point2D {x: 0.0, y: 1.0}", "Point2D {x: 1.0, y: 1.0}"}) {
        auto p = evaluate(m + ".transform_point(" + c + ")").as<cel::euclid::point2d>();
        CHECK(p.value.x >= b.origin.value.x);
        CHECK(p.value.x <= b.origin.value.x + b.size.value.x);
        CHECK(p.value.y >= b.origin.value.y);
        CHECK(p.value.y <= b.origin.value.y + b.size.value.y);
    }
}

TEST_CASE("outer_bounds_reject_overflowing_extents_between_finite_corners") {
    CHECK(evaluation_error(
        "scale(2.0, 1.0).outer_transformed_rect(Rect {origin: Point2D {x: -8.988465674311579e307, y: 0.0}, size: Size2D {width: 1.7976931348623157e308, height: 1.0}})",
        "outer_transformed_rect"));
}

TEST_CASE("remaining_transform_signatures_support_both_call_forms") {
    struct signature {
        std::string name, args, receiver, expected;
    };
    const std::string matrix =
        "Transform2D {m11: 2.0, m12: 1.0, m21: 3.0, m22: 4.0, m31: 5.0, m32: 6.0}";
    const std::string p = "Point2D {x: 1.0, y: 0.0}";
    const std::string v = "Vector2D {x: 10.0, y: 0.0}";
    const std::string r =
        "Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 1.0, height: 1.0}}";
    const std::vector<signature> cases{
        {"m11", matrix, matrix, "2.0"},
        {"m12", matrix, matrix, "1.0"},
        {"m21", matrix, matrix, "3.0"},
        {"m22", matrix, matrix, "4.0"},
        {"m31", matrix, matrix, "5.0"},
        {"m32", matrix, matrix, "6.0"},
        {"translation", v, v,
         "Transform2D {m11: 1.0, m12: 0.0, m21: 0.0, m22: 1.0, m31: 10.0, m32: 0.0}"},
        {"scale", "2.0, 3.0", "2.0, 3.0",
         "Transform2D {m11: 2.0, m12: 0.0, m21: 0.0, m22: 3.0, m31: 0.0, m32: 0.0}"},
        {"rotation", "radians(0.0)", "radians(0.0)", "identity_transform()"},
        {"then", matrix + ", identity_transform()", matrix + ", identity_transform()", matrix},
        {"determinant", matrix, matrix, "5.0"},
        {"inverse", "identity_transform()", "identity_transform()", "identity_transform()"},
        {"transform_point", matrix + ", " + p, matrix + ", " + p, "Point2D {x: 7.0, y: 7.0}"},
        {"transform_vector", matrix + ", " + v, matrix + ", " + v, "Vector2D {x: 20.0, y: 10.0}"},
        {"outer_transformed_rect", "identity_transform(), " + r, "identity_transform(), " + r, r},
        {"normalize", "Vector2D {x: 3.0, y: 4.0}", "Vector2D {x: 3.0, y: 4.0}",
         "Vector2D {x: 0.6, y: 0.8}"}};
    cel::op_lookup lookup;
    cel::stdlib::install(lookup);
    cel::euclid::install(lookup);
    cel::parser parser(std::move(lookup));
    for (const auto& c : cases) {
        std::size_t split = 0;
        int depth = 0;
        for (; split < c.receiver.size(); ++split) {
            const char ch = c.receiver[split];
            if (ch == '(' || ch == '{') ++depth;
            if (ch == ')' || ch == '}') --depth;
            if (ch == ',' && depth == 0) break;
        }
        const auto tail = split < c.receiver.size() ? c.receiver.substr(split + 2) : "";
        const auto receiver = "(" + c.receiver.substr(0, split) + ")." + c.name + "(" + tail + ")";
        CAPTURE(c.name);
        CHECK(evaluate(c.name + "(" + c.args + ") == (" + c.expected + ")").as<bool>());
        CHECK(evaluate(receiver + " == (" + c.expected + ")").as<bool>());
        const auto first = c.receiver.substr(0, split);
        const auto wrong_first = std::string("true") + (tail.empty() ? "" : ", " + tail);
        CHECK_THROWS_AS(parser.parse_str(c.name + "(" + wrong_first + ")"), cel::parse_error);
        CHECK_THROWS_AS(parser.parse_str("(true)." + c.name + "(" + tail + ")"), cel::parse_error);
        if (!tail.empty()) {
            CHECK_THROWS_AS(parser.parse_str(c.name + "(" + first + ", true)"), cel::parse_error);
            CHECK_THROWS_AS(parser.parse_str("(" + first + ")." + c.name + "(true)"),
                            cel::parse_error);
        }
    }
    CHECK(
        evaluate(
            "identity_transform() == Transform2D {m11: 1.0, m12: 0.0, m21: 0.0, m22: 1.0, m31: 0.0, m32: 0.0}")
            .as<bool>());
    CHECK_THROWS_AS(parser.parse_str("identity_transform(true)"), cel::parse_error);
}
