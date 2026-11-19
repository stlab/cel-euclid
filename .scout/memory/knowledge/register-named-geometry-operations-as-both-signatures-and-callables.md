---
title: "Register named geometry operations as both signatures and callables"
entity_type: pattern
confidence: 0.80
created: 2026-11-18T17:08:38Z
last_accessed: 2026-11-19T04:43:11Z
last_reinforced: 2026-11-19T04:43:11Z
access_count: 2
sources:
  - "src/geometry.cpp:15"
  - "src/geometry.cpp:52"
  - "src/transforms.cpp:21"
  - "src/transforms.cpp:27"
  - "tests/euclid_test.cpp:111"
  - "tests/euclid_test.cpp:160"
  - "docs/reference.md:40"
  - "tests/euclid_test.cpp:110"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["operation-registration", "parser", "overloads", "callable", "geometry"]
tier: knowledge
source: inferred
---

# Register named geometry operations as both signatures and callables

A named geometry operation needs both its exact typed overload and register_function with its marker; the local add helpers do both. Operator spellings such as + and == register only overloads, while checked operations use register_operation plus register_function to retain the source span. Named operations support function and receiver forms. Tests register independent scalar min and max overloads before and after geometry to check coexistence without depending on cel-std.
