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
                    padding: 0px;
                    outline: 0;
                    selection-background-color: #A0C4FF;
                    selection-color: #FFFFFF;
                }

                QComboBox QAbstractItemView::item {
                    padding: 4px 8px;
                }

                QComboBox QAbstractItemView::viewport {
                    background-color: #FFFFFF;
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
                /* Base Widget */
                QWidget {
                    background-color: #0B0B0B;   /* ultra dark background */
                    color: #F5F5F5;              /* bright text */
                }

                /* Buttons */
                QPushButton {
                    background-color: #1A1A1A;   /* deep black-gray */
                    border: 1px solid #00BFFF;   /* bright cyan border */
                    border-radius: 8px;
                    padding: 8px 16px;
                    color: #FFFFFF;
                }
                QPushButton:hover {
                    background-color: #003366;   /* dark navy hover */
                }
                QPushButton:pressed {
                    background-color: #0055AA;   /* bold blue pressed */
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background: #121212;
                    border: 1px solid #00BFFF;
                    border-radius: 8px;
                    padding: 8px;
                    color: #F5F5F5;
                    selection-background-color: #00BFFF;
                    selection-color: #000000;
                }

                /* Labels */
                QLabel {
                    font-weight: 500;
                    color: #F5F5F5;
                }
                QLabel#preview {
                    background-color: #1A1A1A;
                    border: 1px solid #333333;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #121212;
                    border: 1px solid #333333;
                    border-radius: 10px;
                    margin-top: 10px;
                    padding: 8px;
                    font-weight: 500;
                    color: #F5F5F5;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #121212;
                    border: 1px solid #00BFFF;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #F5F5F5;
                    min-height: 28px;
                }
                QComboBox:hover {
                    border: 1px solid #33CCFF;
                }
                QComboBox:focus {
                    border: 1px solid #66D9FF;
                }

                /* Down arrow */
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #333333;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                    background-color: #0B0B0B;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QComboBox::down-arrow:on {
                    image: url(:/images/arrow_down_light.svg);
                }

                /* Drop-down popup */
                QComboBox QAbstractItemView {
                    background-color: #121212;
                    border: 1px solid #00BFFF;
                    border-radius: 6px;
                    padding: 0px;
                    outline: 0;
                    selection-background-color: #00BFFF;
                    selection-color: #000000;
                }

                QComboBox QAbstractItemView::item {
                    padding: 4px 8px;
                }

                QComboBox QAbstractItemView::viewport {
                    background-color: #121212;
                }

                /* Scrollbar inside dropdown */
                QComboBox QAbstractScrollArea QScrollBar:vertical {
                    width: 8px;
                    background: #0B0B0B;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical {
                    background: #00BFFF;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {
                    background: #33CCFF;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #121212;
                    border: 1px solid #00BFFF;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #F5F5F5;
                    min-height: 28px;
                }
                QSpinBox:hover {
                    border: 1px solid #33CCFF;
                }
                QSpinBox:focus {
                    border: 1px solid #66D9FF;
                }

                /* Up/Down buttons */
                QSpinBox::up-button, QSpinBox::down-button {
                    subcontrol-origin: border;
                    width: 20px;
                    border-left: 1px solid #333333;
                    background-color: #0B0B0B;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                }
                QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                    background-color: #1A1A1A;
                }
                QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {
                    background-color: #0055AA;
                }

                /* SVG icons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::up-arrow:disabled {
                    image: url(:/images/arrow_up_light.svg);
                    opacity: 0.4;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow:disabled {
                    image: url(:/images/arrow_down_light.svg);
                    opacity: 0.4;
                }

                /* List Widgets */
                QListWidget {
                    background: #121212;
                    border: 1px solid #333333;
                    border-radius: 8px;
                    color: #F5F5F5;
                }
                QListWidget::item:selected {
                    background: #00BFFF;
                    color: #000000;
                    border-radius: 6px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #333333;
                    background: #121212;
                    border-radius: 8px;
                }
                QTabBar::tab {
                    background: #0B0B0B;
                    color: #F5F5F5;
                    padding: 8px 14px;
                    border: 1px solid #333333;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #121212;
                    color: #FFFFFF;
                    border-bottom: 2px solid #00BFFF;
                }

            )";
            break;
        default: // Dark
            styleSheet = R"(
                /* Base Widget */
                QWidget {
                    background-color: #2F3136;
                    color: #DCDDDE;
                }

                /* Buttons */
                QPushButton {
                    background-color: #5865F2;
                    border: none;
                    border-radius: 8px;
                    padding: 8px 16px;
                    color: #FFFFFF;
                }
                QPushButton:hover {
                    background-color: #4752C4;
                }
                QPushButton:pressed {
                    background-color: #3C45A5;
                }

                /* Text Fields */
                QLineEdit, QTextEdit {
                    background: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 8px;
                    padding: 8px;
                    color: #DCDDDE;
                    selection-background-color: #5865F2;
                    selection-color: #FFFFFF;
                }

                /* Labels */
                QLabel {
                    font-weight: 500;
                    color: #DCDDDE;
                }
                QLabel#preview {
                    background-color: #2C2F33;
                    border: 1px solid #4F545C;
                    border-radius: 6px;
                    padding: 6px;
                }

                /* Group Boxes */
                QGroupBox {
                    background-color: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 10px;
                    margin-top: 10px;
                    padding: 8px;
                    font-weight: 500;
                    color: #DCDDDE;
                }

                /* ComboBox */
                QComboBox {
                    background-color: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #DCDDDE;
                    min-height: 28px;
                }
                QComboBox:hover {
                    border: 1px solid #5865F2;
                }
                QComboBox:focus {
                    border: 1px solid #7289DA;
                }

                /* Down arrow */
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 24px;
                    border-left: 1px solid #4F545C;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                    background-color: #2F3136;
                }
                QComboBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QComboBox::down-arrow:on {
                    image: url(:/images/arrow_down_light.svg);
                }

                /* Drop-down popup */
                QComboBox QAbstractItemView {
                    background-color: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 6px;
                    padding: 0px;
                    outline: 0;
                    selection-background-color: #5865F2;
                    selection-color: #FFFFFF;
                }

                QComboBox QAbstractItemView::item {
                    padding: 4px 8px;
                }

                QComboBox QAbstractItemView::viewport {
                    background-color: #202225;
                }

                /* Scrollbar inside dropdown */
                QComboBox QAbstractScrollArea QScrollBar:vertical {
                    width: 8px;
                    background: #2F3136;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical {
                    background: #5865F2;
                    border-radius: 4px;
                }
                QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {
                    background: #7289DA;
                }

                /* Spin Boxes */
                QSpinBox {
                    background-color: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 8px;
                    padding: 6px 10px;
                    color: #DCDDDE;
                    min-height: 28px;
                }
                QSpinBox:hover {
                    border: 1px solid #5865F2;
                }
                QSpinBox:focus {
                    border: 1px solid #7289DA;
                }

                /* Up/Down buttons */
                QSpinBox::up-button, QSpinBox::down-button {
                    subcontrol-origin: border;
                    width: 20px;
                    border-left: 1px solid #4F545C;
                    background-color: #2F3136;
                    border-top-right-radius: 8px;
                    border-bottom-right-radius: 8px;
                }
                QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                    background-color: #3A3C43;
                }
                QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {
                    background-color: #5865F2;
                }

                /* SVG icons */
                QSpinBox::up-arrow {
                    image: url(:/images/arrow_up_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::up-arrow:disabled {
                    image: url(:/images/arrow_up_light.svg);
                    opacity: 0.4;
                }
                QSpinBox::down-arrow {
                    image: url(:/images/arrow_down_light.svg);
                    width: 12px;
                    height: 12px;
                }
                QSpinBox::down-arrow:disabled {
                    image: url(:/images/arrow_down_light.svg);
                    opacity: 0.4;
                }

                /* List Widgets */
                QListWidget {
                    background: #202225;
                    border: 1px solid #4F545C;
                    border-radius: 8px;
                    color: #DCDDDE;
                }
                QListWidget::item:selected {
                    background: #5865F2;
                    color: #FFFFFF;
                    border-radius: 6px;
                }

                /* Tabs */
                QTabWidget::pane {
                    border: 1px solid #4F545C;
                    background: #202225;
                    border-radius: 8px;
                }
                QTabBar::tab {
                    background: #2F3136;
                    color: #DCDDDE;
                    padding: 8px 14px;
                    border: 1px solid #4F545C;
                    border-top-left-radius: 6px;
                    border-top-right-radius: 6px;
                }
                QTabBar::tab:selected {
                    background: #202225;
                    color: #FFFFFF;
                    border-bottom: 2px solid #5865F2;
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
        case Nord:
            return qMakePair(QColor("#2E3440"), QColor("#D8DEE9")); // Cool dark blue, light blue-gray
        case SolarizedDark:
            return qMakePair(QColor("#002b36"), QColor("#839496")); // Deep teal, muted cyan-gray
        case GruvboxDark:
            return qMakePair(QColor("#282828"), QColor("#ebdbb2")); // Warm dark brown, cream
        case OneDark:
            return qMakePair(QColor("#282c34"), QColor("#abb2bf")); // Dark gray, light gray
        default:
            return qMakePair(QColor("#000000"), QColor("#00FF00"));
    }
}
