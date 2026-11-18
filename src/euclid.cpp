// SPDX-License-Identifier: BSL-1.0
#include "records.hpp"
#include <cel/parser/lookup.hpp>
#include <vector>

namespace cel::euclid {
/// Registers the non-transform geometry operation signatures.
void install_geometry(op_lookup& lookup);
/// Registers affine and checked geometry operation signatures.
void install_transforms(op_lookup& lookup);
namespace {
/// Registers one adapter's complete nominal schema.
template <class T>
void add_record(op_lookup& lookup) {
    const auto type = native_type<T>();
    std::vector<record_field> fields;
    fields.reserve(type.fields().size());
    for (std::size_t i = 0; i != type.fields().size(); ++i)
        fields.push_back({type.field_names()[i], type.fields()[i]});
    lookup.register_record(type.record_identity(), fields);
}
} // namespace
void install(op_lookup& lookup) {
    add_record<point2d>(lookup);
    add_record<vector2d>(lookup);
    add_record<size2d>(lookup);
    add_record<rect>(lookup);
    add_record<box2d>(lookup);
    add_record<angle>(lookup);
    add_record<transform2d>(lookup);
    install_geometry(lookup);
    install_transforms(lookup);
}
} // namespace cel::euclid
