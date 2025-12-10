# How Modern C++ Works - December 2025

## Understanding C++ Today

### Core Philosophy

C++ remains a **multi-paradigm** language that supports:
- **Procedural programming**: Functions and structured code
- **Object-oriented programming**: Classes, inheritance, polymorphism
- **Generic programming**: Templates, concepts, metaprogramming
- **Functional programming**: Lambdas, algorithms, ranges
- **Concurrent programming**: Threads, coroutines, async operations

### Memory Management Evolution

#### Traditional C++ (Pre-C++11)
```cpp
// Manual memory management
int* ptr = new int(42);
// ... use ptr ...
delete ptr;  // Must remember to delete
```

#### Modern C++ (C++11+)
```cpp
// RAII - Resource Acquisition Is Initialization
std::unique_ptr<int> ptr = std::make_unique<int>(42);
// Automatic cleanup when ptr goes out of scope
```

**Key Principle**: Resources are tied to object lifetimes. When an object is destroyed, its resources are automatically released.

### Compilation Model

#### Traditional Header-Based Model
```
main.cpp → includes headers → preprocessor expands → compiler processes
  ↓
  Large translation units, slow compilation
```

#### Modern Module-Based Model (C++20/23)
```
main.cpp → imports modules → compiler processes directly
  ↓
  Faster compilation, better encapsulation
```

**Modules** provide:
- Faster compilation (no repeated parsing)
- Better encapsulation (explicit exports)
- Reduced dependency issues

### Type System

#### Static Typing
C++ is **statically typed** - types are checked at compile time:
```cpp
int x = 42;        // x is int
auto y = 3.14;     // y is double (type deduced)
```

#### Type Safety Improvements

**1. Strong Type System**
```cpp
// C++11: enum class (strongly typed enums)
enum class Color { Red, Green, Blue };
Color c = Color::Red;  // Type-safe, no implicit conversion
```

**2. Concepts (C++20)**
```cpp
// Constrain template parameters
template<std::integral T>
T add(T a, T b) {
    return a + b;
}
// Compiler checks T is integral at compile time
```

**3. `constexpr` and `consteval`**
```cpp
// Compile-time evaluation
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
constexpr int fact_10 = factorial(10);  // Computed at compile time
```

### Generic Programming

#### Templates
C++ uses **templates** for generic programming:

```cpp
template<typename T>
T max(T a, T b) {
    return a > b ? a : b;
}
// Compiler generates specific versions: max<int>, max<double>, etc.
```

#### Concepts (C++20) - Enhanced Templates
```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<Addable T>
T sum(T a, T b) {
    return a + b;
}
// Better error messages, clearer intent
```

**How It Works**:
1. Compiler checks if type satisfies concept
2. If not, clear error message
3. Enables better tooling and documentation

### Object-Oriented Programming

#### Classes and Objects
```cpp
class Rectangle {
private:
    double width, height;
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    double area() const { return width * height; }
};
```

**Key Features**:
- **Encapsulation**: Private members, public interface
- **Inheritance**: Derived classes
- **Polymorphism**: Virtual functions

#### Modern OOP Patterns

**1. Rule of Zero/Five**
```cpp
class MyClass {
    // No custom destructor, copy/move constructors/assignments
    // Compiler generates them automatically
    std::unique_ptr<Data> data;  // RAII handles cleanup
};
```

**2. Interface Segregation**
```cpp
// Use concepts instead of abstract base classes
template<typename T>
concept Drawable = requires(T t) {
    t.draw();
};
```

### Functional Programming Features

#### Lambdas (C++11+)
```cpp
auto add = [](int a, int b) { return a + b; };
auto result = add(3, 4);

// Capture variables
int multiplier = 2;
auto times = [multiplier](int x) { return x * multiplier; };
```

#### Algorithms with Ranges (C++20)
```cpp
#include <ranges>
#include <algorithm>

std::vector<int> numbers = {1, 2, 3, 4, 5};
auto evens = numbers 
    | std::views::filter([](int n) { return n % 2 == 0; })
    | std::views::transform([](int n) { return n * 2; });
```

**How It Works**:
- **Lazy evaluation**: Views don't compute until iterated
- **Composable**: Chain operations together
- **Efficient**: Optimized by compiler

### Concurrency Model

#### Traditional Threading (C++11)
```cpp
#include <thread>
#include <mutex>

std::mutex mtx;
void worker() {
    std::lock_guard<std::mutex> lock(mtx);
    // Thread-safe code
}

std::thread t(worker);
t.join();
```

#### Modern Async (C++11+)
```cpp
#include <future>
auto future = std::async(std::launch::async, []() {
    return compute_heavy_task();
});
auto result = future.get();  // Wait for result
```

#### Coroutines (C++20)
```cpp
#include <coroutine>
#include <generator>

std::generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto temp = a + b;
        a = b;
        b = temp;
    }
}

// Usage
for (auto num : fibonacci()) {
    if (num > 1000) break;
    std::cout << num << ' ';
}
```

**How Coroutines Work**:
1. Function can suspend execution (`co_await`, `co_yield`)
2. State is preserved between suspensions
3. Resumes from suspension point
4. Enables efficient async programming

### Execution Model

#### Senders/Receivers (C++26)
```cpp
// Advanced async model
auto task = std::execution::just(42)
    | std::execution::then([](int x) { return x * 2; })
    | std::execution::then([](int x) { return x + 1; });

// Composable, efficient async operations
```

