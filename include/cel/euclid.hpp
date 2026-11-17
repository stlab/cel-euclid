// SPDX-License-Identifier: BSL-1.0
/// \file CEL Euclid nominal geometry values and registrations.
#ifndef CEL_EUCLID_HPP
#define CEL_EUCLID_HPP

#include <cel/runtime/value.hpp>
#include <glm/glm.hpp>

namespace cel {
class op_lookup;
namespace euclid {
/// A point with x and y coordinates. Example: `point2d{{1.0, 2.0}}`.
struct point2d {
    glm::dvec2 value;
};
/// A displacement with x and y components. Example: `vector2d{{3.0, 4.0}}`.
struct vector2d {
    glm::dvec2 value;
};
/// A width and height. Example: `size2d{{3.0, 4.0}}`.
struct size2d {
    glm::dvec2 value;
};
/// An origin and size. Example: `rect{{{1.0, 2.0}}, {{3.0, 4.0}}}`.
struct rect {
    point2d origin;
    size2d size;
};
/// A box with minimum and maximum corners. Example: `box2d{{{0.0, 0.0}}, {{1.0, 1.0}}}`.
struct box2d {
    point2d min;
    point2d max;
};
/// An angle in radians. Example: `angle{1.0}`.
struct angle {
    double radians;
};
/// An affine transform, with columns `(m11,m12)`, `(m21,m22)`, `(m31,m32)`.
/// Example: `transform2d{glm::dmat3x2{1.0}}`.
struct transform2d {
    glm::dmat3x2 value;
};

/// Installs the non-transform geometry schemas and operations into a lookup.
///
/// \pre The lookup has none of the seven record names or installed signatures.
/// Registration failure can leave the lookup partially modified.
/// Expected time is O(s + 1), where s is the number of existing signatures
/// under the registered names.
///
/// Example: `cel::op_lookup lookup; cel::euclid::install(lookup);`.
void install(op_lookup& lookup);
} // namespace euclid

#define CEL_EUCLID_ADAPTER(TYPE)                                                                 \
    /** Provides canonical named-record conversion for a CEL Euclid carrier.                     \
     *                                                                                           \
     * Example using the point adapter:                                                          \
     * \code                                                                                     \
     * using adapter = cel::record_adapter<cel::euclid::point2d>;                                \
     * auto schema = adapter::type();                                                            \
     * auto record = adapter::to_value(cel::euclid::point2d{{1.0, 2.0}});                        \
     * auto point = adapter::from_value(record);                                                 \
     * \endcode                                                                                  \
     */                                                                                          \
    template <>                                                                                  \
    struct record_adapter<euclid::TYPE> {                                                        \
        /** Returns the complete nominal record descriptor. */                                   \
        static value_type type();                                                                \
        /** Returns an independently owned canonical record. */                                  \
        static value to_value(const euclid::TYPE& source);                                       \
        /** Decodes a named record; throws std::invalid_argument for a mismatched descriptor. */ \
        static euclid::TYPE from_value(const value& source);                                     \
    };
CEL_EUCLID_ADAPTER(point2d)
CEL_EUCLID_ADAPTER(vector2d)
CEL_EUCLID_ADAPTER(size2d)
CEL_EUCLID_ADAPTER(rect)
CEL_EUCLID_ADAPTER(box2d)
CEL_EUCLID_ADAPTER(angle)
CEL_EUCLID_ADAPTER(transform2d)
#undef CEL_EUCLID_ADAPTER
} // namespace cel
#endif
