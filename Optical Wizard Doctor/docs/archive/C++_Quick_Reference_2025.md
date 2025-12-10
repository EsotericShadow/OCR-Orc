# C++ Quick Reference - December 2025

## Essential Modern C++ Syntax

### Modules (C++20/23)
```cpp
// math.cppm
module math;
export int add(int a, int b);

// main.cpp
import math;
```

### Concepts (C++20)
```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};
```

### Ranges (C++20/23)
```cpp
#include <ranges>
auto result = vec | std::views::filter(pred) | std::views::transform(fn);
```

### Format (C++20)
```cpp
std::string msg = std::format("Value: {}, Name: {}", value, name);
```

### Smart Pointers
```cpp
std::unique_ptr<T> ptr = std::make_unique<T>();
std::shared_ptr<T> shared = std::make_shared<T>();
```

### Coroutines (C++20)
```cpp
#include <coroutine>
std::generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto temp = a + b;
        a = b;
        b = temp;
    }
}
```

## Popular Frameworks Quick Links

| Framework | Purpose | Website |
|-----------|---------|---------|
| Qt | GUI Development | qt.io |
| Boost | Utility Libraries | boost.org |
| Eigen | Linear Algebra | eigen.tuxfamily.org |
| OpenCV | Computer Vision | opencv.org |
| SFML | Multimedia | sfml-dev.org |
| fmt | Formatting | fmt.dev |
| spdlog | Logging | github.com/gabime/spdlog |

## Build System Commands

### CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### vcpkg
```bash
vcpkg install <package-name>
vcpkg integrate install
```

### Conan
```bash
conan install .
conan build .
```

## Compiler Flags (Modern C++)

### GCC/Clang
```bash
-std=c++23 -Wall -Wextra -Wpedantic
```

### MSVC
```bash
/std:c++latest /W4
```

## Standard Library Headers (Modern)

```cpp
#include <ranges>        // Ranges library
#include <coroutine>      // Coroutines
#include <format>         // Formatting
#include <expected>       // Error handling (C++23)
#include <mdspan>         // Multi-dimensional arrays (C++23)
#include <execution>      // Execution policies (C++26)
```

## Common Patterns

### RAII
```cpp
class Resource {
    std::unique_ptr<Data> data;
public:
    Resource() : data(std::make_unique<Data>()) {}
    // Automatic cleanup
};
```

### Type Traits
```cpp
#include <type_traits>
if constexpr (std::is_integral_v<T>) {
    // compile-time branch
}
```

### Variadic Templates
```cpp
template<typename... Args>
void log(Args... args) {
    (std::cout << ... << args) << '\n';
}
```

## Documentation Quick Access

- **cppreference.com**: Complete reference
- **C++ Core Guidelines**: Best practices
- **Compiler Docs**: GCC, Clang, MSVC
- **Standard Papers**: WG21 papers

## Version Timeline

- **C++98**: First standard
- **C++03**: Bug fixes
- **C++11**: Modern C++ begins
- **C++14**: Small improvements
- **C++17**: Filesystem, optional, variant
- **C++20**: Modules, coroutines, concepts, ranges
- **C++23**: Complete modules, format library
- **C++26**: Senders, execution, reflection (in progress)

---

*Quick Reference - December 2025*

