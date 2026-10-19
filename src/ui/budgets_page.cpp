#include "monthmint/ui/budgets_page.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/money_edit.h"
#include "monthmint/ui/month_selector.h"
#include "monthmint/ui/progress_widget.h"
#include "monthmint/ui/theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace monthmint::ui {

BudgetsPage::BudgetsPage(AppContext& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
    month_ = yearMonthOf(QDate::currentDate());
    buildUi();
    refresh();
}

void BudgetsPage::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 24);
    root->setSpacing(16);

    auto* header = new QHBoxLayout;
    auto* title = new QLabel(QStringLiteral("Budgets"), this);
    title->setObjectName(QStringLiteral("Title"));
    header->addWidget(title);
    header->addStretch();
    monthSelector_ = new MonthSelector(this);
    monthSelector_->setMonth(month_);
    header->addWidget(monthSelector_);
    root->addLayout(header);

    connect(monthSelector_, &MonthSelector::monthChanged, this, [this](YearMonth ym) {
        setMonth(ym);
    });

    // Overall budget card
    auto* overallCard = new QFrame(this);
    overallCard->setObjectName(QStringLiteral("Card"));
    auto* ol = new QVBoxLayout(overallCard);
    ol->setContentsMargins(16, 14, 16, 16);
    ol->setSpacing(10);

    auto* overallTitle = new QLabel(QStringLiteral("Overall monthly budget"), overallCard);
    overallTitle->setObjectName(QStringLiteral("CardTitle"));
    ol->addWidget(overallTitle);

    auto* row = new QHBoxLayout;
    overallEdit_ = new MoneyEdit(overallCard);
    overallEdit_->setCurrencyCode(ctx_.settings.currencyCode);
    overallEdit_->setPlaceholderText(QStringLiteral("No budget"));
    auto* setBtn = new QPushButton(QStringLiteral("Set"), overallCard);
    setBtn->setObjectName(QStringLiteral("PrimaryButton"));
    auto* clearBtn = new QPushButton(QStringLiteral("Clear"), overallCard);
    clearBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* copyBtn = new QPushButton(QStringLiteral("Copy last month"), overallCard);
    copyBtn->setObjectName(QStringLiteral("SecondaryButton"));
    row->addWidget(overallEdit_, 1);
    row->addWidget(setBtn);
    row->addWidget(clearBtn);
    row->addStretch();
    row->addWidget(copyBtn);
    ol->addLayout(row);

    overallProgress_ = new ProgressWidget(overallCard);
    overallStatus_ = new QLabel(overallCard);
    overallStatus_->setObjectName(QStringLiteral("Subtitle"));
    ol->addWidget(overallProgress_);
    ol->addWidget(overallStatus_);
    root->addWidget(overallCard);

    connect(setBtn, &QPushButton::clicked, this, [this] {
        const auto v = overallEdit_->value();
        if (!v) {
            QMessageBox::warning(this, QStringLiteral("Invalid budget"),
                                 QStringLiteral("Enter a valid amount."));
            return;
        }
        const auto r = ctx_.budgetService.setOverallLimit(month_, *v);
        if (!r.ok) {
            QMessageBox::warning(this, QStringLiteral("Could not save"), r.error);
            return;
        }
        updateOverall();
        emit dataChanged();
    });
    connect(clearBtn, &QPushButton::clicked, this, [this] {
        (void)ctx_.budgetService.setOverallLimit(month_, std::nullopt);
        updateOverall();
        emit dataChanged();
    });
    connect(copyBtn, &QPushButton::clicked, this, [this] {
        (void)ctx_.budgetService.copyFromPrevious(month_);
        refresh();
        emit dataChanged();
    });

    // Category budgets
    auto* catCard = new QFrame(this);
    catCard->setObjectName(QStringLiteral("Card"));
    auto* cl = new QVBoxLayout(catCard);
    cl->setContentsMargins(16, 14, 16, 16);
    cl->setSpacing(10);
    auto* catTitle = new QLabel(QStringLiteral("Category limits"), catCard);
    catTitle->setObjectName(QStringLiteral("CardTitle"));
    cl->addWidget(catTitle);

    categoryTable_ = new QTableWidget(catCard);
    categoryTable_->setColumnCount(4);
    categoryTable_->setHorizontalHeaderLabels({QStringLiteral("Category"), QStringLiteral("Limit"),
                                               QStringLiteral("Spent"), QStringLiteral("Remaining")});
    categoryTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    categoryTable_->verticalHeader()->setVisible(false);
    categoryTable_->setAlternatingRowColors(true);
    categoryTable_->setShowGrid(false);
    categoryTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    categoryTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    cl->addWidget(categoryTable_);

    auto* catActions = new QHBoxLayout;
    auto* limitBtn = new QPushButton(QStringLiteral("Set limit…"), catCard);
    limitBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* removeBtn = new QPushButton(QStringLiteral("Remove limit"), catCard);
    removeBtn->setObjectName(QStringLiteral("DangerButton"));
    catActions->addStretch();
    catActions->addWidget(limitBtn);
    catActions->addWidget(removeBtn);
    cl->addLayout(catActions);
    root->addWidget(catCard, 1);

    connect(limitBtn, &QPushButton::clicked, this, [this] {
        const int r = categoryTable_->currentRow();
        if (r < 0) {
            QMessageBox::information(this, QStringLiteral("Select a category"),
                                     QStringLiteral("Select a category first."));
            return;
        }
        const auto id = categoryTable_->item(r, 0)->data(Qt::UserRole).toLongLong();
        bool ok = false;
        const QString currentText = categoryTable_->item(r, 1)->text();
        const QString text = QInputDialog::getText(this, QStringLiteral("Category limit"),
                                                   QStringLiteral("Enter monthly limit:"),
                                                   QLineEdit::Normal, currentText, &ok);
        if (!ok) return;
        const auto amount = parseMoney(text, ctx_.settings.currencyCode);
        if (!amount || amount->isNegative()) {
            QMessageBox::warning(this, QStringLiteral("Invalid amount"),
                                 QStringLiteral("Enter a valid non-negative amount."));
            return;
        }
        const auto res = ctx_.budgetService.setCategoryLimit(month_, id, *amount);
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Could not save"), res.error);
            return;
        }
        updateCategories();
        emit dataChanged();
    });
    connect(removeBtn, &QPushButton::clicked, this, [this] {
        const int r = categoryTable_->currentRow();
        if (r < 0) return;
        const auto id = categoryTable_->item(r, 0)->data(Qt::UserRole).toLongLong();
        (void)ctx_.budgetService.removeCategoryLimit(month_, id);
        updateCategories();
        emit dataChanged();
    });
}

