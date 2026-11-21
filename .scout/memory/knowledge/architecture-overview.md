---
title: "Architecture Overview"
entity_type: project
confidence: 0.80
created: 2026-11-18T17:07:48Z
last_accessed: 2026-11-18T17:07:48Z
last_reinforced: 2026-11-18T17:07:48Z
access_count: 1
sources:
  - "include/cel/euclid.hpp:12"
  - "src/records.cpp:19"
  - "src/euclid.cpp:8"
  - "src/geometry.cpp:52"
  - "src/transforms.cpp:175"
tags: ["architecture", "parser", "runtime", "geometry", "glm"]
tier: knowledge
source: inferred
---

# Architecture Overview

This C++ extension uses distinct native carriers for seven nominal CEL geometry records, with GLM vectors and matrices behind most carriers. `cel::euclid::install` registers their record adapters with the parser lookup, then installs ordinary geometry and affine/checked signatures; runtime values cross the boundary through those adapters. Changes to a type or operation must keep its schema, conversion, parser registration, and Rust-reference semantics aligned. Start at `include/cel/euclid.hpp:12`, `src/records.cpp:19`, and `src/euclid.cpp:8`.

`src/registration.hpp` remains a private cross-translation-unit interface: it declares the two installers and shares registration and finiteness helpers between geometry and transforms. The record-decoding helpers in `src/records.cpp` have translation-unit-local linkage in an unnamed namespace; the public `cel::record_adapter` definitions retain their declared identities.
