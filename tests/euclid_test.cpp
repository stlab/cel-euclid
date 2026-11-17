// SPDX-License-Identifier: BSL-1.0
#include <array>
#include <cel/euclid.hpp>
#include <cel/parser/compiler.hpp>
#include <cel/std.hpp>
#include <cmath>
#include <string>
#include <string_view>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

namespace {
/// Compiles and executes an expression with the given lookup.
cel::value evaluate(cel::op_lookup lookup, std::string_view expression) {
    cel::parser parser(std::move(lookup));
    cel::raw_stack stack;
    return parser.parse_str(expression).execute(stack, {});
}

/// Installs geometry and optionally the scalar library first.
cel::op_lookup installed(bool scalar_first = false) {
    cel::op_lookup lookup;
    if (scalar_first) cel::stdlib::install(lookup);
    cel::euclid::install(lookup);
    if (!scalar_first) cel::stdlib::install(lookup);
    return lookup;
}

struct mark_function {};
} // namespace

TEST_CASE("record_schemas_match_rust") {
    cel::op_lookup lookup;
    cel::euclid::install(lookup);
    const std::array schemas{
        std::pair{"Point2D", std::vector<std::string>{"x", "y"}},
        std::pair{"Vector2D", std::vector<std::string>{"x", "y"}},
        std::pair{"Size2D", std::vector<std::string>{"width", "height"}},
        std::pair{"Rect", std::vector<std::string>{"origin", "size"}},
        std::pair{"Box2D", std::vector<std::string>{"min", "max"}},
        std::pair{"Angle", std::vector<std::string>{"radians"}},
        std::pair{"Transform2D",
                  std::vector<std::string>{"m11", "m12", "m21", "m22", "m31", "m32"}}};
    for (const auto& [name, fields] : schemas) {
        const auto schema = lookup.find_type(name);
        REQUIRE(schema.has_value());
        CHECK(std::vector<std::string>(schema->field_names().begin(),
                                       schema->field_names().end()) == fields);
    }
}

TEST_CASE("reordered_fields_preserve_named_values") {
    std::vector<double> calls;
    auto lookup = installed();
    lookup.register_op<double, double>("mark", [&calls](double x) {
        calls.push_back(x);
        return x;
    });
    lookup.register_function("mark", mark_function{});
    auto result =
        evaluate(std::move(lookup), "Rect { size: Size2D { height: mark(4.0), width: mark(3.0) }, "
                                    "origin: Point2D { y: mark(2.0), x: mark(1.0) } }");
    CHECK((calls == std::vector<double>{4, 3, 2, 1}));
    CHECK(result.field("origin").field("x").as<double>() == 1.0);
    CHECK(result.field("origin").field("y").as<double>() == 2.0);
    CHECK(result.field("size").field("width").as<double>() == 3.0);
    CHECK(result.field("size").field("height").as<double>() == 4.0);
    auto box = evaluate(
        installed(), "Box2D { max: Point2D { y: 4.0, x: 3.0 }, min: Point2D { y: 2.0, x: 1.0 } }");
    CHECK(box.field("max").field("x").as<double>() == 3.0);
    CHECK(box.field("min").field("y").as<double>() == 2.0);
}

TEST_CASE("point_vector_and_size_arithmetic_match_rust") {
    auto result = evaluate(installed(), "Point2D {x: 1.0, y: 2.0} + Vector2D {x: 3.0, y: 4.0}");
    CHECK(result.field("x").as<double>() == 4.0);
    CHECK(result.field("y").as<double>() == 6.0);
    CHECK(evaluate(installed(), "Vector2D {x: 3.0, y: 4.0}.length()").as<double>() == 5.0);
    CHECK(evaluate(installed(), "(Size2D {width: 2.0, height: 3.0} * 2.0).height").as<double>() ==
          6.0);
}