void BudgetsPage::setMonth(YearMonth ym) {
    month_ = ym;
    if (monthSelector_->month() != ym) monthSelector_->setMonth(ym);
    refresh();
}

void BudgetsPage::refresh() {
    updateOverall();
    updateCategories();
}

void BudgetsPage::updateOverall() {
    const auto limit = ctx_.budgetService.overallLimit(month_);
    const auto& cur = ctx_.settings.currencyCode;
    const auto spent = ctx_.budgetService.overallSpent(month_);

    if (limit) {
        overallEdit_->setValue(*limit);
    } else {
        overallEdit_->clearValue();
    }

    if (!limit || !limit->isPositive()) {
        overallProgress_->setProgress(0.0);
        overallProgress_->setLabelText(QStringLiteral("—"));
        overallStatus_->setText(QStringLiteral("No overall budget set for %1.").arg(month_.label()));
        return;
    }
    const double p = ctx_.budgetService.overallProgress(month_);
    overallProgress_->setProgress(p);
    overallProgress_->setLabelText(QStringLiteral("%1%").arg(qRound(p * 100)));
    const auto remaining = *limit - spent;
    if (remaining.isNegative()) {
        overallStatus_->setText(QStringLiteral("Over budget by %1 (%2 spent of %3)")
                                    .arg(formatMoney(remaining.abs(), cur), formatMoney(spent, cur),
                                         formatMoney(*limit, cur)));
        overallStatus_->setStyleSheet(QStringLiteral("color: %1;").arg(dangerColor()));
    } else {
        overallStatus_->setText(QStringLiteral("%1 spent of %2 · %3 remaining")
                                    .arg(formatMoney(spent, cur), formatMoney(*limit, cur),
                                         formatMoney(remaining, cur)));
        overallStatus_->setStyleSheet(
            QStringLiteral("color: %1;").arg(p >= 0.8 ? warningColor() : QStringLiteral("#64748B")));
    }
}

void BudgetsPage::updateCategories() {
    const auto categories = ctx_.categoryRepo.all();
    const auto statuses = ctx_.budgetService.categoryStatus(month_, categories);
    const auto& cur = ctx_.settings.currencyCode;

    // Show all expense categories so users can set limits; join with existing limits.
    QVector<Category> expenseCats;
    for (const auto& c : categories) {
        if (c.kind == CategoryKind::Expense || c.kind == CategoryKind::Both) expenseCats.push_back(c);
    }

    categoryTable_->setRowCount(expenseCats.size());
    for (int i = 0; i < expenseCats.size(); ++i) {
        const auto& c = expenseCats[i];
        auto* nameItem = new QTableWidgetItem(c.name);
        nameItem->setData(Qt::UserRole, QVariant::fromValue(c.id));
        categoryTable_->setItem(i, 0, nameItem);

        const auto it = std::ranges::find_if(statuses, [&](const auto& s) { return s.categoryId == c.id; });
        const bool hasLimit = it != statuses.end();

        auto* limitItem = new QTableWidgetItem(hasLimit ? formatMoney(it->limit, cur) : QStringLiteral("—"));
        limitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        categoryTable_->setItem(i, 1, limitItem);

        const auto spent = ctx_.txRepo.sumByCategory(c.id, month_);
        auto* spentItem = new QTableWidgetItem(formatMoney(spent, cur));
        spentItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        categoryTable_->setItem(i, 2, spentItem);

        QString remainingText = QStringLiteral("—");
        QColor color = QColor(QStringLiteral("#64748B"));
        if (hasLimit) {
            const auto remaining = it->limit - spent;
            remainingText = formatMoney(remaining, cur);
            if (it->progress() >= 1.0) color = QColor(dangerColor());
            else if (it->progress() >= 0.8) color = QColor(warningColor());
            else color = QColor(positiveColor());
        }
        auto* remainItem = new QTableWidgetItem(remainingText);
        remainItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        remainItem->setForeground(color);
        categoryTable_->setItem(i, 3, remainItem);
    }
    categoryTable_->resizeColumnsToContents();
    categoryTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

} // namespace monthmint::ui
