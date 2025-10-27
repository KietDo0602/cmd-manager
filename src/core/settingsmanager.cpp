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
    qDebug() << configPath + "CMDManager/settings.ini";
    qDebug() << configPath + "CMDManager/commands.json";
    m_settings = new QSettings(configPath + "/CMDManager/settings.ini", QSettings::IniFormat, this);
}

// General settings
QString SettingsManager::getDefaultDirectory() const {
    return m_settings->value("%General/defaultDirectory", QDir::homePath()).toString();
}

void SettingsManager::setDefaultDirectory(const QString& dir) {
    m_settings->setValue("%General/defaultDirectory", dir);
}

bool SettingsManager::getAutoSave() const {
    return m_settings->value("%General/autoSave", false).toBool();
}

void SettingsManager::setAutoSave(bool enabled) {
    m_settings->setValue("%General/autoSave", enabled);
}

bool SettingsManager::getMinimizeToTray() const {
    return m_settings->value("%General/minimizeToTray", false).toBool();
}

void SettingsManager::setMinimizeToTray(bool minimize) {
    m_settings->setValue("%General/minimizeToTray", minimize);
}

// Language settings
SettingsManager::Language SettingsManager::getLanguage() const {
    return static_cast<Language>(m_settings->value("%General/language", English).toInt());
}

void SettingsManager::setLanguage(Language lang) {
    m_settings->setValue("%General/language", static_cast<int>(lang));
}

QString SettingsManager::getLanguageCode() const {
    return getLanguageCode(getLanguage());
}

