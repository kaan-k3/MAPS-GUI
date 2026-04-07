#pragma once
#include <QString>

// Shared dark theme stylesheet for all dialogs:

inline QString darkDialogStyle()
{
    return R"(
        QDialog { background-color: #2d2d30; color: #e0e0e0; }

        QGroupBox {
            background-color: #252526; border: 1px solid #3e3e42;
            border-radius: 4px; margin-top: 14px; padding-top: 14px;
            font-weight: bold; font-size: 12px; color: #e0e0e0;
        }
        QGroupBox::title {
            subcontrol-origin: margin; left: 10px; padding: 0px 6px;
            color: #ffffff;
        }
        /* Disabled state: keep readable, just slightly dimmed */
        QLabel:disabled { color: #b0b0b0; }
        QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled,
        QLineEdit:disabled, QTextEdit:disabled {
            color: #d0d0d0; background-color: #1e1e1e; border-color: #3e3e42;
        }
        QCheckBox:disabled { color: #b0b0b0; }

        QLabel { color: #e0e0e0; font-size: 12px; }
        QCheckBox { color: #e0e0e0; font-size: 12px; spacing: 6px; }

        QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #1e1e1e; color: #ffffff;
            border: 1px solid #3e3e42; border-radius: 3px;
            padding: 4px 6px; font-size: 12px;
            selection-background-color: #094771;
        }
        QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border-color: #007acc; }

        QComboBox::drop-down {
            border: none; background-color: #333337;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: url(:/icons/arrow.png);
            width: 10px; height: 6px;
        }
        QComboBox QAbstractItemView {
            background-color: #252526; color: #ffffff;
            border: 1px solid #3e3e42;
            selection-background-color: #094771;
        }

        QLineEdit {
            background-color: #1e1e1e; color: #ffffff;
            border: 1px solid #3e3e42; border-radius: 3px;
            padding: 4px 6px; font-size: 12px;
        }
        QLineEdit:focus { border-color: #007acc; }

        QTextEdit {
            background-color: #1e1e1e; color: #ffffff;
            border: 1px solid #3e3e42; border-radius: 3px;
            padding: 4px; font-size: 12px;
            font-family: "Consolas", "Courier New", monospace;
            selection-background-color: #094771;
        }
        QTextEdit:focus { border-color: #007acc; }

        QTableWidget {
            background-color: #1e1e1e; color: #ffffff;
            border: 1px solid #3e3e42; gridline-color: #3e3e42;
            font-size: 12px; selection-background-color: #094771;
        }
        QHeaderView::section {
            background-color: #252526; color: #cccccc;
            padding: 4px 8px; border: none;
            border-right: 1px solid #3e3e42; border-bottom: 1px solid #3e3e42;
            font-size: 11px; font-weight: bold;
        }

        QTabWidget::pane {
            background-color: #2d2d30; border: 1px solid #3e3e42;
            border-top: none;
        }
        QTabBar::tab {
            background-color: #252526; color: #999999;
            padding: 6px 16px; border: 1px solid #3e3e42;
            border-bottom: none; margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #2d2d30; color: #ffffff;
            border-bottom: 2px solid #007acc;
        }
        QTabBar::tab:hover { color: #ffffff; }

        QListWidget {
            background-color: #252526; color: #cccccc;
            border: 1px solid #3e3e42; font-size: 11px;
        }
        QListWidget::item { padding: 2px 4px; }
        QListWidget::item:selected { background-color: #094771; }

        QPushButton {
            background-color: #0e639c; color: #ffffff; border: none;
            border-radius: 3px; padding: 6px 18px; font-size: 12px; font-weight: bold;
        }
        QPushButton:hover { background-color: #1177bb; }
        QPushButton:pressed { background-color: #094771; }
        QPushButton#cancelBtn { background-color: #3e3e42; color: #cccccc; }
        QPushButton#cancelBtn:hover { background-color: #555555; }
        QPushButton#applyBtn, QPushButton#autoWlBtn, QPushButton#estimateBtn {
            background-color: #3e3e42; color: #cccccc;
        }
        QPushButton#applyBtn:hover, QPushButton#autoWlBtn:hover, QPushButton#estimateBtn:hover {
            background-color: #555555;
        }
        QPushButton#browseBtn {
            background-color: #3e3e42; color: #cccccc;
            padding: 4px 10px; font-weight: normal;
        }
        QPushButton#browseBtn:hover { background-color: #555555; }
        QPushButton#secondaryBtn { background-color: #3e3e42; color: #cccccc; font-weight: normal; }
        QPushButton#secondaryBtn:hover { background-color: #555555; }
        QPushButton#removeBtn { background-color: #6e2020; color: #cccccc; font-weight: normal; }
        QPushButton#removeBtn:hover { background-color: #882020; }

        QScrollArea { border: none; background-color: #1e1e1e; }
    )";
}


inline QString darkMainWindowStyle()
{
    return R"(
QMainWindow, QMenu, QMenuBar, QToolBar, QToolButton, QDockWidget,
QTreeWidget, QStatusBar, QHeaderView {
    font-family: "Segoe UI", "SF Pro Display", "Helvetica Neue", sans-serif;
    font-size: 12px;
}
QMainWindow QLabel {
    font-family: "Segoe UI", "SF Pro Display", "Helvetica Neue", sans-serif;
    font-size: 12px;
}

QMainWindow { background-color: #2d2d30; }

QMenuBar {
    background-color: #1e1e1e; color: #cccccc;
    border-bottom: 1px solid #3e3e42; padding: 2px 0px;
}
QMenuBar::item { padding: 4px 10px; background: transparent; }
QMenuBar::item:selected { background-color: #3e3e42; color: #ffffff; }

QMenu { background-color: #252526; color: #cccccc; border: 1px solid #3e3e42; }
QMenu::item { padding: 5px 30px 5px 20px; }
QMenu::item:selected { background-color: #094771; color: #ffffff; }
QMenu::separator { height: 1px; background: #3e3e42; margin: 4px 10px; }

QToolBar {
    background-color: #2d2d30; border-bottom: 1px solid #3e3e42;
    spacing: 2px; padding: 2px 4px;
}
QToolBar::separator { width: 1px; background: #3e3e42; margin: 4px 6px; }
QToolButton {
    background-color: transparent; color: #cccccc;
    border: 1px solid transparent; border-radius: 3px;
    padding: 4px 8px; font-size: 11px;
}
QToolButton:hover { background-color: #3e3e42; border-color: #555555; }
QToolButton:checked { background-color: #094771; color: #ffffff; border-color: #1177bb; }
QToolButton:pressed { background-color: #094771; }

QDockWidget { color: #cccccc; titlebar-close-icon: none; titlebar-normal-icon: none; }
QDockWidget::title {
    background-color: #252526; padding: 6px 10px;
    border-bottom: 1px solid #3e3e42; font-weight: bold;
    font-size: 11px; text-transform: uppercase; letter-spacing: 1px;
}

QTreeWidget { background-color: #1e1e1e; color: #cccccc; border: none; outline: none; font-size: 12px; }
QTreeWidget::item { padding: 4px 6px; border-bottom: 1px solid #2d2d30; }
QTreeWidget::item:selected { background-color: #094771; color: #ffffff; }
QTreeWidget::item:hover { background-color: #2a2d2e; }
QHeaderView::section {
    background-color: #252526; color: #999999;
    padding: 4px 8px; border: none;
    border-right: 1px solid #3e3e42; border-bottom: 1px solid #3e3e42;
    font-size: 10px; text-transform: uppercase;
}

QStatusBar { background-color: #007acc; color: #ffffff; font-size: 11px; }
QStatusBar::item { border: none; }
QStatusBar QLabel { color: #ffffff; padding: 0px 12px; font-size: 11px; }

QScrollBar:vertical { background: #1e1e1e; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #424242; min-height: 20px; border-radius: 5px; }
QScrollBar::handle:vertical:hover { background: #555555; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )";
}