**Benefits**:
- Composable async operations
- Better integration with execution policies
- GPU/accelerator support

### Compile-Time Programming

#### Template Metaprogramming
```cpp
// Compute at compile time
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

constexpr int fact_5 = Factorial<5>::value;  // Computed at compile time
```

#### Modern Compile-Time Features

**1. `constexpr` Functions**
```cpp
constexpr int square(int x) {
    return x * x;
}
constexpr int result = square(5);  // Computed at compile time
```

**2. `if constexpr`**
```cpp
template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // Code only compiled if T is integral
        std::cout << "Integer: " << value << '\n';
    } else {
        std::cout << "Other: " << value << '\n';
    }
}
```

**3. Concepts at Compile Time**
```cpp
template<typename T>
requires std::integral<T>
void process_integer(T value) {
    // Only accepts integral types
}
```

### Error Handling

#### Traditional: Exceptions
```cpp
try {
    risky_operation();
} catch (const std::exception& e) {
    // Handle error
}
```

#### Modern: `std::expected` (C++23)
```cpp
#include <expected>

std::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

auto result = divide(10, 2);
if (result) {
    std::cout << "Result: " << *result << '\n';
} else {
    std::cout << "Error: " << result.error() << '\n';
}
```

**Benefits**:
- No exception overhead
- Explicit error handling
- Type-safe error values

### Value Semantics

C++ emphasizes **value semantics** - objects are values, not references:

```cpp
void process(std::vector<int> vec) {  // Copy
    // Work with vec
}

void process_ref(const std::vector<int>& vec) {  // Reference
    // Work with vec (no copy)
}

void process_move(std::vector<int>&& vec) {  // Move
    // Take ownership of vec
}
```

**Modern Approach**:
- Pass by value for small types
- Pass by const reference for large types
- Use move semantics for transfer of ownership

### Move Semantics (C++11+)

```cpp
class MyClass {
    std::unique_ptr<Data> data;
public:
    // Move constructor
    MyClass(MyClass&& other) noexcept 
        : data(std::move(other.data)) {}
    
    // Move assignment
    MyClass& operator=(MyClass&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }
};
```

**How It Works**:
1. Move transfers ownership without copying
2. Source object left in valid but unspecified state
3. Enables efficient transfer of resources

### Standard Library Architecture

#### Containers
```cpp
// Sequence containers
std::vector<T>      // Dynamic array
std::array<T, N>    // Fixed-size array
std::deque<T>       // Double-ended queue
std::list<T>        // Doubly-linked list
std::forward_list<T> // Singly-linked list

// Associative containers
std::map<K, V>      // Sorted key-value pairs
std::unordered_map<K, V>  // Hash table
std::set<T>         // Sorted unique elements
std::unordered_set<T>     // Hash set
```

#### Algorithms
```cpp
#include <algorithm>
std::sort(vec.begin(), vec.end());
auto it = std::find(vec.begin(), vec.end(), value);
std::transform(vec.begin(), vec.end(), result.begin(), fn);
```

#### Ranges (C++20)
```cpp
#include <ranges>
std::ranges::sort(vec);
auto it = std::ranges::find(vec, value);
auto result = vec | std::views::filter(pred) | std::views::transform(fn);
```

### Compiler Optimizations

Modern C++ compilers perform extensive optimizations:

1. **Inlining**: Function calls replaced with function body
2. **Dead Code Elimination**: Unused code removed
3. **Constant Folding**: Compile-time computations
4. **Loop Optimizations**: Unrolling, vectorization
5. **Link-Time Optimization (LTO)**: Cross-module optimizations

### Linkage and Visibility

#### Internal vs External Linkage
```cpp
// Internal linkage (file-local)
static int local_var = 42;

// External linkage (visible to other files)
int global_var = 42;

// Inline (multiple definitions allowed)
inline int inline_var = 42;
```

#### Modules (C++20)
```cpp
// math.cppm
module math;
export int public_function();  // Exported
int private_function();        // Not exported
```

### Exception Safety

C++ provides exception safety guarantees:

1. **No-throw guarantee**: Operation never throws
2. **Strong guarantee**: Operation succeeds or has no effect
3. **Basic guarantee**: Program remains in valid state
4. **No guarantee**: Program may be left in invalid state

**RAII** ensures basic guarantee automatically.

### Undefined Behavior

C++ has areas of undefined behavior (UB):
- Dereferencing null pointer
- Accessing out-of-bounds array
- Use after free
- Data races

**Modern C++ helps avoid UB**:
- Smart pointers prevent use-after-free
- Bounds checking (with appropriate flags)
- Thread-safe primitives

### Summary: How C++ Works Today

1. **Compilation**: Source → Object files → Linking → Executable
2. **Memory**: Stack (automatic) and Heap (manual/smart pointers)
3. **Types**: Static typing with compile-time checks
4. **Generics**: Templates generate code at compile time
5. **Objects**: RAII manages resources automatically
6. **Concurrency**: Threads, async, coroutines for parallel execution
7. **Optimization**: Compiler optimizes aggressively
8. **Safety**: Modern features reduce common errors

**Key Modern Principles**:
- **RAII**: Resources tied to object lifetimes
- **Value Semantics**: Objects are values
- **Zero-Cost Abstractions**: High-level code, low-level performance
- **Type Safety**: Compile-time checks prevent errors
- **Composability**: Small pieces combine into larger systems

---

*Understanding Modern C++ - December 2025*

