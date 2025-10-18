#include "settingsdialog.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Settings"));
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
    m_okButton = new QPushButton(tr("OK"));
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_applyButton = new QPushButton(tr("Apply"));
    m_restoreDefaultsButton = new QPushButton(tr("Restore Defaults"));
    
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
    QVBoxLayout* mainLayout = new QVBoxLayout(m_generalTab);
    
    QFormLayout* layout = new QFormLayout();
    
    // Language Support
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem("English", SettingsManager::English);
    m_languageCombo->addItem("Français (French)", SettingsManager::French);
    m_languageCombo->addItem("Español (Spanish)", SettingsManager::Spanish);
    m_languageCombo->addItem("中文 (Chinese)", SettingsManager::Chinese);
    m_languageCombo->addItem("Русский (Russian)", SettingsManager::Russian);
    m_languageCombo->addItem("Tiếng Việt (Vietnamese)", SettingsManager::Vietnamese);
    layout->addRow(tr("Language:"), m_languageCombo);
    
    // Default directory
    QHBoxLayout* dirLayout = new QHBoxLayout();
    m_defaultDirEdit = new QLineEdit();
    m_browseDirButton = new QPushButton(tr("Browse..."));
    dirLayout->addWidget(m_defaultDirEdit);
    dirLayout->addWidget(m_browseDirButton);
    layout->addRow(tr("Default Directory:"), dirLayout);
    
    connect(m_browseDirButton, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Default Directory"), m_defaultDirEdit->text());
        if (!dir.isEmpty()) {
            m_defaultDirEdit->setText(dir);
        }
    });
    
    // Auto-save
    m_autoSaveCheck = new QCheckBox(tr("Automatically save changes"));
    layout->addRow(tr("Auto Save:"), m_autoSaveCheck);
    
    // Minimize to tray
    m_minimizeToTrayCheck = new QCheckBox(tr("Minimize to system tray when all windows are closed"));
    m_minimizeToTrayCheck->setToolTip(tr("When enabled, closing all windows will minimize the app to tray instead of exiting"));
    layout->addRow(tr("System Tray:"), m_minimizeToTrayCheck);
    
    mainLayout->addLayout(layout);
    
    // Export Commands section
    QGroupBox* exportGroup = new QGroupBox(tr("Data Management"));
    QVBoxLayout* exportLayout = new QVBoxLayout(exportGroup);
    
    QPushButton* exportButton = new QPushButton(tr("Export All Commands"));
    exportButton->setToolTip(tr("Export all saved commands to a JSON file"));
    connect(exportButton, &QPushButton::clicked, this, &SettingsDialog::onExportCommands);
    
    QPushButton* importButton = new QPushButton(tr("Import Commands"));
    importButton->setToolTip(tr("Import commands from a JSON file"));
    connect(importButton, &QPushButton::clicked, this, &SettingsDialog::onImportCommands);
    
    exportLayout->addWidget(exportButton);
    exportLayout->addWidget(importButton);
    exportLayout->addStretch();
    
    mainLayout->addWidget(exportGroup);
    mainLayout->addStretch();
    
    m_tabWidget->addTab(m_generalTab, tr("General"));
}

void SettingsDialog::setupAppearanceTab() {
    m_appearanceTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_appearanceTab);
    
    // Theme
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem(tr("Dark"), SettingsManager::Dark);
    m_themeCombo->addItem(tr("Light"), SettingsManager::Light);
    m_themeCombo->addItem(tr("High Contrast"), SettingsManager::Contrast);
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
    layout->addRow(tr("Font Size:"), m_fontSizeSpin);
    
    // Font family
    m_fontFamilyCombo = new QFontComboBox();
    layout->addRow(tr("Font Family:"), m_fontFamilyCombo);
    
    // Preview
    m_previewLabel = new QLabel(tr("Preview: The quick brown fox jumps over the lazy dog"));
    m_previewLabel->setFrameStyle(QFrame::Box);
    m_previewLabel->setFixedHeight(70);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    layout->addRow(tr("Preview:"), m_previewLabel);
    
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onThemeChanged);
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onFontSizeChanged);
    connect(m_fontFamilyCombo, &QFontComboBox::currentFontChanged, this, &SettingsDialog::onFontFamilyChanged);
    
    m_tabWidget->addTab(m_appearanceTab, tr("Appearance"));
}

