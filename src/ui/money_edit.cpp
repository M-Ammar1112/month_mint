#include "monthmint/ui/money_edit.h"
#include "monthmint/core/formatting.h"

#include <QDoubleValidator>

namespace monthmint::ui {

MoneyEdit::MoneyEdit(QWidget* parent) : QLineEdit(parent) {
    setPlaceholderText(QStringLiteral("0.00"));
    const auto& info = currencyInfo(currencyCode_);
    auto* validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setDecimals(info.minorDigits);
    validator->setBottom(0.0);
    setValidator(validator);
    connect(this, &QLineEdit::textChanged, this, [this] { emit valueChanged(); });
}

void MoneyEdit::setCurrencyCode(const QString& code) {
    currencyCode_ = code;
    const auto& info = currencyInfo(code);
    if (auto* v = qobject_cast<QDoubleValidator*>(const_cast<QValidator*>(validator()))) {
        v->setDecimals(info.minorDigits);
    }
}

QString MoneyEdit::currencyCode() const { return currencyCode_; }

std::optional<Money> MoneyEdit::value() const {
    return parseMoney(text(), currencyCode_);
}

void MoneyEdit::setValue(Money minorUnits) {
    const auto& info = currencyInfo(currencyCode_);
    if (info.minorDigits == 0) {
        setText(QString::number(minorUnits.minor()));
    } else {
        const bool neg = minorUnits.isNegative();
        auto abs = minorUnits.abs().minor();
        qulonglong divisor = 1;
        for (int i = 0; i < info.minorDigits; ++i) divisor *= 10;
        const auto maj = abs / static_cast<qulonglong>(divisor);
        const auto min = abs % static_cast<qulonglong>(divisor);
        QString t = QString::number(maj) + QLatin1Char('.') +
                    QStringLiteral("%1").arg(min, info.minorDigits, 10, QLatin1Char('0'));
        if (neg) t.prepend(QLatin1Char('-'));
        setText(t);
    }
}

void MoneyEdit::clearValue() { clear(); }

bool MoneyEdit::hasValidValue() const { return value().has_value(); }

} // namespace monthmint::ui