TEST_CASE("rect_and_box_boundaries_match_rust") {
    auto lookup = installed();
    cel::parser parser(std::move(lookup));
    cel::raw_stack stack;
    CHECK(
        parser
            .parse_str("Rect {origin: Point2D {x: 1.0, y: 2.0}, "
                       "size: Size2D {width: 4.0, height: 6.0}}.contains(Point2D {x: 1.0, y: 2.0})")
            .execute(stack, {})
            .as<bool>());
    CHECK_FALSE(
        parser
            .parse_str("Rect {origin: Point2D {x: 1.0, y: 2.0}, "
                       "size: Size2D {width: 4.0, height: 6.0}}.contains(Point2D {x: 5.0, y: 2.0})")
            .execute(stack, {})
            .as<bool>());
}

TEST_CASE("geometry_min_max_coexist_with_scalar_min_max") {
    for (bool scalar_first : {false, true}) {
        CHECK(evaluate(installed(scalar_first), "min(3.0, 5.0)").as<double>() == 3.0);
        CHECK(evaluate(installed(scalar_first), "max(3.0, 5.0)").as<double>() == 5.0);
        CHECK(evaluate(installed(scalar_first), "Box2D {min: Point2D {x: 1.0, y: 2.0}, "
                                                "max: Point2D {x: 5.0, y: 6.0}}.min().x")
                  .as<double>() == 1.0);
        CHECK(evaluate(installed(scalar_first), "Box2D {min: Point2D {x: 1.0, y: 2.0}, "
                                                "max: Point2D {x: 5.0, y: 6.0}}.max.y")
                  .as<double>() == 6.0);
    }
}

TEST_CASE("f64 suffix and identity casts work while integer values and casts do not") {
    cel::parser parser{cel::op_lookup{}};
    cel::raw_stack stack;
    CHECK_NOTHROW(parser.parse_str("1f64"));
    CHECK(parser.parse_str("1f64").execute(stack, {}).as<double>() == 1.0);
    CHECK_THROWS_AS(parser.parse_str("1"), cel::parse_error);
    CHECK_THROWS_AS(parser.parse_str("1 as f64"), cel::parse_error);
    CHECK(parser.parse_str("1.0 as f64").execute(stack, {}).as<double>() == 1.0);
    CHECK(parser.parse_str("true as bool").execute(stack, {}).as<bool>());
    CHECK_THROWS_AS(parser.parse_str("1.0 as i32"), cel::parse_error);
    CHECK_THROWS_AS(parser.parse_str("true as f64"), cel::parse_error);
}

TEST_CASE("native adapters round-trip canonical named records") {
    using namespace cel::euclid;
    const auto point = cel::value::from(point2d{{1.0, 2.0}});
    CHECK(point.field("x").as<double>() == 1.0);
    CHECK(point.as<point2d>().value.y == 2.0);
    CHECK(cel::value::from(vector2d{{3.0, 4.0}}).as<vector2d>().value.x == 3.0);
    CHECK(cel::value::from(size2d{{5.0, 6.0}}).as<size2d>().value.y == 6.0);
    const auto rectangle = cel::value::from(rect{{{1.0, 2.0}}, {{3.0, 4.0}}});
    CHECK(rectangle.as<rect>().origin.value.y == 2.0);
    CHECK(rectangle.as<rect>().size.value.x == 3.0);
    const auto bounds = cel::value::from(box2d{{{1.0, 2.0}}, {{3.0, 4.0}}});
    CHECK(bounds.as<box2d>().max.value.y == 4.0);
    CHECK(cel::value::from(angle{1.5}).as<angle>().radians == 1.5);
    const transform2d matrix{glm::dmat3x2{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}};
    const auto encoded = cel::value::from(matrix);
    CHECK(encoded.field("m31").as<double>() == 5.0);
    CHECK(encoded.as<transform2d>().value[2][1] == 6.0);
    CHECK_THROWS_AS(cel::record_adapter<vector2d>::from_value(point), std::invalid_argument);
}