void SettingsDialog::setupKeyboardTab() {
    m_keyboardTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_keyboardTab);
    
    // Shortcuts
    m_newCommandEdit = new QKeySequenceEdit();
    layout->addRow(tr("New Command:"), m_newCommandEdit);
    
    m_saveCommandEdit = new QKeySequenceEdit();
    layout->addRow(tr("Save Command:"), m_saveCommandEdit);
    
    m_openCommandsEdit = new QKeySequenceEdit();
    layout->addRow(tr("Open Commands:"), m_openCommandsEdit);
    
    m_startExecuteEdit = new QKeySequenceEdit();
    layout->addRow(tr("Validate + Run:"), m_startExecuteEdit);
    
    m_tabWidget->addTab(m_keyboardTab, tr("Keyboard Shortcuts"));
}

void SettingsDialog::setupHelpTab() {
    m_helpTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_helpTab);

    // Application Info
    layout->addRow(tr("Application Name:"), new QLabel(tr("CMD Manager")));
    layout->addRow(tr("Version:"), new QLabel("v1.0.0"));

    // About
    QLabel* aboutLabel = new QLabel(tr("CMD Manager is a free tool for managing and executing custom commands."));
    aboutLabel->setWordWrap(true);
    layout->addRow(tr("About:"), aboutLabel);

    // Help Link
    QPushButton* helpButton = new QPushButton(tr("Help Link"));
    helpButton->setCursor(Qt::PointingHandCursor);
    connect(helpButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/KietDo0602/cmd-manager/issues"));
    });
    layout->addRow(tr("Help:"), helpButton);

    // Website
    QPushButton* websiteButton = new QPushButton(tr("My Website"));
    websiteButton->setCursor(Qt::PointingHandCursor);
    connect(websiteButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://kietdo.io/"));
    });
    layout->addRow(tr("Website:"), websiteButton);

    // Contact
    QPushButton* contactButton = new QPushButton(tr("Contact Me!"));
    contactButton->setCursor(Qt::PointingHandCursor);
    connect(contactButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("mailto:kietdo0602@gmail.com"));
    });
    layout->addRow(tr("Contact:"), contactButton);

    m_tabWidget->addTab(m_helpTab, tr("Help"));
}

void SettingsDialog::loadSettings() {
    // General
    int langIndex = m_languageCombo->findData(m_settings->getLanguage());
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
    m_defaultDirEdit->setText(m_settings->getDefaultDirectory());
    m_autoSaveCheck->setChecked(m_settings->getAutoSave());
    m_minimizeToTrayCheck->setChecked(m_settings->getMinimizeToTray());
    
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
    m_showCommandPreviewCheck->setChecked(m_settings->getShowCommandPreview());
    m_instantRunCheck->setChecked(m_settings->getInstantRunFromMenu());
    m_autoCloseTerminalCheck->setChecked(m_settings->getAutoCloseTerminal());
    m_playCompletionSoundCheck->setChecked(m_settings->getPlayCompletionSound());

    int terminalSchemeIndex = m_terminalColorSchemeCombo->findData(m_settings->getTerminalColorScheme());
    if (terminalSchemeIndex >= 0) {
        m_terminalColorSchemeCombo->setCurrentIndex(terminalSchemeIndex);
    }
    m_terminalFontCombo->setCurrentFont(QFont(m_settings->getTerminalFontFamily()));
    m_terminalFontSizeSpin->setValue(m_settings->getTerminalFontSize());

    // Update preview
    onThemeChanged();
    onFontSizeChanged();
    onFontFamilyChanged();
    m_settings->applyThemeToAllWindows();
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
    
    // Update the dialog's own appearance with current preview settings
    SettingsManager::Theme theme = static_cast<SettingsManager::Theme>(m_themeCombo->currentData().toInt());
    QString previewStyleSheet = SettingsManager::getThemeStyleSheet(
        theme,
        m_fontSizeSpin->value(),
        m_fontFamilyCombo->currentFont().family()
    );
    setStyleSheet(previewStyleSheet);
}

