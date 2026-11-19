---
title: "Transformed outer bounds round outward around mapped corners"
entity_type: discovery
confidence: 0.80
created: 2026-11-18T17:09:53Z
last_accessed: 2026-11-18T17:09:53Z
last_reinforced: 2026-11-18T17:09:53Z
access_count: 1
sources:
  - "src/transforms.cpp:141"
  - "src/transforms.cpp:148"
  - "src/transforms.cpp:157"
  - "tests/transforms_test.cpp:184"
  - "tests/transforms_test.cpp:247"
  - "tests/transforms_test.cpp:263"
  - "docs/reference.md:109"
relationships:
  - type: extends
    target: "Checked operations preserve evaluation source locations"
  - type: relates_to
    target: "Affine transform storage and composition follow Euclid order"
tags: ["outer-transformed-rect", "bounds", "rounding", "representability", "transform2d"]
tier: knowledge
source: inferred
---

# Transformed outer bounds round outward around mapped corners

On `cel-euclid-cpp-port` at `c7b30fa`, `outer_transformed_rect` bounds the four mapped corners; its axis-aligned result is not an interior test for the transformed shape. Its `extent` helper advances the width or height by one `nextafter` step when recomputing the endpoint would round inward. The checked path rejects unrepresentable endpoints, edges, spans, and bounds, but zero-size rectangles and singular transforms can be valid. Preserve the corner-enclosure and outward-rounding tests when changing this calculation; see `src/transforms.cpp:148` and `tests/transforms_test.cpp:263`.
