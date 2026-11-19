---
title: "Nominal record adapters bridge native carriers and CEL"
entity_type: pattern
confidence: 0.80
created: 2026-11-18T17:08:26Z
last_accessed: 2026-11-18T17:08:26Z
last_reinforced: 2026-11-18T17:08:26Z
access_count: 1
sources:
  - "include/cel/euclid.hpp:77"
  - "src/records.hpp:14"
  - "src/records.hpp:20"
  - "src/records.hpp:39"
  - "src/euclid.cpp:11"
  - "tests/euclid_test.cpp:34"
  - "tests/euclid_test.cpp:157"
relationships:
  - type: extends
    target: "Architecture Overview"
tags: ["record-adapter", "nominal-record", "schema", "parser", "runtime"]
tier: knowledge
source: inferred
---

# Nominal record adapters bridge native carriers and CEL

On `cel-euclid-cpp-port` at `c7b30fa`, a geometry carrier is a nominal CEL record, not just its underlying GLM storage. Adding one requires a public `record_adapter<T>` declaration, a named descriptor and both conversion functions in `src/records.hpp`, registration of the CEL name in `cel::euclid::install`, and schema/round-trip tests. `from_value` checks compatibility with the complete descriptor before decoding, so structurally similar records such as points and vectors are not interchangeable. See `src/records.hpp:14` and `src/euclid.cpp:11`.