TEST_CASE("all non-transform non-checked signatures match representative values") {
    const std::string p = "Point2D {x: 1.0, y: 2.0}";
    const std::string q = "Point2D {x: 4.0, y: 6.0}";
    const std::string v = "Vector2D {x: 3.0, y: 4.0}";
    const std::string s = "Size2D {width: 4.0, height: 6.0}";
    const std::string r = "Rect {origin: " + p + ", size: " + s + "}";
    const std::string b = "Box2D {min: " + p + ", max: Point2D {x: 5.0, y: 8.0}}";
    struct signature {
        std::string name;
        std::vector<std::string> arguments;
        std::string expected;
    };
    const std::vector<signature> cases{
        {"x", {p}, "1.0"},
        {"y", {p}, "2.0"},
        {"x", {v}, "3.0"},
        {"y", {v}, "4.0"},
        {"width", {s}, "4.0"},
        {"height", {s}, "6.0"},
        {"width", {r}, "4.0"},
        {"height", {r}, "6.0"},
        {"width", {b}, "4.0"},
        {"height", {b}, "6.0"},
        {"origin", {r}, p},
        {"size", {r}, s},
        {"size", {b}, s},
        {"min", {b}, p},
        {"max", {b}, "Point2D {x: 5.0, y: 8.0}"},
        {"min_x", {r}, "1.0"},
        {"min_y", {r}, "2.0"},
        {"max_x", {r}, "5.0"},
        {"max_y", {r}, "8.0"},
        {"min_x", {b}, "1.0"},
        {"min_y", {b}, "2.0"},
        {"max_x", {b}, "5.0"},
        {"max_y", {b}, "8.0"},
        {"center", {r}, "Point2D {x: 3.0, y: 5.0}"},
        {"center", {b}, "Point2D {x: 3.0, y: 5.0}"},
        {"dot", {v, v}, "25.0"},
        {"cross", {v, v}, "0.0"},
        {"square_length", {v}, "25.0"},
        {"length", {v}, "5.0"},
        {"distance_to", {p, q}, "5.0"},
        {"translate", {r, v}, "Rect {origin: " + q + ", size: " + s + "}"},
        {"translate", {b, v}, "Box2D {min: " + q + ", max: Point2D {x: 8.0, y: 12.0}}"},
        {"inflate",
         {r, "1.0", "2.0"},
         "Rect {origin: Point2D {x: 0.0, y: 0.0}, size: Size2D {width: 6.0, height: 10.0}}"},
        {"inflate",
         {b, "1.0", "2.0"},
         "Box2D {min: Point2D {x: 0.0, y: 0.0}, max: Point2D {x: 6.0, y: 10.0}}"},
        {"to_box2d", {r}, b},
        {"to_rect", {b}, r},
        {"contains", {r, p}, "true"},
        {"contains", {b, p}, "true"},
        {"contains_rect", {r, r}, "true"},
        {"contains_box", {b, b}, "true"},
        {"intersects", {r, r}, "true"},
        {"intersects", {b, b}, "true"},
        {"union", {r, r}, r},
        {"union", {b, b}, b},
        {"degrees", {"0.0"}, "Angle {radians: 0.0}"},
        {"degrees", {"Angle {radians: 0.0}"}, "0.0"},
        {"radians", {"1.5"}, "Angle {radians: 1.5}"},
        {"radians", {"Angle {radians: 1.5}"}, "1.5"}};
    cel::parser parser(installed());
    cel::raw_stack stack;
    for (const auto& c : cases) {
        std::string tail;
        for (std::size_t i = 1; i < c.arguments.size(); ++i)
            tail += ", " + c.arguments[i];
        const auto ordinary = c.name + "(" + c.arguments[0] + tail + ")";
        const auto receiver =
            "(" + c.arguments[0] + ")." + c.name + "(" + (tail.empty() ? "" : tail.substr(2)) + ")";
        CAPTURE(ordinary);
        CAPTURE(receiver);
        CHECK(parser.parse_str("(" + ordinary + ") == (" + c.expected + ")")
                  .execute(stack, {})
                  .as<bool>());
        CHECK(parser.parse_str("(" + receiver + ") == (" + c.expected + ")")
                  .execute(stack, {})
                  .as<bool>());
        for (std::size_t i = 0; i < c.arguments.size(); ++i) {
            std::string wrong = c.name + "(";
            for (std::size_t j = 0; j < c.arguments.size(); ++j) {
                if (j) wrong += ", ";
                wrong += i == j ? "true" : c.arguments[j];
            }
            CHECK_THROWS_AS(parser.parse_str(wrong + ")"), cel::parse_error);
        }
        CHECK_THROWS_AS(parser.parse_str(c.name + "()"), cel::parse_error);
    }

    for (const auto& expression : std::vector<std::pair<std::string, std::string>>{
             {p + " + " + v, q},
             {v + " + " + v, "Vector2D {x: 6.0, y: 8.0}"},
             {s + " + " + s, "Size2D {width: 8.0, height: 12.0}"},
             {q + " - " + p, v},
             {q + " - " + v, p},
             {v + " - " + v, "Vector2D {x: 0.0, y: 0.0}"},
             {s + " - " + s, "Size2D {width: 0.0, height: 0.0}"},
             {"-(" + v + ")", "Vector2D {x: -3.0, y: -4.0}"},
             {v + " * 2.0", "Vector2D {x: 6.0, y: 8.0}"},
             {v + " / 2.0", "Vector2D {x: 1.5, y: 2.0}"},
             {s + " * 2.0", "Size2D {width: 8.0, height: 12.0}"},
             {s + " / 2.0", "Size2D {width: 2.0, height: 3.0}"}}) {
        CAPTURE(expression.first);
        CHECK(parser.parse_str("(" + expression.first + ") == (" + expression.second + ")")
                  .execute(stack, {})
                  .as<bool>());
    }
    for (const auto& bad : {p + " + " + p, v + " + " + p, "2.0 * " + v, v + " / 2", "-(" + s + ")",
                            v + " - " + p, s + " + " + v})
        CHECK_THROWS_AS(parser.parse_str(bad), cel::parse_error);
}

