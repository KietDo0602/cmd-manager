#include "settingsdialog.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setModal(true);
    resize(500, 400);
    
    m_settings = SettingsManager::instance();
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    setupGeneralTab();
    setupAppearanceTab();
    setupKeyboardTab();
    setupTerminalTab();
    setupHelpTab();
    
    mainLayout->addWidget(m_tabWidget);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("OK");
    m_cancelButton = new QPushButton("Cancel");
    m_applyButton = new QPushButton("Apply");
    m_restoreDefaultsButton = new QPushButton("Restore Defaults");
    
    buttonLayout->addWidget(m_restoreDefaultsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultsClicked);
    
    loadSettings();
}

void SettingsDialog::setupGeneralTab() {
    m_generalTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_generalTab);
    
    // Default directory
    QHBoxLayout* dirLayout = new QHBoxLayout();
    m_defaultDirEdit = new QLineEdit();
    m_browseDirButton = new QPushButton("Browse...");
    dirLayout->addWidget(m_defaultDirEdit);
    dirLayout->addWidget(m_browseDirButton);
    layout->addRow("Default Directory:", dirLayout);
    
    connect(m_browseDirButton, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Choose Default Directory", m_defaultDirEdit->text());
        if (!dir.isEmpty()) {
            m_defaultDirEdit->setText(dir);
        }
    });
    
    // Auto-save
    m_autoSaveCheck = new QCheckBox("Automatically save changes");
    layout->addRow("Auto Save:", m_autoSaveCheck);
    
    m_tabWidget->addTab(m_generalTab, "General");
}

void SettingsDialog::setupAppearanceTab() {
    m_appearanceTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_appearanceTab);
    
    // Theme
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem("Dark", SettingsManager::Dark);
    m_themeCombo->addItem("Light", SettingsManager::Light);
    m_themeCombo->addItem("High Contrast", SettingsManager::Contrast);
    m_themeCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QListView *view = qobject_cast<QListView *>(m_themeCombo->view());
    if (view) {
        view->setSpacing(0);
        view->setStyleSheet("QListView { margin: 0px; padding: 0px; }");
    }

    layout->addRow("Theme:", m_themeCombo);
    
    // Font size
    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(8, 40);
    m_fontSizeSpin->setSuffix(" pt");
    layout->addRow("Font Size:", m_fontSizeSpin);
    
    // Font family
    m_fontFamilyCombo = new QFontComboBox();
    layout->addRow("Font Family:", m_fontFamilyCombo);
    
    // Preview
    m_previewLabel = new QLabel("Preview: The quick brown fox jumps over the lazy dog");
    m_previewLabel->setFrameStyle(QFrame::Box);
    m_previewLabel->setMinimumHeight(50);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    layout->addRow("Preview:", m_previewLabel);
    
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onThemeChanged);
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onFontSizeChanged);
    connect(m_fontFamilyCombo, &QFontComboBox::currentFontChanged, this, &SettingsDialog::onFontFamilyChanged);
    
    m_tabWidget->addTab(m_appearanceTab, "Appearance");
}

void SettingsDialog::setupKeyboardTab() {
    m_keyboardTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_keyboardTab);
    
    // Shortcuts
    m_newCommandEdit = new QKeySequenceEdit();
    layout->addRow("New Command:", m_newCommandEdit);
    
    m_saveCommandEdit = new QKeySequenceEdit();
    layout->addRow("Save Command:", m_saveCommandEdit);
    
    m_openCommandsEdit = new QKeySequenceEdit();
    layout->addRow("Open Commands:", m_openCommandsEdit);
    
    m_startExecuteEdit = new QKeySequenceEdit();
    layout->addRow("Start + Execute:", m_startExecuteEdit);
    
    m_tabWidget->addTab(m_keyboardTab, "Keyboard Shortcuts");
}

void SettingsDialog::setupHelpTab() {
    m_helpTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_helpTab);

    // Application Info
    layout->addRow("Application Name:", new QLabel("CMD Manager"));
    layout->addRow("Version:", new QLabel("v1.0.0"));

    // About
    QLabel* aboutLabel = new QLabel("CMD Manager is a free tool for managing and executing custom commands.");
    aboutLabel->setWordWrap(true);
    layout->addRow("About:", aboutLabel);

    // Help Link
    QPushButton* helpButton = new QPushButton("Help Link");
    helpButton->setCursor(Qt::PointingHandCursor);
    connect(helpButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/KietDo0602/cmd-manager/issues"));
    });
    layout->addRow("Help:", helpButton);

    // Website
    QPushButton* websiteButton = new QPushButton("My Website");
    websiteButton->setCursor(Qt::PointingHandCursor);
    connect(websiteButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://kietdo.io/"));
    });
    layout->addRow("Website:", websiteButton);

    // Contact
    QPushButton* contactButton = new QPushButton("Contact Me!");
    contactButton->setCursor(Qt::PointingHandCursor);
    connect(contactButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("mailto:kietdo0602@gmail.com"));
    });
    layout->addRow("Contact:", contactButton);

    m_tabWidget->addTab(m_helpTab, "Help");
}

