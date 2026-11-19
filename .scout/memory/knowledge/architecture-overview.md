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
  - "src/records.hpp:14"
  - "src/euclid.cpp:10"
  - "src/geometry.cpp:59"
  - "src/transforms.cpp:188"
tags: ["architecture", "parser", "runtime", "geometry", "glm"]
tier: knowledge
source: inferred
---

# Architecture Overview

On `cel-euclid-cpp-port` at `c7b30fa`, this C++ extension uses distinct native carriers for seven nominal CEL geometry records, with GLM vectors and matrices behind most carriers. `cel::euclid::install` registers their record adapters with the parser lookup, then installs ordinary geometry and affine/checked signatures; runtime values cross the boundary through those adapters. Changes to a type or operation must keep its schema, conversion, parser registration, and Rust-reference semantics aligned. Start at `include/cel/euclid.hpp:12`, `src/records.hpp:14`, and `src/euclid.cpp:10`.
