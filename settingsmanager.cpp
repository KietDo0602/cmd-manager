#include "settingsmanager.h"
#include <QStandardPaths>
#include <QDir>

SettingsManager* SettingsManager::m_instance = nullptr;

SettingsManager* SettingsManager::instance() {
    if (!m_instance) {
        m_instance = new SettingsManager();
    }
    return m_instance;
}

SettingsManager::SettingsManager(QObject* parent) : QObject(parent) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configPath + "/CMDManager");
    m_settings = new QSettings(configPath + "/CMDManager/settings.ini", QSettings::IniFormat, this);
}

// General settings
QString SettingsManager::getDefaultDirectory() const {
    return m_settings->value("general/defaultDirectory", QDir::homePath()).toString();
}

void SettingsManager::setDefaultDirectory(const QString& dir) {
    m_settings->setValue("general/defaultDirectory", dir);
}

bool SettingsManager::getAutoSave() const {
    return m_settings->value("general/autoSave", false).toBool();
}

void SettingsManager::setAutoSave(bool enabled) {
    m_settings->setValue("general/autoSave", enabled);
}

// Appearance settings
SettingsManager::Theme SettingsManager::getTheme() const {
    return static_cast<Theme>(m_settings->value("appearance/theme", Dark).toInt());
}

void SettingsManager::setTheme(Theme theme) {
    m_settings->setValue("appearance/theme", static_cast<int>(theme));
    emit themeChanged(theme);
}

int SettingsManager::getFontSize() const {
    return m_settings->value("appearance/fontSize", 10).toInt();
}

void SettingsManager::setFontSize(int size) {
    m_settings->setValue("appearance/fontSize", size);
    emit fontChanged();
}

QString SettingsManager::getFontFamily() const {
    return m_settings->value("appearance/fontFamily", "Arial").toString();
}

void SettingsManager::setFontFamily(const QString& family) {
    m_settings->setValue("appearance/fontFamily", family);
    emit fontChanged();
}

// Keyboard shortcuts
QString SettingsManager::getNewCommandShortcut() const {
    return m_settings->value("shortcuts/newCommand", "Ctrl+N").toString();
}

void SettingsManager::setNewCommandShortcut(const QString& shortcut) {
    m_settings->setValue("shortcuts/newCommand", shortcut);
    emit shortcutsChanged();
}

QString SettingsManager::getSaveCommandShortcut() const {
    return m_settings->value("shortcuts/saveCommand", "Ctrl+S").toString();
}

void SettingsManager::setSaveCommandShortcut(const QString& shortcut) {
    m_settings->setValue("shortcuts/saveCommand", shortcut);
    emit shortcutsChanged();
}

QString SettingsManager::getOpenCommandsShortcut() const {
    return m_settings->value("shortcuts/openCommands", "Ctrl+O").toString();
}

void SettingsManager::setOpenCommandsShortcut(const QString& shortcut) {
    m_settings->setValue("shortcuts/openCommands", shortcut);
    emit shortcutsChanged();
}

QString SettingsManager::getStartExecuteShortcut() const {
    return m_settings->value("shortcuts/startExecute", "F5").toString();
}

void SettingsManager::setStartExecuteShortcut(const QString& shortcut) {
    m_settings->setValue("shortcuts/startExecute", shortcut);
    emit shortcutsChanged();
}

void SettingsManager::applyThemeToAllWindows() {
    // Get the current theme stylesheet
    QString styleSheet = getCurrentThemeStyleSheet();
    
    // Apply to all top-level widgets
    QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topLevelWidgets) {
        if (widget->isVisible() || widget->isWindow()) {
            // Clear any existing stylesheet first
            widget->setStyleSheet("");
            // Then apply the new theme
            widget->setStyleSheet(styleSheet);
            
            // Also apply font
            QFont appFont(getFontFamily(), getFontSize());
            widget->setFont(appFont);
            
            // Apply to all child widgets
            QList<QWidget*> childWidgets = widget->findChildren<QWidget*>();
            for (QWidget* child : childWidgets) {
                child->setFont(appFont);
            }
            
            // Force update
            widget->update();
        }
    }
}