void SettingsDialog::loadSettings() {
    // General
    m_defaultDirEdit->setText(m_settings->getDefaultDirectory());
    m_autoSaveCheck->setChecked(m_settings->getAutoSave());
    
    // Appearance
    m_themeCombo->setCurrentIndex(static_cast<int>(m_settings->getTheme()));
    m_fontSizeSpin->setValue(m_settings->getFontSize());
    m_fontFamilyCombo->setCurrentFont(QFont(m_settings->getFontFamily()));
    
    // Keyboard
    m_newCommandEdit->setKeySequence(QKeySequence(m_settings->getNewCommandShortcut()));
    m_saveCommandEdit->setKeySequence(QKeySequence(m_settings->getSaveCommandShortcut()));
    m_openCommandsEdit->setKeySequence(QKeySequence(m_settings->getOpenCommandsShortcut()));
    m_startExecuteEdit->setKeySequence(QKeySequence(m_settings->getStartExecuteShortcut())); 

    // Terminal
    m_showCommandLabelCheck->setChecked(m_settings->getShowCommandLabel());
    m_instantRunCheck->setChecked(m_settings->getInstantRunFromMenu());

    // Update preview
    onThemeChanged();
    onFontSizeChanged();
    onFontFamilyChanged();
}

void SettingsDialog::onThemeChanged() {
    SettingsManager::Theme theme = static_cast<SettingsManager::Theme>(m_themeCombo->currentData().toInt());
    
    // Use the static method for live preview
    QString previewStyleSheet = SettingsManager::getThemeStyleSheet(
        theme,
        m_fontSizeSpin->value(),
        m_fontFamilyCombo->currentFont().family()
    );
    
    setStyleSheet(previewStyleSheet);
}

void SettingsDialog::onFontSizeChanged() {
    QFont font = m_previewLabel->font();
    font.setPointSize(m_fontSizeSpin->value());
    m_previewLabel->setFont(font);
    
    // Also update the preview theme
    onThemeChanged();
}

void SettingsDialog::onFontFamilyChanged() {
    QFont font = m_fontFamilyCombo->currentFont();
    font.setPointSize(m_fontSizeSpin->value());
    m_previewLabel->setFont(font);
    
    // Also update the preview theme
    onThemeChanged();
}

void SettingsDialog::applySettings() {
    // General
    m_settings->setDefaultDirectory(m_defaultDirEdit->text());
    m_settings->setAutoSave(m_autoSaveCheck->isChecked());
    
    // Appearance
    SettingsManager::Theme theme = static_cast<SettingsManager::Theme>(m_themeCombo->currentData().toInt());
    m_settings->setTheme(theme);
    m_settings->setFontSize(m_fontSizeSpin->value());
    m_settings->setFontFamily(m_fontFamilyCombo->currentFont().family());
    
    // Keyboard
    m_settings->setNewCommandShortcut(m_newCommandEdit->keySequence().toString());
    m_settings->setSaveCommandShortcut(m_saveCommandEdit->keySequence().toString());
    m_settings->setOpenCommandsShortcut(m_openCommandsEdit->keySequence().toString());
    m_settings->setStartExecuteShortcut(m_startExecuteEdit->keySequence().toString());

    // Terminal
    m_settings->setShowCommandLabel(m_showCommandLabelCheck->isChecked());
    m_settings->setInstantRunFromMenu(m_instantRunCheck->isChecked());
    
    // Apply theme to all windows immediately
    m_settings->applyThemeToAllWindows();
}

void SettingsDialog::onApplyClicked() {
    applySettings();
    QMessageBox::information(this, "Settings Applied", "Settings have been applied successfully.");
}

void SettingsDialog::onOkClicked() {
    applySettings();
    accept();
}

void SettingsDialog::onCancelClicked() {
    reject();
}

void SettingsDialog::onRestoreDefaultsClicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Restore Defaults",
        "Are you sure you want to restore all settings to their default values?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Restore defaults
        m_defaultDirEdit->setText(QDir::homePath());
        m_autoSaveCheck->setChecked(false);
        m_themeCombo->setCurrentIndex(0); // Dark theme

        QFont font;
        #ifdef Q_OS_WIN
            font.setFamily("Segoe UI");      // Windows
        #elif defined(Q_OS_MAC)
            font.setFamily("Helvetica Neue"); // macOS
        #else
            font.setFamily("Ubuntu");         // Linux (fallback: Cantarell, Noto Sans)
        #endif
        font.setWeight(QFont::Light);        // thin/elegant
        font.setPointSize(14);               // default modern size
        m_fontFamilyCombo->setCurrentFont(font);
        m_fontSizeSpin->setValue(14);

        m_newCommandEdit->setKeySequence(QKeySequence("Ctrl+N"));
        m_saveCommandEdit->setKeySequence(QKeySequence("Ctrl+S"));
        m_openCommandsEdit->setKeySequence(QKeySequence("Ctrl+O"));
        m_startExecuteEdit->setKeySequence(QKeySequence("F5")); 
        m_showCommandLabelCheck->setChecked(true);
        m_instantRunCheck->setChecked(false);

        onThemeChanged();
        onFontSizeChanged();
        onFontFamilyChanged();
    }
}

void SettingsDialog::setupTerminalTab() {
    m_terminalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_terminalTab);
    
    QGroupBox* displayGroup = new QGroupBox("Display Options");
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    
    m_showCommandLabelCheck = new QCheckBox("Show command information before execution");
    m_showCommandLabelCheck->setToolTip("When enabled, displays working directory, command, and separator before execution");
    displayLayout->addWidget(m_showCommandLabelCheck);
    
    displayLayout->addStretch();
    layout->addWidget(displayGroup);
    
    QGroupBox* executionGroup = new QGroupBox("Execution Options");
    QVBoxLayout* executionLayout = new QVBoxLayout(executionGroup);
    
    m_instantRunCheck = new QCheckBox("Instant run from 'All Commands' menu");
    m_instantRunCheck->setToolTip("When enabled, displays working directory, command, and separator before command gets executed inside the terminal. Only the output of the command and some necessary information are shown.");
    executionLayout->addWidget(m_instantRunCheck);
    
    executionLayout->addStretch();
    layout->addWidget(executionGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_terminalTab, "Terminal");
}
