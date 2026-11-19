---
title: "Register named geometry operations as both signatures and callables"
entity_type: pattern
confidence: 0.80
created: 2026-11-18T17:08:38Z
last_accessed: 2026-11-19T04:43:11Z
last_reinforced: 2026-11-19T04:43:11Z
access_count: 2
sources:
  - "src/registration.hpp:26"
  - "src/geometry.cpp:45"
  - "src/transforms.cpp:23"
  - "tests/euclid_test.cpp:110"
  - "docs/reference.md:40"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["operation-registration", "parser", "overloads", "callable", "geometry"]
tier: knowledge
source: inferred
---

# Register named geometry operations as both signatures and callables

A named geometry operation needs both its exact typed overload and register_function with its marker; the shared `detail::add` helper in `src/registration.hpp` does both (pass `named = false` for operator spellings). Operator spellings such as + and == register only overloads, while checked operations use register_operation plus register_function to retain the source span. Markers are keyed on the callable type, and cel-parser throws at install if one marker type names two functions, so same-signature function pointers must be wrapped in lambdas. Named operations support function and receiver forms. Tests register independent scalar min and max overloads before and after geometry to check coexistence without depending on cel-std.
