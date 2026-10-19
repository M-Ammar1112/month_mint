#include "monthmint/core/money.h"

// Money is header-only; this TU ensures the library always has at least one object file
// and provides an anchor for potential future non-inline helpers.

namespace monthmint {
namespace {
[[maybe_unused]] constexpr Money kZero{};
}
} // namespace monthmint