void SettingsDialog::onFontFamilyChanged() {
    QFont font = m_fontFamilyCombo->currentFont();
    font.setPointSize(m_fontSizeSpin->value());
    m_previewLabel->setFont(font);
    
    // Update the dialog's own appearance with current preview settings
    SettingsManager::Theme theme = static_cast<SettingsManager::Theme>(m_themeCombo->currentData().toInt());
    QString previewStyleSheet = SettingsManager::getThemeStyleSheet(
        theme,
        m_fontSizeSpin->value(),
        m_fontFamilyCombo->currentFont().family()
    );
    setStyleSheet(previewStyleSheet);
}

void SettingsDialog::applySettings() {
    // Check if language changed
    SettingsManager::Language oldLang = m_settings->getLanguage();
    SettingsManager::Language newLang = static_cast<SettingsManager::Language>(m_languageCombo->currentData().toInt());
    bool languageChanged = (oldLang != newLang);
    m_settings->setLanguage(newLang);

    // General
    m_settings->setDefaultDirectory(m_defaultDirEdit->text());
    m_settings->setAutoSave(m_autoSaveCheck->isChecked());
    m_settings->setMinimizeToTray(m_minimizeToTrayCheck->isChecked());
    
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
    m_settings->setShowCommandPreview(m_showCommandPreviewCheck->isChecked());
    m_settings->setInstantRunFromMenu(m_instantRunCheck->isChecked());
    m_settings->setAutoCloseTerminal(m_autoCloseTerminalCheck->isChecked());
    m_settings->setPlayCompletionSound(m_playCompletionSoundCheck->isChecked());

    SettingsManager::TerminalColorScheme terminalScheme = 
    static_cast<SettingsManager::TerminalColorScheme>(m_terminalColorSchemeCombo->currentData().toInt());
    m_settings->setTerminalColorScheme(terminalScheme);
    m_settings->setTerminalFontFamily(m_terminalFontCombo->currentFont().family());
    m_settings->setTerminalFontSize(m_terminalFontSizeSpin->value());
    
    // Apply theme to all windows immediately
    m_settings->applyThemeToAllWindows();

    // Notify about language change
    if (languageChanged) {
        QMessageBox::information(this, tr("Language Changed"), 
            tr("Language change will take effect after restarting the application."));
    }
}

void SettingsDialog::onApplyClicked() {
    applySettings();
    QMessageBox::information(this, tr("Settings Applied"), tr("Settings have been applied successfully."));
}

void SettingsDialog::onOkClicked() {
    applySettings();
    accept();
}

void SettingsDialog::onCancelClicked() {
    reject();
}

void SettingsDialog::onRestoreDefaultsClicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Restore Defaults"),
        tr("Are you sure you want to restore all settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Restore defaults
        m_languageCombo->setCurrentIndex(0); // Change language back to English
        m_defaultDirEdit->setText(QDir::homePath());
        m_autoSaveCheck->setChecked(false);
        m_minimizeToTrayCheck->setChecked(false);

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
        m_showCommandPreviewCheck->setChecked(true);
        m_instantRunCheck->setChecked(true);
        m_autoCloseTerminalCheck->setChecked(false);

        m_terminalColorSchemeCombo->setCurrentIndex(0); // Neon Green
        m_terminalFontCombo->setCurrentFont(QFont("Courier"));
        m_terminalFontSizeSpin->setValue(10);

        onThemeChanged();
        onFontSizeChanged();
        onFontFamilyChanged();
        onTerminalPreviewUpdate();
    }
}

