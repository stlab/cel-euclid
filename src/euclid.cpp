// SPDX-License-Identifier: BSL-1.0
#include <cel/euclid.hpp>
#include <cel/parser/lookup.hpp>

#include "registration.hpp"

namespace cel::euclid {
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
