#pragma once

#include "monthmint/core/result.h"
#include "monthmint/models/transaction.h"
#include <QDialog>
#include <optional>

class QComboBox;
class QDateEdit;
class QLineEdit;
class QPushButton;

namespace monthmint::ui {

class AppContext;
class MoneyEdit;

/// Quick-add / edit dialog for a transaction. Minimal fields: amount, category, date, description.
class AddTransactionDialog : public QDialog {
    Q_OBJECT
public:
    enum class Mode { Expense, Income };

    AddTransactionDialog(Mode mode, AppContext& ctx, QWidget* parent = nullptr);

    /// Edit an existing transaction (type is preserved from the record).
    static std::optional<Transaction> editTransaction(AppContext& ctx, Transaction t,
                                                      QWidget* parent);

    [[nodiscard]] Transaction result() const { return result_; }

protected:
    void accept() override;

private:
    void buildUi();
    void loadCategories();

    Mode mode_;
    AppContext& ctx_;
    std::optional<Transaction> editing_;
    Transaction result_;
    MoneyEdit* amountEdit_ = nullptr;
    QComboBox* categoryCombo_ = nullptr;
    QDateEdit* dateEdit_ = nullptr;
    QLineEdit* descEdit_ = nullptr;
    QPushButton* saveBtn_ = nullptr;
    qint64 selectedCategoryId_ = 0;
};

} // namespace monthmint::ui