void SettingsDialog::setupTerminalTab() {
    m_terminalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_terminalTab);
    
    QGroupBox* displayGroup = new QGroupBox(tr("Display Options"));
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    
    m_showCommandLabelCheck = new QCheckBox(tr("Show command information before execution"));
    m_showCommandLabelCheck->setToolTip(tr("When enabled, displays working directory, command, and separator inside terminal right before execution"));
    displayLayout->addWidget(m_showCommandLabelCheck);
    
    displayLayout->addStretch();
    layout->addWidget(displayGroup);
    
    QGroupBox* executionGroup = new QGroupBox(tr("Execution Options"));
    QVBoxLayout* executionLayout = new QVBoxLayout(executionGroup);

    m_showCommandPreviewCheck = new QCheckBox(tr("Show command preview before execution"));
    m_showCommandPreviewCheck->setToolTip(tr("When enabled, shows a preview dialog with the full command and dangerous command warnings before execution"));
    executionLayout->addWidget(m_showCommandPreviewCheck);

    m_autoCloseTerminalCheck = new QCheckBox(tr("Automatically close terminal when command finishes"));
    m_autoCloseTerminalCheck->setToolTip(tr("When enabled, terminal window closes automatically after command execution completes"));
    executionLayout->addWidget(m_autoCloseTerminalCheck);

    m_playCompletionSoundCheck = new QCheckBox(tr("Play sound when command completes"));
    m_playCompletionSoundCheck->setToolTip(tr("When enabled, plays a notification sound when command execution finishes"));
    executionLayout->addWidget(m_playCompletionSoundCheck);
    
    m_instantRunCheck = new QCheckBox(tr("Instant run from Commands menu"));
    m_instantRunCheck->setToolTip(tr("When enabled, displays working directory, command, and separator before command gets executed inside the terminal. Only the output of the command and some necessary information are shown."));
    executionLayout->addWidget(m_instantRunCheck);
    
    executionLayout->addStretch();
    layout->addWidget(executionGroup);

    QGroupBox* appearanceGroup = new QGroupBox(tr("Terminal Appearance"));
    QFormLayout* appearanceLayout = new QFormLayout(appearanceGroup);
    
    // Color scheme dropdown
    m_terminalColorSchemeCombo = new QComboBox();
    m_terminalColorSchemeCombo->addItem(tr("Neon Green (Default)"), SettingsManager::NeonGreen);
    m_terminalColorSchemeCombo->addItem(tr("Classic (White on Black)"), SettingsManager::Classic);
    m_terminalColorSchemeCombo->addItem(tr("Light Mode (Black on White)"), SettingsManager::LightMode);
    m_terminalColorSchemeCombo->addItem(tr("Matrix (Cyan on Dark Blue)"), SettingsManager::Matrix);
    m_terminalColorSchemeCombo->addItem(tr("Dracula"), SettingsManager::Dracula);
    m_terminalColorSchemeCombo->addItem(tr("Monokai"), SettingsManager::Monokai);
    m_terminalColorSchemeCombo->addItem(tr("Nord"), SettingsManager::Nord);
    m_terminalColorSchemeCombo->addItem(tr("Solarized Dark"), SettingsManager::SolarizedDark);
    m_terminalColorSchemeCombo->addItem(tr("Gruvbox Dark"), SettingsManager::GruvboxDark);
    m_terminalColorSchemeCombo->addItem(tr("One Dark"), SettingsManager::OneDark);
    appearanceLayout->addRow(tr("Color Scheme:"), m_terminalColorSchemeCombo);

    // Font family
    m_terminalFontCombo = new QFontComboBox();
    m_terminalFontCombo->setFontFilters(QFontComboBox::MonospacedFonts);
    appearanceLayout->addRow(tr("Font Family:"), m_terminalFontCombo);
    
    // Font size
    m_terminalFontSizeSpin = new QSpinBox();
    m_terminalFontSizeSpin->setRange(8, 24);
    m_terminalFontSizeSpin->setSuffix(" pt");
    appearanceLayout->addRow(tr("Font Size:"), m_terminalFontSizeSpin);
    
    layout->addWidget(appearanceGroup);
    
    // Preview
    QGroupBox* previewGroup = new QGroupBox(tr("Preview"));
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    
    m_terminalPreview = new QTextEdit();
    m_terminalPreview->setReadOnly(true);
    m_terminalPreview->setMaximumHeight(100);
    m_terminalPreview->setText("$ ls -la\ntotal 48\ndrwxr-xr-x  2 user user 4096 Oct  1 12:34 .\ndrwxr-xr-x 18 user user 4096 Oct  1 12:30 ..\n-rw-r--r--  1 user user  220 Oct  1 12:30 .bash_logout");
    previewLayout->addWidget(m_terminalPreview);
    
    layout->addWidget(previewGroup);
    
    layout->addStretch();

    // Connect signals for live preview
    connect(m_terminalColorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onTerminalPreviewUpdate);
    connect(m_terminalFontCombo, &QFontComboBox::currentFontChanged, 
            this, &SettingsDialog::onTerminalPreviewUpdate);
    connect(m_terminalFontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &SettingsDialog::onTerminalPreviewUpdate);
    
    m_tabWidget->addTab(m_terminalTab, tr("Terminal"));
}

