#pragma once

#include <QWidget>

class QProgressBar;
class QLabel;

namespace monthmint::ui {

/// Horizontal progress bar with optional label; color shifts near/over limit.
class ProgressWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget* parent = nullptr);

    /// progress in [0, >1]. warnAt / dangerAt are fractions (e.g. 0.8 / 1.0).
    void setProgress(double progress, double warnAt = 0.8, double dangerAt = 1.0);
    void setLabelText(const QString& text);
    void setShowPercent(bool show);

private:
    void refreshColor(double progress, double warnAt, double dangerAt);

    QProgressBar* bar_ = nullptr;
    QLabel* label_ = nullptr;
    bool showPercent_ = true;
};

} // namespace monthmint::ui
