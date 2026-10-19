#include "monthmint/ui/month_selector.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace monthmint::ui {

MonthSelector::MonthSelector(QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    prev_ = new QPushButton(QStringLiteral("‹"), this);
    prev_->setObjectName(QStringLiteral("SecondaryButton"));
    prev_->setFixedWidth(36);
    prev_->setToolTip(QStringLiteral("Previous month"));

    next_ = new QPushButton(QStringLiteral("›"), this);
    next_->setObjectName(QStringLiteral("SecondaryButton"));
    next_->setFixedWidth(36);
    next_->setToolTip(QStringLiteral("Next month"));

    label_ = new QLabel(this);
    label_->setObjectName(QStringLiteral("Title"));
    label_->setMinimumWidth(180);
    label_->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(prev_);
    layout->addWidget(label_);
    layout->addWidget(next_);
    layout->addStretch();

    connect(prev_, &QPushButton::clicked, this, [this] {
        current_ = previousMonth(current_);
        refresh();
        emit monthChanged(current_);
    });
    connect(next_, &QPushButton::clicked, this, [this] {
        current_ = nextMonth(current_);
        refresh();
        emit monthChanged(current_);
    });

    setMonth(YearMonth{QDate::currentDate().year(), QDate::currentDate().month()});
}

void MonthSelector::setMonth(YearMonth ym) {
    current_ = ym;
    refresh();
}

void MonthSelector::refresh() {
    label_->setText(current_.label());
}

} // namespace monthmint::ui
