// SPDX-License-Identifier: BSL-1.0
#include "records.hpp"
#include <cel/parser/lookup.hpp>

namespace cel::euclid {
/// Registers the non-transform geometry operation signatures.
void install_geometry(op_lookup& lookup);
/// Registers affine and checked geometry operation signatures.
void install_transforms(op_lookup& lookup);
void install(op_lookup& lookup) {
    lookup.register_record_type<point2d>("Point2D");
    lookup.register_record_type<vector2d>("Vector2D");
    lookup.register_record_type<size2d>("Size2D");
    lookup.register_record_type<rect>("Rect");
    lookup.register_record_type<box2d>("Box2D");
    lookup.register_record_type<angle>("Angle");
    lookup.register_record_type<transform2d>("Transform2D");
    install_geometry(lookup);
    install_transforms(lookup);
}
} // namespace cel::euclid
