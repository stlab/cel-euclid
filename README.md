# cel-euclid

`cel-euclid` adds double-precision geometry records and operations to the C++
CEL parser. Install it into an operation lookup before compiling expressions:

```cpp
#include <cel/euclid.hpp>
#include <cel/parser/compiler.hpp>
#include <utility>

cel::op_lookup lookup;
cel::euclid::install(lookup);
cel::parser parser(std::move(lookup));
```

Records use named fields, including nested `Rect` and `Box2D` fields. The
library registers equality and finite checks for all seven geometry records.
Non-transform operations include point and vector arithmetic, bounds,
containment, and angle conversions. Checked operations and transform operations
are not included.

To build and test from a checkout:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The project requires C++20 and uses the pinned CEL parser, CEL runtime, and
GLM dependencies declared in `CMakeLists.txt`.
