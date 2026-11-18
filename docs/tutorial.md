# Evaluate geometry expressions {#tutorial}

This tutorial shows C++ developers how to install geometry operations, compile
CEL expressions, execute them, and handle checked evaluation failures.

## Before you begin

Use C++20, a binary64 `double`, CMake 3.24 or later, and the dependencies
declared in `CMakeLists.txt`. Build and install the library as described in the
[README](../README.md). Link your executable against `cel::euclid`:

```cmake
find_package(cel-euclid CONFIG REQUIRED)
target_link_libraries(YOUR_TARGET PRIVATE cel::euclid)
```

Replace `YOUR_TARGET` with your executable target. Set `CMAKE_PREFIX_PATH` to
the installation prefix when configuring the consuming project. The parser
and runtime are transitive dependencies; no separate runtime link is needed.

## Compile and execute expressions

1. Include `<cel/parser.hpp>` and `<cel/euclid.hpp>`.
2. Call `cel::euclid::install` on a lookup before moving it into a parser.
   If you also install `cel-std`, either installation order preserves the
   scalar and geometry overloads.
3. Compile a CEL expression with `parse_str`, prepare the result type, and
   call it with a caller-owned `cel::raw_stack`.
4. Catch evaluation errors at a boundary that can handle them.

The following program, also in `examples/example.cpp`, is built and executed
by the test suite:

```cpp
#include <cel/euclid.hpp>
#include <cel/parser.hpp>
#include <iostream>
#include <stdexcept>
#include <utility>

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
```

The output starts with:

```text
center x: 3
mapped x: 22
checked error: normalize at bytes ...
```

The checked error also includes its source byte range and reason. Record
fields are named and may appear in any order; each required field must appear
exactly once. The receiver call `rect.center()` is equivalent to
`center(rect)`. A unary named operation also accepts `rect.center` without
parentheses. `translation(...).then(scale(...))` applies translation first.

CEL values are `bool` and `f64`, with no implicit numeric casts. Write floating
literals such as `1.0`, `1f64`, or `1e2`. An integer literal such as `1`,
including `1 as f64`, is unsupported. An identity cast such as `1.0 as f64`
is supported.

The parser rejects invalid expressions with `cel::parse_error`. The checked
`normalize`, `inverse`, and `outer_transformed_rect` operations throw
source-located `std::runtime_error` during evaluation when their inputs or
results are unrepresentable. Other floating-point operations can produce
infinities or NaNs. See the [reference](reference.md) for the exact conditions.
