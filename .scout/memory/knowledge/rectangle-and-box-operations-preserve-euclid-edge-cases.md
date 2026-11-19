---
title: "Rectangle and box operations preserve Euclid edge cases"
entity_type: discovery
confidence: 0.80
created: 2026-11-18T17:10:23Z
last_accessed: 2026-11-18T17:11:23Z
last_reinforced: 2026-11-18T17:11:23Z
access_count: 2
sources:
  - "src/geometry.cpp:26"
  - "src/geometry.cpp:28"
  - "src/geometry.cpp:33"
  - "src/geometry.cpp:38"
  - "src/geometry.cpp:43"
  - "tests/euclid_test.cpp:344"
  - "tests/euclid_test.cpp:487"
relationships:
  - type: relates_to
    target: "Floating-point parity constrains both library and consumers"
tags: ["rectangle", "box2d", "containment", "signed-zero", "rust-parity"]
tier: knowledge
source: inferred
---

# Rectangle and box operations preserve Euclid edge cases

On `cel-euclid-cpp-port` at `c7b30fa`, point containment includes minimum edges and excludes maximum edges. A box with non-increasing or unordered endpoints is empty for containment/union, but `intersects` still compares endpoints directly even for reversed boxes. `contains_box` accepts an empty inner box. Box union ignores one empty operand, returns the first when both are empty, and selects the first endpoint on ties, preserving signed-zero signs; rectangle union converts through boxes. Do not normalize endpoints or replace these comparisons with generic GLM helpers without preserving the NaN, reversed-box, and signed-zero cases in `tests/euclid_test.cpp:344` and `tests/euclid_test.cpp:487`.
