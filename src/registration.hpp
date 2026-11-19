// SPDX-License-Identifier: BSL-1.0
#ifndef CEL_EUCLID_REGISTRATION_HPP
#define CEL_EUCLID_REGISTRATION_HPP
#include <cel/euclid.hpp>
#include <cel/parser/lookup.hpp>
#include <cmath>
#include <string_view>

namespace cel::euclid {
/// Registers the non-transform geometry operation signatures.
void install_geometry(op_lookup& lookup);
/// Registers affine and checked geometry operation signatures.
void install_transforms(op_lookup& lookup);
} // namespace cel::euclid

namespace cel::euclid::detail {
/// Identifies a named callable.
template <class F>
struct marker {};
/// Registers one exact geometry signature, and also as a named callable if `named`.
template <class R, class... A, class F>
void add(op_lookup& lookup, std::string_view name, F operation, bool named = true) {
    lookup.register_op<R, A...>(name, operation);
    if (named) lookup.register_function(name, marker<F>{});
}
/// Returns whether both components are finite.
inline bool finite(glm::dvec2 v) { return std::isfinite(v.x) && std::isfinite(v.y); }
/// Returns whether every matrix coefficient is finite.
inline bool finite(transform2d t) {
    return finite(t.value[0]) && finite(t.value[1]) && finite(t.value[2]);
}
} // namespace cel::euclid::detail

#endif
