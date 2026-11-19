# cel-euclid

`cel-euclid` adds two-dimensional geometry records and operations to the C++
CEL parser. It requires C++20 and binary64 `double`.

To build and test the library:

```sh
cmake -S . -B build -DBUILD_TESTING=ON -DBUILD_DOCS=ON -DCEL_INSTALL=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --build build --target docs
```

The documentation target requires Doxygen 1.13.1 or later. To install, run
`cmake --install build --prefix /PATH/TO/PREFIX`, replacing the prefix with
your installation directory. Install compatible `cel-parser`, `cel-runtime`,
and GLM packages separately, and make their prefixes discoverable through
`CMAKE_PREFIX_PATH` when configuring consumers. Consumers use
`find_package(cel-euclid CONFIG REQUIRED)`, link `cel::euclid`, and include
`<cel/euclid.hpp>`. The target supplies the dependency links.

See the [tutorial](docs/tutorial.md) for a runnable example and the
[reference](docs/reference.md) for schemas, operations, and checked errors.
The [Rust geometry reference](https://stlab.github.io/cel-rs/cel-book/geometry.html)
describes the shared geometry semantics.

The library is distributed under the Boost Software License 1.0; see
`LICENSE`. Third-party notices are in `licenses/`.
