---
title: "Inverse distinguishes exact zero from coefficient underflow"
entity_type: discovery
confidence: 0.80
created: 2026-11-18T17:09:35Z
last_accessed: 2026-11-18T17:09:35Z
last_reinforced: 2026-11-18T17:09:35Z
access_count: 1
sources:
  - "src/transforms.cpp:71"
  - "src/transforms.cpp:81"
  - "src/transforms.cpp:100"
  - "tests/transforms_test.cpp:88"
  - "tests/transforms_test.cpp:102"
  - "docs/reference.md:106"
relationships:
  - type: extends
    target: "Checked operations preserve evaluation source locations"
  - type: relates_to
    target: "Affine transform storage and composition follow Euclid order"
tags: ["inverse", "underflow", "cofactor", "binary64", "transform2d"]
tier: knowledge
source: inferred
---

# Inverse distinguishes exact zero from coefficient underflow

On `cel-euclid-cpp-port` at `c7b30fa`, a finite nonsingular affine matrix is not sufficient for `inverse`: nonfinite inverse coefficients and coefficients rounded completely to zero when their exact cofactors are nonzero are rejected. Exact-zero cofactors and representable nonzero subnormals are allowed. `equal_products` factors finite binary64 operands and cancels significands to compare translation cofactors without rounding the products; retain that distinction if changing inverse arithmetic. See `src/transforms.cpp:81`, `src/transforms.cpp:100`, and the underflow cases at `tests/transforms_test.cpp:88`.