QString SettingsManager::getLanguageCode(Language lang) {
    switch (lang) {
        case French: return "fr";
        case Spanish: return "es";
        case Chinese: return "zh";
        case Russian: return "ru";
        case Vietnamese: return "vi";
        default: return "en";
    }
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
                    background-color: #F5F7FA;
                    color: #1F2937;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #2563EB;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #FFFFFF;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #1E40AF;
                }
                QPushButton:pressed {
                    background-color: #1E3A8A;
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background-color: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #1F2937;
                    selection-background-color: #2563EB;
                    selection-color: #FFFFFF;
                }

                /* Labels */
                QLabel {
                    font-weight: 500;
                    color: #4B5563;
                }
                QLabel#preview {
                    background-color: #F0F0F0;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #1F2937;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #1F2937;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #2563EB;
                }
                QComboBox:focus {
                    border: 1px solid #3B82F6;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #D1D5DB;
                    background-color: #F5F7FA;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Drop-down popup */
                QComboBox QAbstractItemView {
                    background-color: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    selection-background-color: #2563EB;
                    selection-color: #FFFFFF;
                }

                /* Scrollbar */
                QScrollBar:vertical {
                    width: 8px;
                    background: #F0F0F0;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #2563EB;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #3B82F6;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #1F2937;
                }
                QSpinBox:hover {
                    border: 1px solid #2563EB;
                }
                QSpinBox:focus {
                    border: 1px solid #3B82F6;
                }

                /* Up/Down buttons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Checkboxes */
                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #2563EB;
                    background-color: #FFFFFF;
                    border-radius: 4px;
                }
                QCheckBox::indicator:hover {
                    background-color: #E0E7FF;
                    border: 2px solid #3B82F6;
                }
                QCheckBox::indicator:checked {
                    background-color: #2563EB;
                    border: 2px solid #2563EB;
                }
                QCheckBox::indicator:checked:hover {
                    background-color: #3B82F6;
                    border: 2px solid #1E3A8A;
                }
                QCheckBox::indicator:disabled {
                    background-color: #CBD5E1;
                    border: 2px solid #94A3B8;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #2563EB;
                }

                /* List Widgets */
                QListWidget {
                    background: #FFFFFF;
                    border: 1px solid #D1D5DB;
                    border-radius: 6px;
                    color: #1F2937;
                }
                QListWidget::item:selected {
                    background: #2563EB;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #D1D5DB;
                    background: #FFFFFF;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #F0F0F0;
                    color: #1F2937;
                    padding: 8px 16px;
                    border: 1px solid #D1D5DB;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    border-bottom: 2px solid #2563EB;
                }
            )";
            break;
        case SolarizedDark:
            styleSheet = R"(
                QWidget {
                    background-color: #002b36;
                    color: #839496;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #268bd2;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #fdf6e3;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #3693e0;
                }
                QPushButton:pressed {
                    background-color: #0f5fcc;
                }

                QLineEdit, QTextEdit {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #eee8d5;
                    selection-background-color: #268bd2;
                    selection-color: #fdf6e3;
                }

                QLabel {
                    font-weight: 500;
                    color: #93a1a1;
                }
                QLabel#preview {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #839496;
                }

                QComboBox {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #eee8d5;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #268bd2;
                }
                QComboBox:focus {
                    border: 1px solid #93a1a1;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #586e75;
                    background-color: #073642;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    selection-background-color: #268bd2;
                    selection-color: #fdf6e3;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #073642;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #268bd2;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #3693e0;
                }

                QSpinBox {
                    background-color: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #eee8d5;
                }
                QSpinBox:hover {
                    border: 1px solid #268bd2;
                }
                QSpinBox:focus {
                    border: 1px solid #93a1a1;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #268bd2;
                    background-color: #002b36;
                    border-radius: 4px;
                }
                QCheckBox::indicator:hover {
                    background-color: #073642;
                    border: 2px solid #93a1a1;
                }
                QCheckBox::indicator:checked {
                    background-color: #268bd2;
                    border: 2px solid #268bd2;
                }
                QCheckBox::indicator:checked:hover {
                    background-color: #0f5fcc;
                    border: 2px solid #93a1a1;
                }
                QCheckBox::indicator:disabled {
                    background-color: #586e75;
                    border: 2px solid #586e75;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #268bd2;
                }

                QListWidget {
                    background: #073642;
                    border: 1px solid #586e75;
                    border-radius: 6px;
                    color: #eee8d5;
                }
                QListWidget::item:selected {
                    background: #268bd2;
                    color: #fdf6e3;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #586e75;
                    background: #073642;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #002b36;
                    color: #eee8d5;
                    padding: 8px 16px;
                    border: 1px solid #586e75;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #073642;
                    border-bottom: 2px solid #268bd2;
                }
            )";
            break;
        case SolarizedLight:
            styleSheet = R"(
                QWidget {
                    background-color: #fdf6e3;
                    color: #657b83;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #268bd2;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #fdf6e3;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #3693e0;
                }
                QPushButton:pressed {
                    background-color: #0f5fcc;
                }

                QLineEdit, QTextEdit {
                    background-color: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #002b36;
                    selection-background-color: #268bd2;
                    selection-color: #fdf6e3;
                }

                QLabel {
                    font-weight: 500;
                    color: #586e75;
                }
                QLabel#preview {
                    background-color: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #fdf6e3;
                    border: 1px solid #93a1a1;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #657b83;
                }

                QComboBox {
                    background-color: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #002b36;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #268bd2;
                }
                QComboBox:focus {
                    border: 1px solid #93a1a1;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #93a1a1;
                    background-color: #eee8d5;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    selection-background-color: #268bd2;
                    selection-color: #fdf6e3;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #eee8d5;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #268bd2;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #3693e0;
                }

                QSpinBox {
                    background-color: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #002b36;
                }
                QSpinBox:hover {
                    border: 1px solid #268bd2;
                }
                QSpinBox:focus {
                    border: 1px solid #93a1a1;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #268bd2;
                    background-color: #fdf6e3;
                    border-radius: 4px;
                }
                QCheckBox::indicator:hover {
                    background-color: #eee8d5;
                    border: 2px solid #93a1a1;
                }
                QCheckBox::indicator:checked {
                    background-color: #268bd2;
                    border: 2px solid #268bd2;
                }
                QCheckBox::indicator:checked:hover {
                    background-color: #0f5fcc;
                    border: 2px solid #93a1a1;
                }
                QCheckBox::indicator:disabled {
                    background-color: #93a1a1;
                    border: 2px solid #93a1a1;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #268bd2;
                }

                QListWidget {
                    background: #eee8d5;
                    border: 1px solid #93a1a1;
                    border-radius: 6px;
                    color: #002b36;
                }
                QListWidget::item:selected {
                    background: #268bd2;
                    color: #fdf6e3;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #93a1a1;
                    background: #fdf6e3;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #eee8d5;
                    color: #002b36;
                    padding: 8px 16px;
                    border: 1px solid #93a1a1;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #fdf6e3;
                    border-bottom: 2px solid #268bd2;
                }
            )";
            break;
        case Pastel:
            styleSheet = R"(
                QWidget {
                    background-color: #F7F5F9;
                    color: #404040;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #A1C4FD;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #FFFFFF;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #C2E9FB;
                    color: #333333;
                }
                QPushButton:pressed {
                    background-color: #7AB8F5;
                    color: #FFFFFF;
                }

                QLineEdit, QTextEdit {
                    background-color: #FAF9FC;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #404040;
                    selection-background-color: #A1C4FD;
                    selection-color: #FFFFFF;
                }

                QLabel {
                    font-weight: 500;
                    color: #555555;
                }
                QLabel#preview {
                    background-color: #FFFFFF;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D8D3E0;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #404040;
                }

                QComboBox {
                    background-color: #FAF9FC;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #404040;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #A1C4FD;
                }
                QComboBox:focus {
                    border: 1px solid #7AB8F5;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #D8D3E0;
                    background-color: #FAF9FC;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #FFFFFF;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    selection-background-color: #A1C4FD;
                    selection-color: #FFFFFF;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #FAF9FC;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #A1C4FD;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #C2E9FB;
                }

                QSpinBox {
                    background-color: #FAF9FC;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #404040;
                }
                QSpinBox:hover {
                    border: 1px solid #A1C4FD;
                }
                QSpinBox:focus {
                    border: 1px solid #7AB8F5;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #A1C4FD;
                    background-color: #FFFFFF;
                    border-radius: 4px;
                }
                QCheckBox::indicator:hover {
                    background-color: #FAF9FC;
                    border: 2px solid #7AB8F5;
                }
                QCheckBox::indicator:checked {
                    background-color: #A1C4FD;
                    border: 2px solid #A1C4FD;
                }
                QCheckBox::indicator:checked:hover {
                    background-color: #7AB8F5;
                    border: 2px solid #A1C4FD;
                }
                QCheckBox::indicator:disabled {
                    background-color: #E5E5E5;
                    border: 2px solid #E5E5E5;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #7AB8F5;
                }

                QListWidget {
                    background: #FFFFFF;
                    border: 1px solid #D8D3E0;
                    border-radius: 6px;
                    color: #404040;
                }
                QListWidget::item:selected {
                    background: #A1C4FD;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #D8D3E0;
                    background: #FFFFFF;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #FAF9FC;
                    color: #404040;
                    padding: 8px 16px;
                    border: 1px solid #D8D3E0;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    border-bottom: 2px solid #A1C4FD;
                }
            )";
            break;
        case Nature:
            styleSheet = R"(
                QWidget {
                    background-color: #F3F7F2;
                    color: #2E3A29;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #6AA84F;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #FFFFFF;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #8CC46B;
                }
                QPushButton:pressed {
                    background-color: #4E7D3A;
                }

                QLineEdit, QTextEdit {
                    background-color: #FAFBF9;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #2E3A29;
                    selection-background-color: #6AA84F;
                    selection-color: #FFFFFF;
                }

                QLabel {
                    font-weight: 500;
                    color: #374733;
                }
                QLabel#preview {
                    background-color: #FFFFFF;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #C9D7C4;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #2E3A29;
                }

                QComboBox {
                    background-color: #FAFBF9;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #2E3A29;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #6AA84F;
                }
                QComboBox:focus {
                    border: 1px solid #8CC46B;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #C9D7C4;
                    background-color: #FAFBF9;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #FFFFFF;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    selection-background-color: #6AA84F;
                    selection-color: #FFFFFF;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #E9EFE7;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #6AA84F;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #8CC46B;
                }

                QSpinBox {
                    background-color: #FAFBF9;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #2E3A29;
                }
                QSpinBox:hover {
                    border: 1px solid #6AA84F;
                }
                QSpinBox:focus {
                    border: 1px solid #8CC46B;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #6AA84F;
                    background-color: #FFFFFF;
                    border-radius: 4px;
                }
                QCheckBox::indicator:hover {
                    background-color: #E9EFE7;
                    border: 2px solid #8CC46B;
                }
                QCheckBox::indicator:checked {
                    background-color: #6AA84F;
                    border: 2px solid #6AA84F;
                }
                QCheckBox::indicator:checked:hover {
                    background-color: #4E7D3A;
                    border: 2px solid #8CC46B;
                }
                QCheckBox::indicator:disabled {
                    background-color: #B8C4B1;
                    border: 2px solid #B8C4B1;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #6AA84F;
                }

                QListWidget {
                    background: #FFFFFF;
                    border: 1px solid #C9D7C4;
                    border-radius: 6px;
                    color: #2E3A29;
                }
                QListWidget::item:selected {
                    background: #6AA84F;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #C9D7C4;
                    background: #FFFFFF;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #FAFBF9;
                    color: #2E3A29;
                    padding: 8px 16px;
                    border: 1px solid #C9D7C4;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    border-bottom: 2px solid #6AA84F;
                }
            )";
            break;
        case Nord:
            styleSheet = R"(
                QWidget {
                    background-color: #2E3440;
                    color: #D8DEE9;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #5E81AC;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #ECEFF4;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #81A1C1;
                }
                QPushButton:pressed {
                    background-color: #4C688C;
                }

                QLineEdit, QTextEdit {
                    background-color: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #ECEFF4;
                    selection-background-color: #81A1C1;
                    selection-color: #2E3440;
                }

                QLabel {
                    font-weight: 500;
                    color: #E5E9F0;
                }
                QLabel#preview {
                    background-color: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #2E3440;
                    border: 1px solid #4C566A;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #D8DEE9;
                }

                QComboBox {
                    background-color: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #ECEFF4;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #81A1C1;
                }
                QComboBox:focus {
                    border: 1px solid #88C0D0;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #4C566A;
                    background-color: #3B4252;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    selection-background-color: #81A1C1;
                    selection-color: #2E3440;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #3B4252;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #5E81AC;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #81A1C1;
                }

                QSpinBox {
                    background-color: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #ECEFF4;
                }
                QSpinBox:hover {
                    border: 1px solid #81A1C1;
                }
                QSpinBox:focus {
                    border: 1px solid #88C0D0;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #5E81AC;
                    background-color: #2E3440;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #3B4252;
                    border: 2px solid #81A1C1;
                }

                QCheckBox::indicator:checked {
                    background-color: #81A1C1;
                    border: 2px solid #81A1C1;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #88C0D0;
                    border: 2px solid #88C0D0;
                }

                QCheckBox::indicator:disabled {
                    background-color: #4C566A;
                    border: 2px solid #4C566A;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #81A1C1;
                }

                QListWidget {
                    background: #3B4252;
                    border: 1px solid #4C566A;
                    border-radius: 6px;
                    color: #ECEFF4;
                }
                QListWidget::item:selected {
                    background: #5E81AC;
                    color: #ECEFF4;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #4C566A;
                    background: #2E3440;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #3B4252;
                    color: #ECEFF4;
                    padding: 8px 16px;
                    border: 1px solid #4C566A;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #2E3440;
                    border-bottom: 2px solid #81A1C1;
                }
            )";
            break;
        case Paper:
            styleSheet = R"(
                QWidget {
                    background-color: #FAFAFA;
                    color: #2E2E2E;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #E9ECEF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #2E2E2E;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #DEE2E6;
                }
                QPushButton:pressed {
                    background-color: #CED4DA;
                }

                QLineEdit, QTextEdit {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #2E2E2E;
                    selection-background-color: #4A90E2;
                    selection-color: #FFFFFF;
                }

                QLabel {
                    font-weight: 500;
                    color: #555555;
                }
                QLabel#preview {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #2E2E2E;
                }

                QComboBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #2E2E2E;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #4A90E2;
                }
                QComboBox:focus {
                    border: 1px solid #B3D4FC;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #D0D0D0;
                    background-color: #FFFFFF;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    selection-background-color: #4A90E2;
                    selection-color: #FFFFFF;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #F1F1F1;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #D0D0D0;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #B3D4FC;
                }

                QSpinBox {
                    background-color: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #2E2E2E;
                }
                QSpinBox:hover {
                    border: 1px solid #4A90E2;
                }
                QSpinBox:focus {
                    border: 1px solid #B3D4FC;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #4A90E2;
                    background-color: #FFFFFF;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #E9ECEF;
                    border: 2px solid #4A90E2;
                }

                QCheckBox::indicator:checked {
                    background-color: #4A90E2;
                    border: 2px solid #4A90E2;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #367DD9;
                    border: 2px solid #367DD9;
                }

                QCheckBox::indicator:disabled {
                    background-color: #CED4DA;
                    border: 2px solid #CED4DA;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #4A90E2;
                }

                QListWidget {
                    background: #FFFFFF;
                    border: 1px solid #D0D0D0;
                    border-radius: 6px;
                    color: #2E2E2E;
                }
                QListWidget::item:selected {
                    background: #4A90E2;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #D0D0D0;
                    background: #FFFFFF;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #F5F5F5;
                    color: #2E2E2E;
                    padding: 8px 16px;
                    border: 1px solid #D0D0D0;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #FFFFFF;
                    border-bottom: 2px solid #4A90E2;
                }
            )";
            break;
        case RosePine:
            styleSheet = R"(
                QWidget {
                    background-color: #191724;
                    color: #e0def4;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #31748f;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #e0def4;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #9ccfd8;
                    color: #191724;
                }
                QPushButton:pressed {
                    background-color: #28647A;
                }

                QLineEdit, QTextEdit {
                    background-color: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #e0def4;
                    selection-background-color: #31748f;
                    selection-color: #e0def4;
                }

                QLabel {
                    font-weight: 500;
                    color: #908caa;
                }
                QLabel#preview {
                    background-color: #26233a;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #e0def4;
                }

                QComboBox {
                    background-color: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #e0def4;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #31748f;
                }
                QComboBox:focus {
                    border: 1px solid #eb6f92;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #403d52;
                    background-color: #1f1d2e;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    selection-background-color: #31748f;
                    selection-color: #e0def4;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #1f1d2e;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #31748f;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #9ccfd8;
                }

                QSpinBox {
                    background-color: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #e0def4;
                }
                QSpinBox:hover {
                    border: 1px solid #31748f;
                }
                QSpinBox:focus {
                    border: 1px solid #eb6f92;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #31748f;
                    background-color: #191724;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #26233a;
                    border: 2px solid #9ccfd8;
                }

                QCheckBox::indicator:checked {
                    background-color: #eb6f92;
                    border: 2px solid #eb6f92;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #eb6f92;
                    border: 2px solid #e0def4;
                }

                QCheckBox::indicator:disabled {
                    background-color: #403d52;
                    border: 2px solid #403d52;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #9ccfd8;
                }

                QListWidget {
                    background: #1f1d2e;
                    border: 1px solid #403d52;
                    border-radius: 6px;
                    color: #e0def4;
                }
                QListWidget::item:selected {
                    background: #31748f;
                    color: #e0def4;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #403d52;
                    background: #191724;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #26233a;
                    color: #e0def4;
                    padding: 8px 16px;
                    border: 1px solid #403d52;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #1f1d2e;
                    border-bottom: 2px solid #eb6f92;
                }
            )";
            break;
        case NightOwl:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #011627;   /* deep navy */
                    color: #d6deeb;              /* soft light blue text */
                    font-family: "Fira Code", "Consolas", "Segoe UI", sans-serif;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #1d3b53;   /* blue-gray base */
                    color: #d6deeb;
                    border: 1px solid #0b2942;
                    border-radius: 6px;
                    padding: 6px 12px;
                }

                QPushButton:hover {
                    background-color: #2a547e;   /* lighter blue on hover */
                }

                QPushButton:pressed {
                    background-color: #224761;   /* darker pressed tone */
                }

                QPushButton:disabled {
                    background-color: #0b2942;
                    color: #5f7e97;
                }

                /* Line Edits / Text Fields */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #011221;   /* slightly darker than main bg */
                    color: #d6deeb;
                    border: 1px solid #1d3b53;
                    border-radius: 4px;
                    selection-background-color: #2a547e;
                    selection-color: #ffffff;
                    padding: 4px;
                }

                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 1px solid #7e57c2;   /* violet highlight */
                }

                /* Labels */
                QLabel {
                    color: #d6deeb;
                    background: transparent;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #011221;
                    color: #d6deeb;
                    border: 1px solid #1d3b53;
                    border-radius: 4px;
                    padding: 4px 6px;
                }

                QComboBox:hover {
                    border: 1px solid #2a547e;
                }

                QComboBox QAbstractItemView {
                    background-color: #011627;
                    border: 1px solid #1d3b53;
                    selection-background-color: #2a547e;
                    selection-color: #ffffff;
                }

                /* Scrollbars */
                QScrollBar:vertical {
                    background: #011627;
                    width: 10px;
                    margin: 2px;
                }

                QScrollBar::handle:vertical {
                    background: #2a547e;
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical:hover {
                    background: #4c8eda;
                }

                QScrollBar::add-line:vertical,
                QScrollBar::sub-line:vertical {
                    background: none;
                    height: 0;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #1d3b53;
                    background: #011221;
                }

                QTabBar::tab {
                    background: #011627;
                    color: #d6deeb;
                    padding: 6px 10px;
                    border-top-left-radius: 4px;
                    border-top-right-radius: 4px;
                }

                QTabBar::tab:selected {
                    background: #2a547e;
                    color: #ffffff;
                }

                /* Checkboxes & Radio Buttons */
                QCheckBox, QRadioButton {
                    color: #d6deeb;
                    spacing: 6px;
                }

                QCheckBox::indicator, QRadioButton::indicator {
                    width: 14px;
                    height: 14px;
                    border: 1px solid #1d3b53;
                    background: #011221;
                }

                QCheckBox::indicator:checked, QRadioButton::indicator:checked {
                    background-color: #2a547e;
                    border: 1px solid #7e57c2;
                }

                /* Menus */
                QMenu {
                    background-color: #011627;
                    border: 1px solid #1d3b53;
                    color: #d6deeb;
                }

                QMenu::item:selected {
                    background-color: #2a547e;
                }

                /* Tooltips */
                QToolTip {
                    background-color: #2a547e;
                    color: #ffffff;
                    border: 1px solid #0b2942;
                    border-radius: 4px;
                }

                /* Progress Bars */
                QProgressBar {
                    background-color: #011221;
                    border: 1px solid #1d3b53;
                    border-radius: 4px;
                    text-align: center;
                    color: #d6deeb;
                }

                QProgressBar::chunk {
                    background-color: #7e57c2;   /* violet progress */
                    border-radius: 4px;
                }
            )";
            break;
        case Horizon:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #1C1E26;   /* deep dusk background */
                    color: #E0E0E0;              /* soft gray text */
                    font-family: "Fira Code", "Segoe UI", "Consolas", sans-serif;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #2E303E;   /* muted purple-gray */
                    color: #E0E0E0;
                    border: 1px solid #3D4050;
                    border-radius: 6px;
                    padding: 6px 12px;
                }

                QPushButton:hover {
                    background-color: #44475A;   /* hover: lighter purple tone */
                }

                QPushButton:pressed {
                    background-color: #393B4A;   /* pressed: deeper purple tone */
                }

                QPushButton:disabled {
                    background-color: #2E303E;
                    color: #7A7E8A;
                }

                /* Line Edits / Text Fields */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #232530;   /* dark violet-blue */
                    color: #E0E0E0;
                    border: 1px solid #3D4050;
                    border-radius: 4px;
                    selection-background-color: #E95678; /* pink selection */
                    selection-color: #FFFFFF;
                    padding: 4px;
                }

                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 1px solid #FAB795;   /* coral focus border */
                }

                /* Labels */
                QLabel {
                    color: #E0E0E0;
                    background: transparent;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #232530;
                    color: #E0E0E0;
                    border: 1px solid #3D4050;
                    border-radius: 4px;
                    padding: 4px 6px;
                }

                QComboBox:hover {
                    border: 1px solid #E95678;  /* pink border on hover */
                }

                QComboBox QAbstractItemView {
                    background-color: #1C1E26;
                    border: 1px solid #3D4050;
                    selection-background-color: #FAB795;
                    selection-color: #1C1E26;
                }

                /* Scrollbars */
                QScrollBar:vertical {
                    background: #1C1E26;
                    width: 10px;
                    margin: 2px;
                }

                QScrollBar::handle:vertical {
                    background: #E95678;  /* pink handle */
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical:hover {
                    background: #FAB795;  /* coral hover */
                }

                QScrollBar::add-line:vertical,
                QScrollBar::sub-line:vertical {
                    background: none;
                    height: 0;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #3D4050;
                    background: #232530;
                }

                QTabBar::tab {
                    background: #2E303E;
                    color: #E0E0E0;
                    padding: 6px 10px;
                    border-top-left-radius: 4px;
                    border-top-right-radius: 4px;
                }

                QTabBar::tab:selected {
                    background: #E95678;   /* pink active tab */
                    color: #1C1E26;
                }

                /* Checkboxes & Radio Buttons */
                QCheckBox, QRadioButton {
                    color: #E0E0E0;
                    spacing: 6px;
                }

                QCheckBox::indicator, QRadioButton::indicator {
                    width: 14px;
                    height: 14px;
                    border: 1px solid #3D4050;
                    background: #232530;
                }

                QCheckBox::indicator:checked, QRadioButton::indicator:checked {
                    background-color: #FAB795;  /* coral check */
                    border: 1px solid #E95678;
                }

                /* Menus */
                QMenu {
                    background-color: #1C1E26;
                    border: 1px solid #3D4050;
                    color: #E0E0E0;
                }

                QMenu::item:selected {
                    background-color: #E95678;
                    color: #1C1E26;
                }

                /* Tooltips */
                QToolTip {
                    background-color: #2E303E;
                    color: #FFFFFF;
                    border: 1px solid #E95678;
                    border-radius: 4px;
                }

                /* Progress Bars */
                QProgressBar {
                    background-color: #232530;
                    border: 1px solid #3D4050;
                    border-radius: 4px;
                    text-align: center;
                    color: #E0E0E0;
                }

                QProgressBar::chunk {
                    background-color: #E95678;   /* vivid pink chunk */
                    border-radius: 4px;
                }
            )";
            break;
        case Monochrome:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #121212;   /* deep neutral black-gray */
                    color: #E0E0E0;              /* light gray text */
                    font-family: "Segoe UI", "Helvetica Neue", "Arial", sans-serif;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #1E1E1E;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    padding: 8px 16px;
                    color: #E0E0E0;
                    font-weight: 500;
                }

                QPushButton:hover {
                    background-color: #2A2A2A;
                }

                QPushButton:pressed {
                    background-color: #3C3C3C;
                }

                QPushButton:disabled {
                    background-color: #1E1E1E;
                    color: #666666;
                    border: 1px solid #2A2A2A;
                }

                /* Input Fields */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #E0E0E0;
                    selection-background-color: #FFFFFF;
                    selection-color: #000000;
                }

                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 1px solid #AAAAAA;
                }

                /* Labels */
                QLabel {
                    color: #D0D0D0;
                    font-weight: 500;
                }

                QLabel#preview {
                    background-color: #181818;
                    border: 1px solid #2A2A2A;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Box */
                QGroupBox {
                    background-color: #1E1E1E;
                    border: 1px solid #3C3C3C;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    color: #E0E0E0;
                    font-weight: 500;
                }

                /* Combo Box */
                QComboBox {
                    background-color: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                    min-height: 30px;
                }

                QComboBox:hover {
                    border: 1px solid #AAAAAA;
                }

                QComboBox:focus {
                    border: 1px solid #FFFFFF;
                }

                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #3C3C3C;
                    background-color: #1A1A1A;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }

                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    selection-background-color: #FFFFFF;
                    selection-color: #000000;
                }

                /* Scrollbars */
                QScrollBar:vertical {
                    width: 8px;
                    background: #181818;
                    border-radius: 4px;
                }

                QScrollBar::handle:vertical {
                    background: #666666;
                    border-radius: 4px;
                }

                QScrollBar::handle:vertical:hover {
                    background: #AAAAAA;
                }

                /* Spin Box */
                QSpinBox {
                    background-color: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                }

                QSpinBox:hover {
                    border: 1px solid #AAAAAA;
                }

                QSpinBox:focus {
                    border: 1px solid #FFFFFF;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Checkboxes */
                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #AAAAAA;
                    background-color: #121212;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #1E1E1E;
                    border: 2px solid #FFFFFF;
                }

                QCheckBox::indicator:checked {
                    background-color: #FFFFFF;
                    border: 2px solid #FFFFFF;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #AAAAAA;
                    border: 2px solid #FFFFFF;
                }

                QCheckBox::indicator:disabled {
                    background-color: #444444;
                    border: 2px solid #444444;
                }

                /* Lists */
                QListWidget {
                    background: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    color: #FFFFFF;
                }

                QListWidget::item:selected {
                    background: #FFFFFF;
                    color: #000000;
                    border-radius: 4px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #3C3C3C;
                    background: #1E1E1E;
                    border-radius: 6px;
                }

                QTabBar::tab {
                    background: #1A1A1A;
                    color: #FFFFFF;
                    padding: 8px 16px;
                    border: 1px solid #3C3C3C;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }

                QTabBar::tab:selected {
                    background: #FFFFFF;
                    color: #000000;
                    border-bottom: 2px solid #FFFFFF;
                }

                /* Tooltips */
                QToolTip {
                    background-color: #1E1E1E;
                    color: #E0E0E0;
                    border: 1px solid #3C3C3C;
                    border-radius: 4px;
                }

                /* Progress Bar */
                QProgressBar {
                    background-color: #1A1A1A;
                    border: 1px solid #3C3C3C;
                    border-radius: 6px;
                    text-align: center;
                    color: #E0E0E0;
                }

                QProgressBar::chunk {
                    background-color: #FFFFFF;
                    border-radius: 6px;
                }
            )";
            break;
        case Neon:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #0A0A0F;   /* deep black-blue background */
                    color: #E0E0E0;              /* bright neutral text */
                    font-family: "Segoe UI", "Roboto Mono", "Consolas", monospace;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #111122;
                    color: #00FFFF;              /* neon cyan */
                    border: 1px solid #00FFFF;
                    border-radius: 8px;
                    padding: 8px 16px;
                    font-weight: 600;
                }

                QPushButton:hover {
                    background-color: #141438;
                    border: 1px solid #FF00FF;   /* neon magenta glow */
                    color: #FF00FF;
                }

                QPushButton:pressed {
                    background-color: #1E004E;
                    color: #FFFFFF;
                    border: 1px solid #FFFFFF;
                }

                QPushButton:disabled {
                    background-color: #0D0D1A;
                    color: #555555;
                    border: 1px solid #222244;
                }

                /* Input Fields */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #141420;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #FFFFFF;
                    selection-background-color: #FF00FF;
                    selection-color: #000000;
                }

                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 1px solid #39FF14;   /* neon green accent */
                    box-shadow: 0 0 10px #39FF14;
                }

                /* Labels */
                QLabel {
                    color: #E0E0E0;
                    font-weight: 500;
                }

                QLabel#preview {
                    background-color: #0D0D1A;
                    border: 1px solid #222244;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* GroupBox */
                QGroupBox {
                    background-color: #111122;
                    border: 1px solid #222244;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    color: #00FFFF;
                    font-weight: 600;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #141420;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                    min-height: 30px;
                }

                QComboBox:hover {
                    border: 1px solid #FF00FF;
                    color: #FF00FF;
                }

                QComboBox:focus {
                    border: 1px solid #39FF14;
                }

                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #00FFFF;
                    background-color: #0F0F20;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }

                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #0A0A10;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    selection-background-color: #FF00FF;
                    selection-color: #FFFFFF;
                }

                /* Scrollbars */
                QScrollBar:vertical {
                    width: 10px;
                    background: #0F0F20;
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical {
                    background: #00FFFF;
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical:hover {
                    background: #FF00FF;
                }

                QScrollBar::add-line:vertical,
                QScrollBar::sub-line:vertical {
                    background: none;
                    height: 0;
                }

                /* SpinBox */
                QSpinBox {
                    background-color: #141420;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                }

                QSpinBox:hover {
                    border: 1px solid #FF00FF;
                }

                QSpinBox:focus {
                    border: 1px solid #39FF14;
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                /* CheckBoxes */
                QCheckBox::indicator {
                    width: 18px;
                    height: 18px;
                    border: 2px solid #00FFFF;
                    background-color: #0A0A0F;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    border: 2px solid #FF00FF;
                    background-color: #14142A;
                }

                QCheckBox::indicator:checked {
                    background-color: #39FF14;
                    border: 2px solid #39FF14;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #00FFAA;
                }

                /* List Widgets */
                QListWidget {
                    background: #0D0D18;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    color: #FFFFFF;
                }

                QListWidget::item:selected {
                    background: #FF00FF;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #00FFFF;
                    background: #111122;
                    border-radius: 6px;
                }

                QTabBar::tab {
                    background: #0D0D1A;
                    color: #00FFFF;
                    padding: 8px 16px;
                    border: 1px solid #00FFFF;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }

                QTabBar::tab:selected {
                    background: #FF00FF;
                    color: #FFFFFF;
                    border-bottom: 2px solid #39FF14;
                }

                /* Tooltips */
                QToolTip {
                    background-color: #0D0D18;
                    color: #39FF14;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                }

                /* Progress Bar */
                QProgressBar {
                    background-color: #0A0A0F;
                    border: 1px solid #00FFFF;
                    border-radius: 6px;
                    text-align: center;
                    color: #FFFFFF;
                }

                QProgressBar::chunk {
                    background-color: #FF00FF;
                    border-radius: 6px;
                    margin: 1px;
                }
            )";
            break;
        case Rainbow:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #1E1E1E;   /* dark neutral background */
                    color: #FFFFFF;              /* default white text */
                    font-family: "Segoe UI", "Consolas", "Fira Code", monospace;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #2A2A2A;
                    color: #FFFFFF;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    padding: 8px 16px;
                    font-weight: 500;
                }

                QPushButton:hover {
                    background-color: #3A3A3A;
                    border: 1px solid #FF5555;   /* red hover */
                    color: #FF5555;
                }

                QPushButton:pressed {
                    background-color: #444444;
                    color: #FFFF55;              /* yellow pressed */
                    border: 1px solid #FFFF55;
                }

                QPushButton:disabled {
                    background-color: #2A2A2A;
                    color: #555555;
                    border: 1px solid #444444;
                }

                /* Input Fields */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #FFFFFF;
                    selection-background-color: #FF5555;
                    selection-color: #FFFFFF;
                }

                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 1px solid #55FF55;   /* green focus */
                }

                /* Labels */
                QLabel {
                    color: #FFFFFF;
                    font-weight: 500;
                }

                QLabel#preview {
                    background-color: #2A2A2A;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Box */
                QGroupBox {
                    background-color: #2A2A2A;
                    border: 1px solid #555555;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    color: #FFFFFF;
                    font-weight: 500;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                    min-height: 30px;
                }

                QComboBox:hover {
                    border: 1px solid #FF55FF;   /* magenta hover */
                }

                QComboBox:focus {
                    border: 1px solid #5555FF;   /* blue focus */
                }

                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #888888;
                    background-color: #252525;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }

                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    selection-background-color: #55FFFF; /* cyan selection */
                    selection-color: #000000;
                }

                /* Scrollbars */
                QScrollBar:vertical {
                    width: 10px;
                    background: #1E1E1E;
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical {
                    background: #FF5555;   /* red handle */
                    border-radius: 5px;
                }

                QScrollBar::handle:vertical:hover {
                    background: #55FF55;   /* green hover */
                }

                QScrollBar::add-line:vertical,
                QScrollBar::sub-line:vertical {
                    background: none;
                    height: 0;
                }

                /* SpinBox */
                QSpinBox {
                    background-color: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                }

                QSpinBox:hover {
                    border: 1px solid #FFFF55;  /* yellow hover */
                }

                QSpinBox:focus {
                    border: 1px solid #FF55FF;  /* magenta focus */
                }

                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Checkboxes */
                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #FFFFFF;
                    background-color: #252525;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    border: 2px solid #FF55FF;   /* magenta hover */
                }

                QCheckBox::indicator:checked {
                    background-color: #55FFFF;   /* cyan check */
                    border: 2px solid #55FFFF;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #FFFF55;   /* yellow hover */
                }

                /* Lists */
                QListWidget {
                    background: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    color: #FFFFFF;
                }

                QListWidget::item:selected {
                    background: #FF55FF; /* magenta selection */
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #888888;
                    background: #2A2A2A;
                    border-radius: 6px;
                }

                QTabBar::tab {
                    background: #252525;
                    color: #FFFFFF;
                    padding: 8px 16px;
                    border: 1px solid #888888;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }

                QTabBar::tab:selected {
                    background: #55FFFF; /* cyan active tab */
                    color: #000000;
                    border-bottom: 2px solid #FF5555; /* red border */
                }

                /* Tooltips */
                QToolTip {
                    background-color: #2A2A2A;
                    color: #FFFFFF;
                    border: 1px solid #FF55FF;
                    border-radius: 4px;
                }

                /* Progress Bar */
                QProgressBar {
                    background-color: #252525;
                    border: 1px solid #888888;
                    border-radius: 6px;
                    text-align: center;
                    color: #FFFFFF;
                }

                QProgressBar::chunk {
                    background-color: #FF5555;  /* red chunk */
                    border-radius: 6px;
                    margin: 1px;
                }
            )";
            break;
        case Contrast:
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #000000;
                    color: #FFFFFF;
                    font-size: 14px;
                }

                /* Buttons */
                QPushButton {
                    background-color: #FFFF00;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    padding: 10px 20px;
                    color: #000000;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #FFD700;
                }
                QPushButton:pressed {
                    background-color: #FFA500;
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background-color: #000000;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    padding: 8px 12px;
                    color: #FFFFFF;
                    selection-background-color: #FFFF00;
                    selection-color: #000000;
                }

                /* Labels */
                QLabel {
                    font-weight: bold;
                    color: #FFFFFF;
                }
                QLabel#preview {
                    background-color: #1A1A1A;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #000000;
                    border: 2px solid #FFFFFF;
                    border-radius: 6px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: bold;
                    color: #FFFFFF;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #000000;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 2px solid #FFFF00;
                }
                QComboBox:focus {
                    border: 2px solid #FFA500;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 2px solid #FFFFFF;
                    background-color: #000000;
                    border-top-right-radius: 4px;
                    border-bottom-right-radius: 4px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                /* Drop-down popup */
                QComboBox QAbstractItemView {
                    background-color: #000000;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    selection-background-color: #FFFF00;
                    selection-color: #000000;
                }

                /* Scrollbar */
                QScrollBar:vertical {
                    width: 10px;
                    background: #000000;
                    border: 2px solid #FFFFFF;
                }
                QScrollBar::handle:vertical {
                    background: #FFFF00;
                }
                QScrollBar::handle:vertical:hover {
                    background: #FFA500;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #000000;
                    border: 2px solid #FFFFFF;
                    border-radius: 4px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                }
                QSpinBox:hover {
                    border: 2px solid #FFFF00;
                }
                QSpinBox:focus {
                    border: 2px solid #FFA500;
                }

                /* Up/Down buttons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #FFFF00;
                    background-color: #000000;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #333333;
                    border: 2px solid #FFD700;
                }

                QCheckBox::indicator:checked {
                    background-color: #FFFF00;
                    border: 2px solid #FFFF00;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #FFD700;
                    border: 2px solid #FF4500;
                }

                QCheckBox::indicator:disabled {
                    background-color: #808080;
                    border: 2px solid #FFFF00;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 2px solid #FFFF00;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 2px solid #FFFFFF;
                    background: #000000;
                    border-radius: 4px;
                }
                QTabBar::tab {
                    background: #1A1A1A;
                    color: #FFFFFF;
                    padding: 8px 16px;
                    border: 2px solid #FFFFFF;
                    border-top-left-radius: 4px;
                    border-top-right-radius: 4px;
                }
                QTabBar::tab:selected {
                    background: #000000;
                    border-bottom: 2px solid #FFFF00;
                }
            )";
            break;
        default: // Dark
            styleSheet = R"(
                QWidget {
                    background-color: #020D22;
                    color: #E0E0E0;
                    font-size: 14px;
                }

                QPushButton {
                    background-color: #007BFF;
                    border: none;
                    border-radius: 6px;
                    padding: 10px 20px;
                    color: #FFFFFF;
                    font-weight: 500;
                }
                QPushButton:hover {
                    background-color: #5CAEFF;
                }
                QPushButton:pressed {
                    background-color: #0F5FCC;
                }

                QLineEdit, QTextEdit {
                    background-color: #111A2E;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #FFFFFF;
                    selection-background-color: #007BFF;
                    selection-color: #FFFFFF;
                }

                QLabel {
                    font-weight: 500;
                    color: #CCCCCC;
                }
                QLabel#preview {
                    background-color: #1A2233;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    padding: 6px;
                }

                QGroupBox {
                    background-color: #0F172A;
                    border: 1px solid #1F2F4F;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding: 10px;
                    font-weight: 500;
                    color: #E0E0E0;
                }

                QComboBox {
                    background-color: #111A2E;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                    min-height: 30px;
                }
                QComboBox:hover {
                    border: 1px solid #007BFF;
                }
                QComboBox:focus {
                    border: 1px solid #5CAEFF;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #2A3A5A;
                    background-color: #111A2E;
                    border-top-right-radius: 6px;
                    border-bottom-right-radius: 6px;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #111A2E;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    selection-background-color: #007BFF;
                    selection-color: #FFFFFF;
                }

                QScrollBar:vertical {
                    width: 8px;
                    background: #0F172A;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background: #007BFF;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #5CAEFF;
                }

                QSpinBox {
                    background-color: #111A2E;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #FFFFFF;
                }
                QSpinBox:hover {
                    border: 1px solid #007BFF;
                }
                QSpinBox:focus {
                    border: 1px solid #5CAEFF;
                }

                /* Up/Down buttons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }

                QCheckBox::indicator {
                    width: 16px;
                    height: 16px;
                    border: 2px solid #007BFF;
                    background-color: #020D22;
                    border-radius: 4px;
                }

                QCheckBox::indicator:hover {
                    background-color: #0A1A3A;
                    border: 2px solid #369CFF;
                }

                QCheckBox::indicator:checked {
                    background-color: #007BFF;
                    border: 2px solid #007BFF;
                }

                QCheckBox::indicator:checked:hover {
                    background-color: #0515f7;
                    border: 2px solid #707070;
                }

                QCheckBox::indicator:disabled {
                    background-color: #369CFF;
                    border: 2px solid #369CFF;
                }

                QKeySequenceEdit QLineEdit:focus {
                    border: 1px solid #60A5FA;
                }

                QListWidget {
                    background: #111A2E;
                    border: 1px solid #2A3A5A;
                    border-radius: 6px;
                    color: #FFFFFF;
                }
                QListWidget::item:selected {
                    background: #007BFF;
                    color: #FFFFFF;
                    border-radius: 4px;
                }

                QTabWidget::pane {
                    border: 1px solid #2A3A5A;
                    background: #0F172A;
                    border-radius: 6px;
                }
                QTabBar::tab {
                    background: #1A2233;
                    color: #FFFFFF;
                    padding: 8px 16px;
                    border: 1px solid #2A3A5A;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #0F172A;
                    border-bottom: 2px solid #007BFF;
                }
            )";
        break;
    }
    
    return styleSheet;
}

QString SettingsManager::getCurrentThemeStyleSheet() const {
    return getThemeStyleSheet(getTheme(), getFontSize(), getFontFamily());
}

bool SettingsManager::getShowCommandLabel() const {
    return m_settings->value("terminal/showCommandLabel", true).toBool();
}

void SettingsManager::setShowCommandLabel(bool show) {
    m_settings->setValue("terminal/showCommandLabel", show);
}

bool SettingsManager::getAutoCloseTerminal() const {
    return m_settings->value("terminal/autoClose", false).toBool();
}

void SettingsManager::setAutoCloseTerminal(bool autoClose) {
    m_settings->setValue("terminal/autoClose", autoClose);
}

bool SettingsManager::getInstantRunFromMenu() const {
    return m_settings->value("terminal/instantRunFromMenu", false).toBool();
}

void SettingsManager::setInstantRunFromMenu(bool instant) {
    m_settings->setValue("terminal/instantRunFromMenu", instant);
}

bool SettingsManager::getPlayCompletionSound() const {
    return m_settings->value("terminal/playCompletionSound", false).toBool();
}

void SettingsManager::setPlayCompletionSound(bool play) {
    m_settings->setValue("terminal/playCompletionSound", play);
}

bool SettingsManager::getShowCommandPreview() const {
    return m_settings->value("terminal/showCommandPreview", true).toBool();
}

void SettingsManager::setShowCommandPreview(bool show) {
    m_settings->setValue("terminal/showCommandPreview", show);
}

SettingsManager::TerminalColorScheme SettingsManager::getTerminalColorScheme() const {
    return static_cast<TerminalColorScheme>(m_settings->value("terminal/colorScheme", NeonGreen).toInt());
}

void SettingsManager::setTerminalColorScheme(TerminalColorScheme scheme) {
    m_settings->setValue("terminal/colorScheme", static_cast<int>(scheme));
}

QString SettingsManager::getTerminalFontFamily() const {
    return m_settings->value("terminal/fontFamily", "Courier").toString();
}

void SettingsManager::setTerminalFontFamily(const QString& family) {
    m_settings->setValue("terminal/fontFamily", family);
}

int SettingsManager::getTerminalFontSize() const {
    return m_settings->value("terminal/fontSize", 10).toInt();
}

void SettingsManager::setTerminalFontSize(int size) {
    m_settings->setValue("terminal/fontSize", size);
}

QPair<QColor, QColor> SettingsManager::getTerminalColors(TerminalColorScheme scheme) {
    // Returns QPair<background, foreground>
    switch (scheme) {
        case NeonGreen:
            return qMakePair(QColor("#000000"), QColor("#00FF00")); // Black bg, neon green text
        case Classic:
            return qMakePair(QColor("#000000"), QColor("#FFFFFF")); // Black bg, white text
        case LightMode:
            return qMakePair(QColor("#FFFFFF"), QColor("#000000")); // White bg, black text
        case Matrix:
            return qMakePair(QColor("#001a1a"), QColor("#00FFFF")); // Dark blue bg, cyan text
        case Dracula:
            return qMakePair(QColor("#282a36"), QColor("#f8f8f2")); // Dark purple bg, light text
        case Monokai:
            return qMakePair(QColor("#272822"), QColor("#F8F8F2")); // Dark bg, cream text
        case Nord2:
            return qMakePair(QColor("#2E3440"), QColor("#D8DEE9")); // Cool dark blue, light blue-gray
        case SolarizedDark2:
            return qMakePair(QColor("#002b36"), QColor("#839496")); // Deep teal, muted cyan-gray
        case GruvboxDark:
            return qMakePair(QColor("#282828"), QColor("#ebdbb2")); // Warm dark brown, cream
        case OneDark:
            return qMakePair(QColor("#282c34"), QColor("#abb2bf")); // Dark gray, light gray
        default:
            return qMakePair(QColor("#000000"), QColor("#00FF00"));
    }
}
