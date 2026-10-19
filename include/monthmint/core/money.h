#pragma once

#include <cstdint>
#include <compare>
#include <stdexcept>
#include <limits>

namespace monthmint {

class MoneyOverflowError : public std::overflow_error {
public:
    MoneyOverflowError() : std::overflow_error("Money arithmetic overflow") {}
};

/// Monetary value stored as signed 64-bit integer in the smallest currency unit (e.g. cents, paisa).
class Money {
public:
    using Value = std::int64_t;

    constexpr Money() noexcept = default;
    constexpr explicit Money(Value minorUnits) noexcept : minor_(minorUnits) {}

    [[nodiscard]] constexpr Value minor() const noexcept { return minor_; }
    [[nodiscard]] constexpr bool isZero() const noexcept { return minor_ == 0; }
    [[nodiscard]] constexpr bool isNegative() const noexcept { return minor_ < 0; }
    [[nodiscard]] constexpr bool isPositive() const noexcept { return minor_ > 0; }

    [[nodiscard]] constexpr Money operator-() const {
        if (minor_ == std::numeric_limits<Value>::min()) throw MoneyOverflowError{};
        return Money{-minor_};
    }

    [[nodiscard]] friend constexpr Money checkedAdd(Money a, Money b) {
        const auto ua = static_cast<std::uint64_t>(a.minor_);
        const auto ub = static_cast<std::uint64_t>(b.minor_);
        const auto sum = ua + ub;
        const auto result = static_cast<Value>(sum);
        if (b.minor_ > 0 && result < a.minor_) throw MoneyOverflowError{};
        if (b.minor_ < 0 && result > a.minor_) throw MoneyOverflowError{};
        return Money{result};
    }

    [[nodiscard]] friend constexpr Money operator+(Money a, Money b) { return checkedAdd(a, b); }

    [[nodiscard]] friend constexpr Money operator-(Money a, Money b) {
        if (b.minor_ == std::numeric_limits<Value>::min()) throw MoneyOverflowError{};
        return checkedAdd(a, Money{-b.minor_});
    }

    constexpr Money& operator+=(Money rhs) { return *this = *this + rhs; }
    constexpr Money& operator-=(Money rhs) { return *this = *this - rhs; }

    [[nodiscard]] friend constexpr auto operator<=>(Money, Money) noexcept = default;

    [[nodiscard]] constexpr Money abs() const {
        if (minor_ == std::numeric_limits<Value>::min()) throw MoneyOverflowError{};
        return minor_ < 0 ? Money{-minor_} : *this;
    }

    [[nodiscard]] static constexpr Money max() noexcept {
        return Money{std::numeric_limits<Value>::max()};
    }
    [[nodiscard]] static constexpr Money min() noexcept {
        return Money{std::numeric_limits<Value>::min()};
    }

private:
    Value minor_ = 0;
};

[[nodiscard]] inline Money multiply(Money m, std::int64_t factor) {
    const auto a = static_cast<std::uint64_t>(m.minor() < 0 ? -(m.minor() + 1) + 1 : m.minor());
    const auto b = static_cast<std::uint64_t>(factor < 0 ? -(factor + 1) + 1 : factor);
    constexpr auto limit = static_cast<std::uint64_t>(std::numeric_limits<Money::Value>::max());
    if (a != 0 && b > limit / a) throw MoneyOverflowError{};
    const auto product = static_cast<Money::Value>(m.minor() * factor);
    if (factor != 0 && product / factor != m.minor()) throw MoneyOverflowError{};
    return Money{product};
}

/// Integer division rounding half away from zero.
[[nodiscard]] inline Money divideRounding(Money m, std::int64_t divisor) {
    if (divisor == 0) throw std::invalid_argument{"divide by zero"};
    const bool negative = (m.minor() < 0) != (divisor < 0);
    auto n = static_cast<std::uint64_t>(m.minor() < 0 ? -(m.minor() + 1) + 1 : m.minor());
    auto d = static_cast<std::uint64_t>(divisor < 0 ? -(divisor + 1) + 1 : divisor);
    const auto q = n / d;
    const auto r = n % d;
    auto result = static_cast<Money::Value>(q);
    if (r * 2 >= d) ++result;
    return Money{negative ? -result : result};
}

} // namespace monthmint
