---
title: "Floating-point parity constrains both library and consumers"
entity_type: decision
confidence: 0.80
created: 2026-11-18T17:10:10Z
last_accessed: 2026-11-19T04:45:31Z
last_reinforced: 2026-11-19T04:45:31Z
access_count: 5
sources:
  - "include/cel/euclid.hpp:7"
  - "CMakeLists.txt:26"
  - "CMakeLists.txt:47"
  - "CMakeLists.txt:61"
  - "docs/reference.md:8"
  - "tests/euclid_test.cpp:84"
  - "tests/transforms_test.cpp:127"
  - "CMakeLists.txt:21"
  - "CMakeLists.txt:40"
  - "CMakeLists.txt:49"
  - "tests/euclid_test.cpp:83"
  - "CMakeLists.txt:41"
  - "CMakeLists.txt:46"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["floating-point", "fast-math", "finite-math", "fma", "rust-parity"]
tier: knowledge
source: inferred
---

# Floating-point parity constrains both library and consumers

The library and consumers must preserve NaNs, infinities, and signed zero. The public header rejects compiler-reported fast-math and finite-only modes; individual unsafe flags without detectable macros remain unsupported. The library disables fused contraction (/fp:strict or -ffp-contract=off) so multiply/add rounds separately like Rust, with regression and compile-fail tests guarding the contract.
