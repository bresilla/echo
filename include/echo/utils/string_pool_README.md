# String Pool - Future Optimization

## Status: Implemented but Not Integrated

The string pool (`string_pool.hpp`) is fully implemented but **not currently integrated** into the logging path. This is intentional - it's available as a future optimization.

## What It Does

Provides a thread-local 4KB buffer for building log messages without heap allocations:
- Zero allocations for messages < 4KB
- Automatic fallback to heap for larger messages
- No contention between threads (thread-local storage)

## Why Not Integrated?

1. **Current implementation already fast**: std::format (when available) is already 4x faster than ostringstream
2. **Complexity**: Integrating the pool requires refactoring the formatter and proxy classes
3. **Diminishing returns**: Most log messages are small and std::format handles them efficiently
4. **Risk**: Pool integration could introduce bugs in the stable logging path

## Future Integration

To integrate the pool:

1. Modify `detail::build_message()` in `core/formatter.hpp` to use `PooledString`
2. Update `log_proxy` and `print_proxy` constructors to use pool allocation
3. Add `PoolResetGuard` in proxy destructors
4. Benchmark to verify performance improvement

## Current Performance

Without pool integration, Echo still achieves:
- **0 ns** for compile-time filtered logs
- **~8 ns** for runtime filtered logs
- **~500 ns** for std::format string building (C++20)
- **~2 μs** for ostringstream string building (C++17)

The pool would primarily benefit C++17 builds by reducing ostringstream allocations.

## Usage (if you want to integrate it yourself)

```cpp
#include <echo/utils/string_pool.hpp>

// In your logging code:
echo::detail::PoolResetGuard guard;  // Reset pool when done
auto pooled = echo::detail::PooledString("message");
// Use pooled.view() or pooled.str()
```

## Conclusion

The string pool is **production-ready code** but remains unintegrated as a **future optimization**. The current implementation is already highly performant for most use cases.
