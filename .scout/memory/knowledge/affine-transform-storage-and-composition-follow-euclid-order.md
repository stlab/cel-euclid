---
title: "Affine transform storage and composition follow Euclid order"
entity_type: concept
confidence: 0.80
created: 2026-11-18T17:09:22Z
last_accessed: 2026-11-18T17:09:22Z
last_reinforced: 2026-11-18T17:09:22Z
access_count: 1
sources:
  - "include/cel/euclid.hpp:55"
  - "src/records.hpp:77"
  - "src/transforms.cpp:58"
  - "src/transforms.cpp:64"
  - "src/transforms.cpp:69"
  - "tests/transforms_test.cpp:38"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["cel-euclid-transform2d", "transform2d", "glm-dmat3x2", "affine-transform", "composition"]
tier: knowledge
source: inferred
---

# Affine transform storage and composition follow Euclid order

On `cel-euclid-cpp-port` at `c7b30fa`, `Transform2D` stores the GLM `dmat3x2` columns `(m11,m12)`, `(m21,m22)`, `(m31,m32)`. Points map to `(m11*x + m21*y + m31, m12*x + m22*y + m32)`; vectors omit translation. `a.then(b)` applies `a` before `b`, as the translation/scale tests demonstrate. Preserve this order and the explicit coefficient mapping when changing matrix code rather than assuming GLM's multiplication order matches the CEL contract. See `src/transforms.cpp:58` and `tests/transforms_test.cpp:38`.

Aliases / also known as: cel::euclid::transform2d, Transform2D, transform2d, affine transform
