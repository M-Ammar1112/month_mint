#include "monthmint/ui/main_window.h"
#include "monthmint/services/backup_service.h"
#include "monthmint/ui/add_transaction_dialog.h"
#include "monthmint/ui/budgets_page.h"
#include "monthmint/ui/dashboard_page.h"
#include "monthmint/ui/recurring_dialog.h"
#include "monthmint/ui/savings_page.h"
#include "monthmint/ui/settings_dialog.h"
#include "monthmint/ui/theme.h"
#include "monthmint/ui/transactions_page.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace monthmint::ui {

namespace {
constexpr int kDashboard = 0;
constexpr int kTransactions = 1;
constexpr int kBudgets = 2;
constexpr int kSavings = 3;
} // namespace

MainWindow::MainWindow(std::unique_ptr<AppContext> ctx, QWidget* parent)
    : QMainWindow(parent), ctx_(std::move(ctx)) {
    month_ = yearMonthOf(QDate::currentDate());
    buildUi();
    checkRecurringOnStartup();
    refreshAll();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    setWindowTitle(QStringLiteral("MonthMint"));
    resize(1100, 760);
    setMinimumSize(900, 620);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    buildSidebar();
    layout->addWidget(sidebar_);

    stack_ = new QStackedWidget(central);
    dashboard_ = new DashboardPage(*ctx_, stack_);
    transactions_ = new TransactionsPage(*ctx_, stack_);
    budgets_ = new BudgetsPage(*ctx_, stack_);
    savings_ = new SavingsPage(*ctx_, stack_);
    stack_->addWidget(dashboard_);
    stack_->addWidget(transactions_);
    stack_->addWidget(budgets_);
    stack_->addWidget(savings_);
    layout->addWidget(stack_, 1);

    setCentralWidget(central);

    connect(dashboard_, &DashboardPage::navigateToTransactions, this,
            [this] { switchPage(kTransactions); });
    connect(transactions_, &TransactionsPage::dataChanged, this, &MainWindow::refreshAll);
    connect(budgets_, &BudgetsPage::dataChanged, this, &MainWindow::refreshAll);
    connect(savings_, &SavingsPage::dataChanged, this, &MainWindow::refreshAll);

    switchPage(kDashboard);
}

void MainWindow::buildSidebar() {
    auto* sidebar = new QFrame(this);
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(210);
    sidebar_ = sidebar;
    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(14, 20, 14, 20);
    layout->setSpacing(8);

    auto* brand = new QLabel(QStringLiteral("MonthMint"), sidebar);
    brand->setObjectName(QStringLiteral("Title"));
    auto* tagline = new QLabel(QStringLiteral("One simple monthly view"), sidebar);
    tagline->setObjectName(QStringLiteral("Subtitle"));
    layout->addWidget(brand);
    layout->addWidget(tagline);
    layout->addSpacing(18);

    const auto makeNav = [&](const QString& text, int index) {
        auto* btn = new QPushButton(text, sidebar);
        btn->setObjectName(QStringLiteral("NavButton"));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, index] { switchPage(index); });
        layout->addWidget(btn);
        return btn;
    };

    navDashboard_ = makeNav(QStringLiteral("  Dashboard"), kDashboard);
    navTransactions_ = makeNav(QStringLiteral("  Transactions"), kTransactions);
    navBudgets_ = makeNav(QStringLiteral("  Budgets"), kBudgets);
    navSavings_ = makeNav(QStringLiteral("  Savings"), kSavings);
    layout->addStretch();

    auto* addExpense = new QPushButton(QStringLiteral("+  Add expense"), sidebar);
    addExpense->setObjectName(QStringLiteral("PrimaryButton"));
    addExpense->setCursor(Qt::PointingHandCursor);
    connect(addExpense, &QPushButton::clicked, this, &MainWindow::addExpense);
    layout->addWidget(addExpense);

    auto* addIncome = new QPushButton(QStringLiteral("+  Add income"), sidebar);
    addIncome->setObjectName(QStringLiteral("SecondaryButton"));
    addIncome->setCursor(Qt::PointingHandCursor);
    connect(addIncome, &QPushButton::clicked, this, &MainWindow::addIncome);
    layout->addWidget(addIncome);

    auto* recurring = new QPushButton(QStringLiteral("Recurring"), sidebar);
    recurring->setObjectName(QStringLiteral("SecondaryButton"));
    connect(recurring, &QPushButton::clicked, this, [this] {
        RecurringDialog dlg(*ctx_, this);
        connect(&dlg, &RecurringDialog::dataChanged, this, &MainWindow::refreshAll);
        dlg.exec();
    });
    layout->addWidget(recurring);

    auto* settings = new QPushButton(QStringLiteral("Settings"), sidebar);
    settings->setObjectName(QStringLiteral("SecondaryButton"));
    connect(settings, &QPushButton::clicked, this, &MainWindow::openSettings);
    layout->addWidget(settings);
}

void MainWindow::switchPage(int index) {
    stack_->setCurrentIndex(index);
    navDashboard_->setChecked(index == kDashboard);
    navTransactions_->setChecked(index == kTransactions);
    navBudgets_->setChecked(index == kBudgets);
    navSavings_->setChecked(index == kSavings);
    if (index == kTransactions) transactions_->refresh();
    if (index == kBudgets) budgets_->refresh();
    if (index == kSavings) savings_->refresh();
}

void MainWindow::refreshAll() {
    dashboard_->setMonth(month_);
    budgets_->setMonth(month_);
    savings_->setMonth(month_);
    transactions_->setMonth(month_);
}

void MainWindow::changeMonth(YearMonth ym) {
    month_ = ym;
    refreshAll();
}

void MainWindow::addExpense() {
    AddTransactionDialog dlg(AddTransactionDialog::Mode::Expense, *ctx_, this);
    if (dlg.exec() == QDialog::Accepted) refreshAll();
}

void MainWindow::addIncome() {
    AddTransactionDialog dlg(AddTransactionDialog::Mode::Income, *ctx_, this);
    if (dlg.exec() == QDialog::Accepted) refreshAll();
}

void MainWindow::openSettings() {
    SettingsDialog dlg(*ctx_, this);
    connect(&dlg, &SettingsDialog::settingsChanged, this, [this] {
        applyTheme(ctx_->settings.theme);
        refreshAll();
    });
    dlg.exec();
}

void MainWindow::checkRecurringOnStartup() {
    const auto due = ctx_->recurringService.duePreviews(QDate::currentDate());
    if (due.isEmpty()) return;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Recurring transactions"),
        QStringLiteral("%1 recurring transaction(s) are due this month.\n\nReview and post them now?")
            .arg(due.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer != QMessageBox::Yes) return;

    RecurringDialog dlg(*ctx_, this);
    connect(&dlg, &RecurringDialog::dataChanged, this, &MainWindow::refreshAll);
    dlg.exec();
}

} // namespace monthmint::ui
