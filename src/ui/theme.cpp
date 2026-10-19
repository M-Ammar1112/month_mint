#include "monthmint/ui/theme.h"

#include <QApplication>
#include <QPalette>

namespace monthmint::ui {

namespace {
QString g_positive = QStringLiteral("#16A34A");
QString g_warning = QStringLiteral("#D97706");
QString g_danger = QStringLiteral("#DC2626");
} // namespace

QString positiveColor() { return g_positive; }
QString warningColor() { return g_warning; }
QString dangerColor() { return g_danger; }

QString themeStylesheet(const QString& themeName) {
    const bool dark = themeName == QLatin1String("dark");

    if (dark) {
        g_positive = QStringLiteral("#4ADE80");
        g_warning = QStringLiteral("#FBBF24");
        g_danger = QStringLiteral("#F87171");

        return QStringLiteral(R"CSS(
* { font-family: "Segoe UI", "Inter", "Noto Sans", sans-serif; font-size: 14px; }
QMainWindow, QDialog { background: #0F1419; color: #E7ECF3; }
QWidget { color: #E7ECF3; }
QFrame#Sidebar { background: #141A21; border-right: 1px solid #232B36; }
QFrame#Card { background: #171E27; border: 1px solid #232B36; border-radius: 12px; }
QLabel#Title { font-size: 22px; font-weight: 700; }
QLabel#Subtitle { color: #8B98A9; font-size: 13px; }
QLabel#CardTitle { font-size: 13px; font-weight: 600; color: #8B98A9; }
QLabel#BigAmount { font-size: 26px; font-weight: 700; }
QPushButton#NavButton {
    background: transparent; border: none; border-radius: 8px; padding: 8px 14px;
    color: #B7C2D0; text-align: left; font-size: 14px;
}
QPushButton#NavButton:hover { background: #1D2632; color: #E7ECF3; }
QPushButton#NavButton:checked { background: #1F3D2B; color: #4ADE80; font-weight: 600; }
QPushButton#PrimaryButton {
    background: #16A34A; color: white; border: none; border-radius: 8px;
    padding: 9px 18px; font-weight: 600;
}
QPushButton#PrimaryButton:hover { background: #15803D; }
QPushButton#PrimaryButton:disabled { background: #2A3340; color: #6B7787; }
QPushButton#SecondaryButton {
    background: #1D2632; color: #E7ECF3; border: 1px solid #2C3644; border-radius: 8px;
    padding: 8px 16px;
}
QPushButton#SecondaryButton:hover { background: #243040; }
QPushButton#DangerButton {
    background: #7F1D1D; color: #FECACA; border: none; border-radius: 8px; padding: 8px 16px;
}
QLineEdit, QComboBox, QDateEdit, QSpinBox, QTextEdit {
    background: #10161D; border: 1px solid #2C3644; border-radius: 8px;
    padding: 7px 10px; selection-background-color: #16A34A;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus { border-color: #16A34A; }
QComboBox QAbstractItemView {
    background: #171E27; border: 1px solid #2C3644; selection-background-color: #1F3D2B;
}
QTableWidget {
    background: #141A21; border: 1px solid #232B36; border-radius: 10px;
    gridline-color: #1D2632; alternate-background-color: #171E27;
}
QHeaderView::section {
    background: #171E27; color: #8B98A9; border: none; border-bottom: 1px solid #232B36;
    padding: 8px; font-weight: 600;
}
QTableWidget::item:selected { background: #1F3D2B; color: #E7ECF3; }
QProgressBar {
    background: #10161D; border: none; border-radius: 6px; height: 10px; text-align: center;
}
QProgressBar::chunk { border-radius: 6px; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #2C3644; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QTabWidget::pane { border: none; }
QMessageBox { background: #171E27; }
QToolTip { background: #1D2632; color: #E7ECF3; border: 1px solid #2C3644; }
)CSS");
    }

    g_positive = QStringLiteral("#16A34A");
    g_warning = QStringLiteral("#D97706");
    g_danger = QStringLiteral("#DC2626");

    return QStringLiteral(R"CSS(
* { font-family: "Segoe UI", "Inter", "Noto Sans", sans-serif; font-size: 14px; }
QMainWindow, QDialog { background: #F6F8FB; color: #17202B; }
QWidget { color: #17202B; }
QFrame#Sidebar { background: #FFFFFF; border-right: 1px solid #E5EAF0; }
QFrame#Card { background: #FFFFFF; border: 1px solid #E5EAF0; border-radius: 12px; }
QLabel#Title { font-size: 22px; font-weight: 700; }
QLabel#Subtitle { color: #64748B; font-size: 13px; }
QLabel#CardTitle { font-size: 13px; font-weight: 600; color: #64748B; }
QLabel#BigAmount { font-size: 26px; font-weight: 700; }
QPushButton#NavButton {
    background: transparent; border: none; border-radius: 8px; padding: 8px 14px;
    color: #475569; text-align: left; font-size: 14px;
}
QPushButton#NavButton:hover { background: #EEF2F7; color: #17202B; }
QPushButton#NavButton:checked { background: #E7F6EC; color: #15803D; font-weight: 600; }
QPushButton#PrimaryButton {
    background: #16A34A; color: white; border: none; border-radius: 8px;
    padding: 9px 18px; font-weight: 600;
}
QPushButton#PrimaryButton:hover { background: #15803D; }
QPushButton#PrimaryButton:disabled { background: #CBD5E1; color: #64748B; }
QPushButton#SecondaryButton {
    background: #FFFFFF; color: #17202B; border: 1px solid #D7DEE8; border-radius: 8px;
    padding: 8px 16px;
}
QPushButton#SecondaryButton:hover { background: #F1F5F9; }
QPushButton#DangerButton {
    background: #FEF2F2; color: #B91C1C; border: 1px solid #FECACA; border-radius: 8px;
    padding: 8px 16px;
}
QLineEdit, QComboBox, QDateEdit, QSpinBox, QTextEdit {
    background: #FFFFFF; border: 1px solid #D7DEE8; border-radius: 8px;
    padding: 7px 10px; selection-background-color: #BBF7D0;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus { border-color: #16A34A; }
QComboBox QAbstractItemView {
    background: #FFFFFF; border: 1px solid #D7DEE8; selection-background-color: #E7F6EC;
}
QTableWidget {
    background: #FFFFFF; border: 1px solid #E5EAF0; border-radius: 10px;
    gridline-color: #EEF2F7; alternate-background-color: #F8FAFC;
}
QHeaderView::section {
    background: #F8FAFC; color: #64748B; border: none; border-bottom: 1px solid #E5EAF0;
    padding: 8px; font-weight: 600;
}
QTableWidget::item:selected { background: #E7F6EC; color: #17202B; }
QProgressBar {
    background: #EEF2F7; border: none; border-radius: 6px; height: 10px; text-align: center;
}
QProgressBar::chunk { border-radius: 6px; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #CBD5E1; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QTabWidget::pane { border: none; }
QToolTip { background: #17202B; color: #F8FAFC; border: none; }
)CSS");
}

void applyTheme(const QString& themeName) {
    qApp->setStyleSheet(themeStylesheet(themeName));
}

} // namespace monthmint::ui