QString SettingsManager::getThemeStyleSheet(Theme theme, int fontSize, const QString& fontFamily) {
    QString styleSheet;
    switch (theme) {
        case Light:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #FFFDF9;   /* warm off-white */
                    color: #333333;              /* soft dark gray text */
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #E8F0FE;   /* soft blue background */
                    border: 1px solid #A0C4FF;   /* pastel blue border */
                    border-radius: 8px;
                    padding: 8px 16px;
                    color: #1F2937;              /* balanced gray text */
                }
                QPushButton:hover {
                    background-color: #D0E4FF;   /* slightly deeper blue */
                }
                QPushButton:pressed {
                    background-color: #BBD4FF;   /* pressed = more saturated */
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background: #FFFFFF;
                    border: 1px solid #BFD7EA;   /* cool blue-gray border */
                    border-radius: 8px;
                    padding: 8px;
                    color: #333333;
                    selection-background-color: #A0C4FF;
                    selection-color: #FFFFFF;
                }

                /* Labels */
                QLabel {
                    font-weight: 500;
                    color: #333333;
                }
                QLabel#preview {
                    background-color: #F6F6F6;
                    border: 1px solid #DADADA;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #E0E0E0;
                    border-radius: 10px;
                    margin-top: 10px;
                    padding: 8px;
                    font-weight: 500;
                    color: #333333;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #FFFFFF;
                    border: 1px solid #BFD7EA;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #333333;
                    min-height: 28px;
                }

                QComboBox:hover {
                    border: 1px solid #A0C4FF;
                }

                QComboBox:focus {
                    border: 1px solid #7FB2FF; /* subtle stronger blue */
                }

                /* Down arrow */
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #E0E0E0;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                    background-color: #F9FAFB;
                }

                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox::down-arrow:on { /* when pressed */
                    image: url(:/images/arrow_down.svg);
                }

                /* Drop-down popup */
                QComboBox QAbstractItemView {
                    background-color: #FFFFFF;
                    border: 1px solid #BFD7EA;
                    border-radius: 6px;
                    padding: 4px;
                    outline: 0;
                    selection-background-color: #A0C4FF;
                    selection-color: #FFFFFF;
                }

                /* Optional: scrollbar inside dropdown */
                QComboBox QAbstractScrollArea QScrollBar:vertical {
                    width: 8px;
                    background: #F6F6F6;
                    border-radius: 4px;
                }

                QComboBox QAbstractScrollArea QScrollBar::handle:vertical {
                    background: #D0D7DE;
                    border-radius: 4px;
                }

                QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {
                    background: #A0C4FF;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #FFFFFF;
                    border: 1px solid #BFD7EA;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #333333;
                    min-height: 28px;
                }

                QSpinBox:hover {
                    border: 1px solid #A0C4FF;
                }

                QSpinBox:focus {
                    border: 1px solid #7FB2FF;
                }

                /* Up/Down buttons */
                QSpinBox::up-button, QSpinBox::down-button {
                    subcontrol-origin: border;
                    width: 20px;
                    border-left: 1px solid #E0E0E0;
                    background-color: #F9FAFB;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                }

                QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                    background-color: #E8F0FE;
                }

                QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {
                    background-color: #D0E4FF;
                }

                /* Use existing SVG icons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Dim the same arrow when disabled */
                QSpinBox::up-arrow:disabled {
                    image: url(:/images/arrow_up.svg);
                    opacity: 0.4;
                }

                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QSpinBox::down-arrow:disabled {
                    image: url(:/images/arrow_down.svg);
                    opacity: 0.4;
                }

                /* List Widgets */
                QListWidget {
                    background: #FFFFFF;
                    border: 1px solid #E0E0E0;
                    border-radius: 8px;
                    color: #333333;
                }
                QListWidget::item:selected {
                    background: #A0C4FF;   /* pastel blue highlight */
                    color: #FFFFFF;
                    border-radius: 6px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #E0E0E0;
                    background: #FFFFFF;
                    border-radius: 8px;
                }
                QTabBar::tab {
                    background: #F6F8FB;   /* subtle off-white tab */
                    color: #333333;
                    padding: 8px 14px;
                    border: 1px solid #E0E0E0;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    color: #1F2937;
                    border-bottom: 2px solid #A0C4FF; /* elegant blue underline */
                }
            )";
            break;
        case Contrast:
            styleSheet = R"(
                QWidget {
                    background-color: #FDFDFB;  /* slightly brighter */
                    color: #1F2937;             /* stronger text contrast */
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #D6E4FF;
                    border: 1px solid #7FB2FF;
                    border-radius: 8px;
                    padding: 8px 16px;
                    color: #1F2937;
                }
                QPushButton:hover { background-color: #BBD4FF; }
                QPushButton:pressed { background-color: #A0C4FF; }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background: #FFFFFF;
                    border: 1px solid #A0C4FF;
                    border-radius: 8px;
                    padding: 8px;
                    color: #1F2937;
                    selection-background-color: #7FB2FF;
                    selection-color: #FFFFFF;
                }

                /* Labels */
                QLabel, QLabel#preview { color: #1F2937; }

                /* Group Boxes */
                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #C0D4EA;
                    border-radius: 10px;
                    color: #1F2937;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #FFFFFF;
                    border: 1px solid #A0C4FF;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #1F2937;
                }
                QComboBox:hover { border: 1px solid #7FB2FF; }
                QComboBox:focus { border: 1px solid #5FA0FF; }

                /* Drop-down & arrows remain the same but with stronger color highlights */
                QComboBox::drop-down { background-color: #F0F4FF; border-left: 1px solid #C0D4EA; }
                QComboBox::down-arrow, QSpinBox::up-arrow, QSpinBox::down-arrow { opacity: 1; }

                /* SpinBox */
                QSpinBox { border: 1px solid #A0C4FF; }
                QSpinBox:hover { border: 1px solid #7FB2FF; }
                QSpinBox:focus { border: 1px solid #5FA0FF; }
                QSpinBox::up-button:hover, QSpinBox::down-button:hover { background-color: #D6E4FF; }
                QSpinBox::up-button:pressed, QSpinBox::down-button:pressed { background-color: #BBD4FF; }

                /* List Widgets */
                QListWidget::item:selected { background: #7FB2FF; }

                /* Tabs */
                QTabBar::tab {
                    background: #F6F8FB;   /* subtle off-white tab */
                    color: #333333;
                    padding: 8px 14px;
                    border: 1px solid #E0E0E0;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    color: #1F2937;
                    border-bottom: 2px solid #7FB2FF; /* elegant blue underline */
                }

            )";
            break;
        default: // Dark
            styleSheet = R"(
                QWidget {
                    background-color: #121212;   /* deep dark background */
                    color: #E0E0E0;              /* light gray text */
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #1E3A8A;   /* dark blue */
                    border: 1px solid #3B82F6;   /* vibrant blue border */
                    border-radius: 8px;
                    padding: 8px 16px;
                    color: #FFFFFF;
                }
                QPushButton:hover {
                    background-color: #2563EB;
                }
                QPushButton:pressed {
                    background-color: #1D4ED8;
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background: #1E1E1E;
                    border: 1px solid #3B82F6;
                    border-radius: 8px;
                    padding: 8px;
                    color: #E0E0E0;
                    selection-background-color: #3B82F6;
                    selection-color: #FFFFFF;
                }

                /* Labels */
                QLabel {
                    font-weight: 500;
                    color: #E0E0E0;
                }
                QLabel#preview {
                    background-color: #1A1A1A;
                    border: 1px solid #333333;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #1E1E1E;
                    border: 1px solid #333333;
                    border-radius: 10px;
                    margin-top: 10px;
                    padding: 8px;
                    font-weight: 500;
                    color: #E0E0E0;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #1E1E1E;
                    border: 1px solid #3B82F6;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #E0E0E0;
                    min-height: 28px;
                }
                QComboBox:hover {
                    border: 1px solid #60A5FA;
                }
                QComboBox:focus {
                    border: 1px solid #93C5FD;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #333333;
                    background-color: #2A2A2A;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }
                QComboBox QAbstractItemView {
                    background-color: #1E1E1E;
                    border: 1px solid #3B82F6;
                    border-radius: 6px;
                    padding: 4px;
                    selection-background-color: #3B82F6;
                    selection-color: #FFFFFF;
                }

                /* Scrollbar */
                QComboBox QAbstractScrollArea QScrollBar:vertical {
                    width: 8px;
                    background: #2A2A2A;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical {
                    background: #3B82F6;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {
                    background: #60A5FA;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #1E1E1E;
                    border: 1px solid #3B82F6;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #E0E0E0;
                }
                QSpinBox:hover {
                    border: 1px solid #60A5FA;
                }
                QSpinBox:focus {
                    border: 1px solid #93C5FD;
                }
                QSpinBox::up-button, QSpinBox::down-button {
                    background-color: #2A2A2A;
                    border-left: 1px solid #333333;
                }
                QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                    background-color: #3B82F6;
                }
                QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {
                    background-color: #2563EB;
                }
                QSpinBox::up-arrow, QSpinBox::down-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::up-arrow:disabled, QSpinBox::down-arrow:disabled {
                    opacity: 0.4;
                }

                /* List Widgets */
                QListWidget {
                    background: #1E1E1E;
                    border: 1px solid #333333;
                    border-radius: 8px;
                    color: #E0E0E0;
                }
                QListWidget::item:selected {
                    background: #3B82F6;
                    color: #FFFFFF;
                    border-radius: 6px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #333333;
                    background: #1E1E1E;
                    border-radius: 8px;
                }
                QTabBar::tab {
                    background: #2A2A2A;
                    color: #E0E0E0;
                    padding: 8px 14px;
                    border: 1px solid #333333;
                }
                QTabBar::tab:selected {
                    background: #1E1E1E;
                    color: #FFFFFF;
                    border-bottom: 2px solid #3B82F6;
                }
            )";
            break;
    }
    
    return styleSheet;
}

QString SettingsManager::getCurrentThemeStyleSheet() const {
    return getThemeStyleSheet(getTheme(), getFontSize(), getFontFamily());
}
