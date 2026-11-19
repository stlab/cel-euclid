---
title: "Checked operations preserve evaluation source locations"
entity_type: pattern
confidence: 0.80
created: 2026-11-18T17:08:58Z
last_accessed: 2026-11-18T17:08:58Z
last_reinforced: 2026-11-18T17:08:58Z
access_count: 1
sources:
  - "src/transforms.cpp:23"
  - "src/transforms.cpp:175"
  - "docs/reference.md:92"
  - "tests/transforms_test.cpp:21"
  - "tests/transforms_test.cpp:127"
relationships:
  - type: extends
    target: "Register named geometry operations as both signatures and callables"
tags: ["checked-operations", "source-span", "runtime-error", "evaluation", "geometry"]
tier: knowledge
source: inferred
---

# Checked operations preserve evaluation source locations

On `cel-euclid-cpp-port` at `c7b30fa`, `normalize`, `inverse`, and `outer_transformed_rect` use the `checked` registration helper. It captures the parser's source span and translates their `std::domain_error` into an evaluation-time `std::runtime_error` containing the operation name and byte range. Ordinary operations intentionally retain IEEE NaN, infinity, overflow, and division behavior instead of gaining these checks. Use the checked path and test the source-located error when introducing a checked operation; see `src/transforms.cpp:23` and `docs/reference.md:92`.