void SettingsDialog::onTerminalPreviewUpdate() {
    SettingsManager::TerminalColorScheme scheme = 
        static_cast<SettingsManager::TerminalColorScheme>(m_terminalColorSchemeCombo->currentData().toInt());
    
    QPair<QColor, QColor> colors = SettingsManager::getTerminalColors(scheme);
    QColor bgColor = colors.first;
    QColor fgColor = colors.second;
    
    QString previewStyle = QString(
        "QTextEdit { "
        "background-color: %1; "
        "color: %2; "
        "font-family: '%3'; "
        "font-size: %4pt; "
        "border: 1px solid #555; "
        "}"
    ).arg(bgColor.name())
     .arg(fgColor.name())
     .arg(m_terminalFontCombo->currentFont().family())
     .arg(m_terminalFontSizeSpin->value());
    
    m_terminalPreview->setStyleSheet(previewStyle);
}

void SettingsDialog::onExportCommands() {
    QString fileName = QFileDialog::getSaveFileName(this, 
        tr("Export Commands"), 
        QDir::homePath() + "/cmd_manager_commands.json",
        tr("JSON Files (*.json)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Get the commands file path
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString commandsFile = configPath + "/CMDManager/commands.json";
    
    // Copy the commands file to the selected location
    if (QFile::exists(commandsFile)) {
        if (QFile::exists(fileName)) {
            QFile::remove(fileName);
        }
        
        if (QFile::copy(commandsFile, fileName)) {
            QMessageBox::information(this, tr("Export Successful"), 
                QString(tr("Commands exported successfully to:\n%1")).arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Export Failed"), 
                tr("Failed to export commands. Please check file permissions."));
        }
    } else {
        QMessageBox::warning(this, tr("No Commands"), 
            tr("No commands found to export."));
    }
}

void SettingsDialog::onImportCommands() {
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Import Commands"), 
        QDir::homePath(),
        tr("JSON Files (*.json)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Read the import file
    QFile importFile(fileName);
    if (!importFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Import Failed"), 
            tr("Failed to open file for reading."));
        return;
    }
    
    QByteArray importData = importFile.readAll();
    importFile.close();
    
    // Validate JSON
    QJsonDocument importDoc = QJsonDocument::fromJson(importData);
    if (!importDoc.isObject()) {
        QMessageBox::warning(this, tr("Invalid File"), 
            tr("The selected file is not a valid commands JSON file."));
        return;
    }
    
    // Ask user how to handle conflicts
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        tr("Import Commands"),
        tr("How do you want to handle existing commands?\n"
        "Yes - Merge (keep existing, add new)\n"
        "No - Replace (overwrite all existing commands)"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    
    if (reply == QMessageBox::Cancel) {
        return;
    }
    
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString commandsFile = configPath + "/CMDManager/commands.json";
    
    QJsonObject finalCommands;
    
    if (reply == QMessageBox::Yes) {
        // Merge: Load existing commands first
        QFile existingFile(commandsFile);
        if (existingFile.open(QIODevice::ReadOnly)) {
            QJsonDocument existingDoc = QJsonDocument::fromJson(existingFile.readAll());
            existingFile.close();
            if (existingDoc.isObject()) {
                finalCommands = existingDoc.object();
            }
        }
        
        // Add/overwrite with imported commands
        QJsonObject importedCommands = importDoc.object();
        for (auto it = importedCommands.begin(); it != importedCommands.end(); ++it) {
            finalCommands[it.key()] = it.value();
        }
    } else {
        // Replace: Use only imported commands
        finalCommands = importDoc.object();
    }
    
    // Write to commands file
    QDir().mkpath(QFileInfo(commandsFile).absolutePath());
    QFile file(commandsFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(finalCommands);
        file.write(doc.toJson());
        file.close();
        
        QMessageBox::information(this, tr("Import Successful"), 
            QString(tr("Successfully imported commands from:\n%1")).arg(fileName));
    } else {
        QMessageBox::warning(this, tr("Import Failed"), 
            tr("Failed to write commands. Please check file permissions."));
    }
}
