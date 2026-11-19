---
title: "Keep Euclid tests independent of cel-std"
entity_type: decision
confidence: 0.95
created: 2026-11-19T04:42:46Z
last_accessed: 2026-11-19T04:45:27Z
last_reinforced: 2026-11-19T04:45:27Z
access_count: 2
sources:
  - "CMakeLists.txt:14"
  - "CMakeLists.txt:32"
  - "tests/euclid_test.cpp:110"
  - "tests/transforms_test.cpp:13"
  - "CMakeLists.txt:15"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["cmake", "dependency-tracking", "parser", "cpp-library", "cel-std"]
tier: knowledge
source: written
---

# Keep Euclid tests independent of cel-std

Keep cel-std out of Euclid's build and test dependencies. The parser supplies the scalar operators used in geometry tests, and tests register independent scalar min/max overloads to exercise coexistence in both installation orders. Consumers can install cel-std separately; Euclid's public targets require the parser, runtime, and GLM instead.