TEST_CASE("angle conversions preserve radians and degrees") {
    CHECK(evaluate(installed(), "radians(degrees(180.0))").as<double>() ==
          doctest::Approx(3.141592653589793).epsilon(1e-15));
    CHECK(evaluate(installed(), "degrees(radians(3.141592653589793))").as<double>() ==
          doctest::Approx(180.0).epsilon(1e-15));
    CHECK(std::signbit(evaluate(installed(), "radians(degrees(-0.0))").as<double>()));
    CHECK(std::isinf(evaluate(installed(), "degrees(radians(1.0 / 0.0))").as<double>()));
}

TEST_CASE("equality and finite inspect every nominal record component") {
    const std::vector<std::pair<std::string, int>> records{
        {"Point2D {x: 1.0, y: 2.0}", 2},
        {"Vector2D {x: 1.0, y: 2.0}", 2},
        {"Size2D {width: 1.0, height: 2.0}", 2},
        {"Rect {origin: Point2D {x: 1.0, y: 2.0}, size: Size2D {width: 3.0, height: 4.0}}", 4},
        {"Box2D {min: Point2D {x: 1.0, y: 2.0}, max: Point2D {x: 3.0, y: 4.0}}", 4},
        {"Angle {radians: 1.0}", 1},
        {"Transform2D {m11: 1.0, m12: 2.0, m21: 3.0, m22: 4.0, m31: 5.0, m32: 6.0}", 6}};
    cel::parser parser(installed());
    cel::raw_stack stack;
    CHECK_THROWS_AS(parser.parse_str("is_finite(true)"), cel::parse_error);
    for (const auto& [record, count] : records) {
        CAPTURE(record);
        CHECK(parser.parse_str(record + " == " + record).execute(stack, {}).as<bool>());
        CHECK_FALSE(parser.parse_str(record + " != " + record).execute(stack, {}).as<bool>());
        CHECK(parser.parse_str("is_finite(" + record + ")").execute(stack, {}).as<bool>());
        CHECK_THROWS_AS(parser.parse_str(record + " == true"), cel::parse_error);
        for (int i = 1; i <= count; ++i) {
            auto nonfinite = record;
            const auto part = std::to_string(i) + ".0";
            nonfinite.replace(nonfinite.find(part), part.size(), "(0.0 / 0.0)");
            CAPTURE(nonfinite);
            CHECK_FALSE(
                parser.parse_str("(" + nonfinite + ").is_finite()").execute(stack, {}).as<bool>());
            CHECK_FALSE(
                parser.parse_str(nonfinite + " == " + nonfinite).execute(stack, {}).as<bool>());
            CHECK(parser.parse_str(nonfinite + " != " + nonfinite).execute(stack, {}).as<bool>());
        }
    }
}

