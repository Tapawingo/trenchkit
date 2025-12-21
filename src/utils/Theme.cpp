#include "Theme.h"

QString Theme::getStyleSheet() {
    return R"(
        /* ===== MAIN WINDOW ===== */
        QWidget#centralwidget {
            background-color: #1e1e1e;
        }

        /* ===== PANELS ===== */
        #leftPanel, #middlePanel, #rightPanel {
            margin: 8px;
            margin-top: 0px;
            border: 1px solid #171717;
            border-radius: 3px;
        }

        #middlePanel {
            margin-left: 0px;
            margin-right: 0px;
        }

        GradientFrame {
            margin: 0px;
        }

        /* ===== TITLE BAR ===== */
        #titleBar {
            background-color: #2c2c2c;
        }
        #minimizeButton, #closeButton {
            background-color: transparent;
            color: #e1d0ab;
            border: none;
            font-size: 24px;
            font-weight: 300;
        }
        #minimizeButton:hover {
            background-color: #404040;
        }
        #closeButton:hover {
            background-color: #e81123;
        }

        /* ===== SECTION TITLES ===== */
        #installPathTitle,
        #profileTitle,
        QLabel[objectName="sectionTitle"] {
            color: #e1d0ab;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 4px;
        }

        /* ===== INSTALL PATH WIDGET ===== */
        #installPathInput {
            background-color: #16130e;
            color: #e1d0ab;
            border: 1px solid #404040;
            padding: 6px;
        }
        #installPathInput:focus {
            border-color: #3d402e;
        }
        #installPathBrowse {
            background-color: #3d402e;
            color: #e1d0ab;
            border: none;
            padding: 6px 12px;
        }
        #installPathBrowse:hover {
            background-color: #373A29;
        }
        #installPathBrowse:pressed {
            background-color: #313325;
        }
        #installPathStatus {
            color: #cccccc;
            font-size: 12px;
            margin-top: 4px;
        }
        #installPathStatus[status="valid"] {
            color: #4ec9b0;
        }
        #installPathStatus[status="invalid"] {
            color: #f48771;
        }
        #installPathStatus[status="loading"] {
            color: #cccccc;
        }

        /* ===== PROFILE MANAGER ===== */
        #profileList {
            background-color: #16130e;
            color: #e1d0ab;
            border: 1px solid #404040;
            padding: 4px;
        }
        #profileList::item {
            padding: 0px;
            background-color: transparent;
        }
        #profileList::item:selected {
            background-color: transparent;
        }
        #profileList::item:hover {
            background-color: transparent;
        }
        #profileButton {
            background-color: #3d402e;
            color: #e1d0ab;
            border: none;
            padding: 6px 12px;
            font-size: 12px;
        }
        #profileButton:hover {
            background-color: #373A29;
        }
        #profileButton:pressed {
            background-color: #313325;
        }
        #profileButton:disabled {
            background-color: #646658;
            color: #808080;
        }
        #importIconButton {
            background-color: #3d402e;
            border: none;
            padding: 4px;
        }
        #importIconButton:hover {
            background-color: #373A29;
        }
        #importIconButton:pressed {
            background-color: #313325;
        }

        /* ===== PROFILE ROW WIDGET ===== */
        ProfileRowWidget {
            background-color: #2c2c2c;
            min-height: 32px;
            margin-bottom: 3px;
        }
        ProfileRowWidget[selected="true"] {
            background-color: #3d402e;
        }
        ProfileRowWidget:hover {
            background-color: #3a3a3a;
        }
        ProfileRowWidget[selected="true"]:hover {
            background-color: #373A29;
        }
        #profileNameLabel {
            color: #ffffff;
            font-size: 13px;
        }
        #profileNameLabel[active="true"] {
            color: #4ec9b0;
            font-weight: bold;
        }

        /* ===== MOD LIST ===== */
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            background-color: transparent;
            border: none;
            padding: 0px;
        }
        QListWidget::item:selected {
            background-color: transparent;
            border: none;
        }
        QListWidget::item:hover {
            background-color: transparent;
        }

        /* ===== MOD LIST WIDGET ===== */
        #modLoadingLabel {
            color: #888888;
            font-size: 14px;
            font-style: italic;
            padding: 20px;
        }
        #modListTitle {
            color: #e1d0ab;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 4px;
        }

        /* ===== MOD ROW WIDGET ===== */
        ModRowWidget {
            background-color: transparent;
            border-bottom: 1px solid #000000;
            margin: 0px;
        }
        ModRowWidget[selected="true"] {
            background-color: #3d402e;
            border: 2px solid #373A29;
        }
        ModRowWidget:hover {
            background-color: #888;
            border-color: #4d4d4d;
        }
        ModRowWidget[selected="true"]:hover {
            background-color: #373A29;
            border-color: #1a8dd8;
        }
        #modNameLabel {
            font-size: 14px;
            font-weight: bold;
            color: #e0e0e0;
        }
        #modDateLabel {
            font-size: 11px;
            color: #888888;
            margin-left: 28px;
        }

        /* ===== RIGHT PANEL WIDGET ===== */
        #modActionsTitle, #launchTitle, #backupTitle, #activityLogTitle {
            color: #e1d0ab;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 8px;
        }

        /* ===== BUTTONS (RIGHT PANEL) ===== */
        QPushButton {
            background-color: #3d402e;
            color: white;
            border: none;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #373A29;
        }
        QPushButton:pressed {
            background-color: #313325;
        }
        QPushButton:disabled {
            background-color: #646658;
            color: #888888;
        }

        /* ===== TOOL BUTTON (LAUNCH BUTTON) ===== */
        QToolButton {
            background-color: #3d402e;
            color: white;
            border: none;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 13px;
        }
        QToolButton:hover {
            background-color: #373A29;
        }
        QToolButton:pressed {
            background-color: #313325;
        }
        QToolButton::menu-button {
            background-color: #3d402e;
            border: none;
            border-left: 1px solid #0a4f7a;
            width: 18px;
        }
        QToolButton::menu-button:hover {
            background-color: #373A29;
        }
        QToolButton::menu-button:pressed {
            background-color: #313325;
        }
        QToolButton::menu-indicator {
            subcontrol-position: right center;
            subcontrol-origin: padding;
            font-size: 10px;
            left: -6px;
            width: 10px;
            height: 10px;
        }

        /* ===== SEPARATORS ===== */
        QFrame[frameShape="4"][frameShadow="16"] {
            background-color: #16130e;
            max-height: 1px;
        }

        /* ===== CHECKBOXES ===== */
        QCheckBox {
            color: #e1d0ab;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #404040;
            background-color: #2c2c2c;
        }
        QCheckBox::indicator:hover {
            border-color: #3d402e;
        }
        QCheckBox::indicator:checked {
            background-color: #3d402e;
            border-color: #3d402e;
        }

        /* ===== LABELS ===== */
        QLabel {
            color: #e1d0ab;
        }

        /* ===== DIALOGS ===== */
        QDialog {
            background-color: #2c2c2c;
            color: #e1d0ab;
        }

        /* ===== LINE EDITS ===== */
        QLineEdit {
            background-color: #2c2c2c;
            color: #e1d0ab;
            border: 1px solid #404040;
            padding: 6px;
        }
        QLineEdit:focus {
            border-color: #3d402e;
        }

        /* ===== TEXT EDITS ===== */
        QTextEdit {
            background-color: #2c2c2c;
            color: #e1d0ab;
            border: 1px solid #404040;
            padding: 6px;
        }
        QTextEdit:focus {
            border-color: #3d402e;
        }

        /* ===== MESSAGE BOXES ===== */
        QMessageBox {
            background-color: #2c2c2c;
        }
        QMessageBox QLabel {
            color: #e1d0ab;
        }

        /* ===== MENUS ===== */
        QMenu {
            background-color: #2c2c2c;
            color: #e1d0ab;
            border: 1px solid #404040;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px;
        }
        QMenu::item:selected {
            background-color: #3d402e;
        }
        QMenu::separator {
            height: 1px;
            background-color: #404040;
            margin: 4px 0px;
        }

        /* ===== SCROLL BARS ===== */
        QScrollBar:vertical {
            background-color: #2c2c2c;
            width: 12px;
        }
        QScrollBar::handle:vertical {
            background-color: #404040;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #4d4d4d;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background-color: #2c2c2c;
            height: 12px;
        }
        QScrollBar::handle:horizontal {
            background-color: #404040;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #4d4d4d;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        /* ===== ACTIVITY LOG ===== */
        LogEntryWidget {
            border-radius: 2px;
            border-width: 1px;
            border-style: solid;
        }

        LogEntryWidget[level="info"] {
            background-color: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #2a2a2a, stop: 0.6 #00000000);
        }

        LogEntryWidget[level="success"] {
            background-color: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #1a4d3f, stop: 0.6 #00000000);
        }

        LogEntryWidget[level="warning"] {
            background-color: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #4d2f1a, stop: 0.6 #00000000);
        }

        LogEntryWidget[level="error"] {
            background-color: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #4d1a1f, stop: 0.6 #00000000);
        }

        LogEntryWidget QLabel {
            color: #e0e0e0;
        }
    )";
}
