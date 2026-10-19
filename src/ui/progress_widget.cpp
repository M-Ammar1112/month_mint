#include "monthmint/ui/progress_widget.h"
#include "monthmint/ui/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

namespace monthmint::ui {

ProgressWidget::ProgressWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    bar_ = new QProgressBar(this);
    bar_->setRange(0, 1000);
    bar_->setValue(0);
    bar_->setTextVisible(false);
    bar_->setFixedHeight(10);

    label_ = new QLabel(this);
    label_->setMinimumWidth(48);
    label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(bar_, 1);
    layout->addWidget(label_);
}

void ProgressWidget::setProgress(double progress, double warnAt, double dangerAt) {
    const int clamped = static_cast<int>(qBound(0.0, progress, 1.0) * 1000.0);
    bar_->setValue(clamped);
    refreshColor(progress, warnAt, dangerAt);
    if (showPercent_) {
        label_->setText(QStringLiteral("%1%").arg(qRound(progress * 100)));
    }
}

void ProgressWidget::setLabelText(const QString& text) {
    label_->setText(text);
}

void ProgressWidget::setShowPercent(bool show) {
    showPercent_ = show;
}

void ProgressWidget::refreshColor(double progress, double warnAt, double dangerAt) {
    QString color;
    if (progress >= dangerAt) color = dangerColor();
    else if (progress >= warnAt) color = warningColor();
    else color = positiveColor();
    bar_->setStyleSheet(QStringLiteral(
        "QProgressBar::chunk { background: %1; border-radius: 6px; }").arg(color));
}

} // namespace monthmint::ui
