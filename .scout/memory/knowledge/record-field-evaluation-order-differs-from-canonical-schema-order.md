---
title: "Record field evaluation order differs from canonical schema order"
entity_type: observation
confidence: 0.80
created: 2026-11-18T17:08:48Z
last_accessed: 2026-11-18T17:08:48Z
last_reinforced: 2026-11-18T17:08:48Z
access_count: 1
sources:
  - "tests/euclid_test.cpp:53"
  - "src/records.hpp:20"
  - "src/records.hpp:39"
  - "docs/reference.md:23"
relationships:
  - type: relates_to
    target: "Nominal record adapters bridge native carriers and CEL"
tags: ["records", "field-order", "evaluation-order", "parser", "schema"]
tier: knowledge
source: inferred
---

# Record field evaluation order differs from canonical schema order

On `cel-euclid-cpp-port` at `c7b30fa`, record literals accept fields in any order, but their field expressions are evaluated in source order, even across nested records; the resulting values are then associated with the canonical named schema. `tests/euclid_test.cpp:53` records side effects to distinguish source evaluation order from schema order. Preserve both properties when changing parser integration or adapters rather than sorting expressions before evaluating them.
