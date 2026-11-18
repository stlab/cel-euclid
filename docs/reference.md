# Geometry reference {#reference}

This reference lists the CEL geometry types and all 98 exact supported
signatures. See the [Rust geometry reference](https://stlab.github.io/cel-rs/cel-book/geometry.html)
for individual operation semantics. Operation links in the table point to
their corresponding Rust entries.

## Values and records

The C++ language supports `bool` and binary64 `f64`, not integer values.
Floating literals include `1.0`, `1f64`, and `1e2`; `1` and `1 as f64` are
unsupported. Identity casts such as `1.0 as f64` and `true as bool` work;
there are no implicit numeric conversions.

Records require each field exactly once, in any order. Missing, duplicate,
unknown, or incorrectly typed fields are parse errors. Construction does not
validate finiteness, reorder endpoints, or require nonnegative sizes.

| Record | Fields in schema order |
| --- | --- |
| `Point2D` | `x: f64`, `y: f64` |
| `Vector2D` | `x: f64`, `y: f64` |
| `Size2D` | `width: f64`, `height: f64` |
| `Rect` | `origin: Point2D`, `size: Size2D` |
| `Box2D` | `min: Point2D`, `max: Point2D` |
| `Angle` | `radians: f64` |
| `Transform2D` | `m11: f64`, `m12: f64`, `m21: f64`, `m22: f64`, `m31: f64`, `m32: f64` |

Install `cel::euclid::install` once per lookup before constructing the parser.
`cel-std` and geometry can be installed in either order; their `min` and `max`
overloads coexist. A registration failure can leave the lookup partially
modified. Each named operation with a first argument supports both
`name(value, args...)` and `value.name(args...)`; a unary operation also
supports `value.name`. `identity_transform()` has no receiver.

## Signatures

`T` is shorthand for the explicitly listed alternatives, not CEL generic
syntax. Each operator in a grouped entry has the listed signatures.

| Operation | Operands → result |
| --- | --- |
| [Equality `==`, `!=`](https://stlab.github.io/cel-rs/cel-book/geometry.html#equality--) | `(T, T) → bool`, for each of the seven records |
| [`is_finite`](https://stlab.github.io/cel-rs/cel-book/geometry.html#is_finite) | `(T) → bool`, for each of the seven records |
| [Addition `+`](https://stlab.github.io/cel-rs/cel-book/geometry.html#addition-) | `(Point2D, Vector2D) → Point2D`; `(Vector2D, Vector2D) → Vector2D`; `(Size2D, Size2D) → Size2D` |
| [Subtraction `-`](https://stlab.github.io/cel-rs/cel-book/geometry.html#subtraction--) | `(Point2D, Point2D) → Vector2D`; `(Point2D, Vector2D) → Point2D`; `(Vector2D, Vector2D) → Vector2D`; `(Size2D, Size2D) → Size2D` |
| [Negation `-`](https://stlab.github.io/cel-rs/cel-book/geometry.html#negation-unary--) | `(Vector2D) → Vector2D` |
| [Scaling `*`, `/`](https://stlab.github.io/cel-rs/cel-book/geometry.html#scaling--) | `(Vector2D, f64) → Vector2D`; `(Size2D, f64) → Size2D` |
| [`x`, `y`](https://stlab.github.io/cel-rs/cel-book/geometry.html#x-y) | `(Point2D) → f64`; `(Vector2D) → f64` |
| [`width`, `height`](https://stlab.github.io/cel-rs/cel-book/geometry.html#width-height) | `(Size2D) → f64`; `(Rect) → f64`; `(Box2D) → f64` |
| [`origin`](https://stlab.github.io/cel-rs/cel-book/geometry.html#origin) | `(Rect) → Point2D` |
| [`size`](https://stlab.github.io/cel-rs/cel-book/geometry.html#size) | `(Rect) → Size2D`; `(Box2D) → Size2D` |
| [`min`, `max`](https://stlab.github.io/cel-rs/cel-book/geometry.html#min-max) | `(Box2D) → Point2D` |
| [`min_x`, `min_y`, `max_x`, `max_y`](https://stlab.github.io/cel-rs/cel-book/geometry.html#min_x-min_y-max_x-max_y) | `(Rect) → f64`; `(Box2D) → f64` |
| [`center`](https://stlab.github.io/cel-rs/cel-book/geometry.html#center) | `(Rect) → Point2D`; `(Box2D) → Point2D` |
| [`dot`](https://stlab.github.io/cel-rs/cel-book/geometry.html#dot), [`cross`](https://stlab.github.io/cel-rs/cel-book/geometry.html#cross) | `(Vector2D, Vector2D) → f64` |
| [`square_length`](https://stlab.github.io/cel-rs/cel-book/geometry.html#square_length), [`length`](https://stlab.github.io/cel-rs/cel-book/geometry.html#length) | `(Vector2D) → f64` |
| [`normalize`](https://stlab.github.io/cel-rs/cel-book/geometry.html#normalize) | `(Vector2D) → Vector2D`, checked |
| [`distance_to`](https://stlab.github.io/cel-rs/cel-book/geometry.html#distance_to) | `(Point2D, Point2D) → f64` |
| [`translate`](https://stlab.github.io/cel-rs/cel-book/geometry.html#translate) | `(Rect, Vector2D) → Rect`; `(Box2D, Vector2D) → Box2D` |
| [`inflate`](https://stlab.github.io/cel-rs/cel-book/geometry.html#inflate) | `(Rect, f64, f64) → Rect`; `(Box2D, f64, f64) → Box2D` |
| [`to_box2d`](https://stlab.github.io/cel-rs/cel-book/geometry.html#to_box2d) | `(Rect) → Box2D` |
| [`to_rect`](https://stlab.github.io/cel-rs/cel-book/geometry.html#to_rect) | `(Box2D) → Rect` |
| [`contains`](https://stlab.github.io/cel-rs/cel-book/geometry.html#contains) | `(Rect, Point2D) → bool`; `(Box2D, Point2D) → bool` |
| [`contains_rect`](https://stlab.github.io/cel-rs/cel-book/geometry.html#contains_rect) | `(Rect, Rect) → bool` |
| [`contains_box`](https://stlab.github.io/cel-rs/cel-book/geometry.html#contains_box) | `(Box2D, Box2D) → bool` |
| [`intersects`](https://stlab.github.io/cel-rs/cel-book/geometry.html#intersects) | `(Rect, Rect) → bool`; `(Box2D, Box2D) → bool` |
| [`union`](https://stlab.github.io/cel-rs/cel-book/geometry.html#union) | `(Rect, Rect) → Rect`; `(Box2D, Box2D) → Box2D` |
| [`degrees`](https://stlab.github.io/cel-rs/cel-book/geometry.html#degrees), [`radians`](https://stlab.github.io/cel-rs/cel-book/geometry.html#radians) | `(f64) → Angle`; `(Angle) → f64` |
| [`m11`, `m12`, `m21`, `m22`, `m31`, `m32`](https://stlab.github.io/cel-rs/cel-book/geometry.html#m11-m12-m21-m22-m31-m32) | `(Transform2D) → f64` |
| [`identity_transform`](https://stlab.github.io/cel-rs/cel-book/geometry.html#identity_transform) | `() → Transform2D` |
| [`translation`](https://stlab.github.io/cel-rs/cel-book/geometry.html#translation) | `(Vector2D) → Transform2D` |
| [`scale`](https://stlab.github.io/cel-rs/cel-book/geometry.html#scale) | `(f64, f64) → Transform2D` |
| [`rotation`](https://stlab.github.io/cel-rs/cel-book/geometry.html#rotation) | `(Angle) → Transform2D` |
| [`then`](https://stlab.github.io/cel-rs/cel-book/geometry.html#then) | `(Transform2D, Transform2D) → Transform2D` |
| [`determinant`](https://stlab.github.io/cel-rs/cel-book/geometry.html#determinant) | `(Transform2D) → f64` |
| [`inverse`](https://stlab.github.io/cel-rs/cel-book/geometry.html#inverse) | `(Transform2D) → Transform2D`, checked |
| [`transform_point`](https://stlab.github.io/cel-rs/cel-book/geometry.html#transform_point) | `(Transform2D, Point2D) → Point2D` |
| [`transform_vector`](https://stlab.github.io/cel-rs/cel-book/geometry.html#transform_vector) | `(Transform2D, Vector2D) → Vector2D` |
| [`outer_transformed_rect`](https://stlab.github.io/cel-rs/cel-book/geometry.html#outer_transformed_rect) | `(Transform2D, Rect) → Rect`, checked |

## Geometry and checked failures

The affine transform maps `(x, y)` to
`(m11*x + m21*y + m31, m12*x + m22*y + m32)`. Vectors ignore translation.
`a.then(b)` applies `a` first and `b` second. Ordinary point containment in a
rectangle or box excludes the maximum edges. Ordinary operations preserve
floating-point NaNs, infinities, overflow, and division by zero rather than
turning them into checked errors. `is_finite` checks stored components.

The following checked operations throw a source-located `std::runtime_error`
during evaluation:

- `normalize` rejects zero or nonfinite computed length and nonfinite result
  components. Length computation can overflow or underflow.
- `inverse` rejects nonfinite inputs, a zero or nonfinite determinant,
  nonfinite inverse coefficients, and zero coefficients whose exact cofactors
  are nonzero. Exact-zero cofactors and nonzero subnormals are allowed.
- `outer_transformed_rect` rejects nonfinite inputs, endpoints, or mapped
  corners; negative sizes; positive sizes whose endpoints do not advance;
  overflowing or completely underflowing projected edges; collapsed positive
  projected spans; and unrepresentable bounds. Zero sizes and singular
  transforms are otherwise allowed. The result encloses the four mapped
  corners, advancing an extent by one ULP when needed to avoid rounding
  inward; it does not prove containment inside the transformed shape.
