# MonthMint

> Your money. One simple monthly view.

A modern, offline-first desktop personal finance manager built with C++20 and Qt 6. Track income, expenses, budgets, and savings goals in a clean, intuitive interface.

## Features

- **Monthly Dashboard** — Income, expenses, net cash flow, savings progress, budget status, and recent transactions at a glance
- **Quick Expense Tracking** — Record expenses with amount, category, date, and optional description
- **Income Management** — Track salary, freelance, business, gifts, and other income sources
- **Monthly Budgeting** — Set overall and per-category spending limits with visual progress indicators
- **Savings Goals** — Set targets and track allocations without double-counting as expenses
- **Recurring Transactions** — Manage monthly rules for rent, subscriptions, salary, etc.
- **Transaction History** — Filterable table with search, sort, edit, and delete
- **Export & Backup** — CSV export, SQLite backup/restore
- **Multi-Currency** — PKR, USD, EUR, GBP, JPY, INR, AED, SAR
- **Light/Dark Themes** — Toggle between clean light and dark color schemes

## Requirements

- **C++20** compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- **Qt 6.2+** (Core, Gui, Widgets, Sql, Test modules)
- **CMake 3.21+**
- **SQLite3** (bundled with Qt SQL driver)

## Building

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt-get install qt6-base-dev cmake g++

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run tests
cd build && ctest --output-on-failure

# Run application
./MonthMint
```

### Windows
```cmd
:: Configure and build with Visual Studio generator
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel

:: Or with MinGW
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --parallel
```

### macOS
```bash
# Install Qt via Homebrew
brew install qt6 cmake

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./MonthMint
```

## Project Structure

```
MonthMint/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── include/monthmint/
│   ├── core/                   # Money type, date utils, formatting
│   ├── models/                 # Domain models (Transaction, Category, etc.)
│   ├── services/               # Business logic services
│   ├── database/               # SQLite repositories and migrations
│   └── ui/                     # Qt widget classes
├── src/
│   ├── main.cpp                # Application entry point
│   ├── core/                   # Implementation files
│   ├── models/
│   ├── services/
│   ├── database/
│   └── ui/
└── tests/                      # Qt Test unit tests
```

## Architecture

MonthMint follows a layered architecture:

1. **Core** — Shared utilities: `Money` (fixed-point arithmetic), date helpers, formatting, CSV
2. **Models** — Domain value objects: Transaction, Category, Budget, SavingsGoal, RecurringRule
3. **Database** — SQLite repositories with migration support and parameterized queries
4. **Services** — Business logic layer: TransactionService, BudgetService, FinanceSummaryService, RecurringService, SavingsService
5. **UI** — Qt Widgets: MainWindow, DashboardPage, TransactionsPage, BudgetsPage, SavingsPage, SettingsDialog

## Key Design Decisions

### Money Storage
All monetary values are stored as signed 64-bit integers in the smallest currency unit (cents, paise). No floating-point arithmetic is used for financial calculations.

### Database
- SQLite with WAL mode for better concurrent read performance
- Foreign key constraints enabled
- Parameterized SQL queries (no string concatenation for user data)
- Versioned schema migrations
- Data stored in OS standard app-data directory

### Testing
- Isolated temporary SQLite databases per test case
- Tests for money arithmetic, transaction CRUD, monthly summaries, budget calculations, recurring rule posting, CSV export, and migrations

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Limitations

- No cloud sync or multi-device support (local-first by design)
- No encryption of the SQLite database
- Limited to single-user, single-wallet usage
- Recurring transaction posting requires manual confirmation (no auto-posting)

## License

MIT License — See LICENSE file for details.

## Acknowledgments

Built with [Qt 6](https://www.qt.io/) and [SQLite](https://www.sqlite.org/).
