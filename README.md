# cel-euclid

`cel-euclid` adds two-dimensional geometry records and operations to the C++
CEL parser. It requires C++20 and binary64 `double`.

To build and test the library:

```sh
cmake --preset test
cmake --build --preset test
ctest --preset test
```

The build fetches cpp-library 6.2.0, cel-runtime 1.1.3, cel-parser 1.1.2,
and GLM 1.0.3. Public contract tests remain in `tests/`; the generated
in-source runner has no test cases. Generated tests have 120-second timeouts.

To build documentation, run `cmake --preset docs`, then
`cmake --build --preset docs`. This requires Doxygen 1.13.1 or later.

Installation is opt-in: configure and build with the `install` preset, then run
`cmake --install build/install --prefix /PATH/TO/PREFIX`, replacing the prefix with
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