TEST_CASE("ordinary bounds retain reversed empty nonfinite and signed-zero behavior") {
    const std::string good = "Box2D {min: Point2D {x: 0.0, y: 0.0}, max: Point2D {x: 2.0, y: 2.0}}";
    const std::string reversed =
        "Box2D {min: Point2D {x: 3.0, y: 0.0}, max: Point2D {x: 0.0, y: 2.0}}";
    const std::string wide =
        "Box2D {min: Point2D {x: -1.0, y: -1.0}, max: Point2D {x: 4.0, y: 4.0}}";
    const std::string nanbox =
        "Box2D {min: Point2D {x: 0.0 / 0.0, y: 0.0}, max: Point2D {x: 1.0, y: 1.0}}";
    cel::parser parser(installed());
    cel::raw_stack stack;
    /// Euclid tests endpoint comparisons even when one operand is reversed.
    CHECK(parser.parse_str("intersects(" + reversed + ", " + wide + ")")
              .execute(stack, {})
              .as<bool>());
    CHECK(parser.parse_str("contains_box(" + good + ", " + reversed + ")")
              .execute(stack, {})
              .as<bool>());
    CHECK(parser.parse_str("union(" + good + ", " + nanbox + ") == " + good)
              .execute(stack, {})
              .as<bool>());
    CHECK(parser.parse_str("union(" + nanbox + ", " + good + ") == " + good)
              .execute(stack, {})
              .as<bool>());
    CHECK(
        parser
            .parse_str(
                "is_finite(Rect {origin: Point2D {x: 1e308, y: 0.0}, size: Size2D {width: 1e308, height: 1.0}})")
            .execute(stack, {})
            .as<bool>());
    CHECK(std::isinf(
        parser
            .parse_str(
                "max_x(Rect {origin: Point2D {x: 1e308, y: 0.0}, size: Size2D {width: 1e308, height: 1.0}})")
            .execute(stack, {})
            .as<double>()));
    const auto joined =
        parser
            .parse_str(
                "union(Box2D {min: Point2D {x: -0.0, y: 0.0}, max: Point2D {x: 2.0, y: 2.0}}, "
                "Box2D {min: Point2D {x: 0.0, y: 0.0}, max: Point2D {x: 3.0, y: 3.0}})")
            .execute(stack, {});
    CHECK(std::signbit(joined.field("min").field("x").as<double>()));
    CHECK_FALSE(
        parser
            .parse_str(
                "contains(Rect {origin: Point2D {x: 1.0, y: 1.0}, size: Size2D {width: -2.0, height: 1.0}}, Point2D {x: 0.0, y: 1.0})")
            .execute(stack, {})
            .as<bool>());
    CHECK(
        parser
            .parse_str(
                "contains_rect(Rect {origin: Point2D {x: 1.0, y: 1.0}, size: Size2D {width: 1.0, height: 1.0}}, Rect {origin: Point2D {x: 9.0, y: 9.0}, size: Size2D {width: 0.0, height: 1.0}})")
            .execute(stack, {})
            .as<bool>());
}
