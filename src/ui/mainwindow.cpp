#include "mainwindow.h"

// Initialize static member
QSet<QString> MainWindow::openedCommands;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), process(new QProcess(this)), hasAnyChanges(false) {

    setWindowTitle(tr("CMD Manager"));

    // Setup system tray
    setupSystemTray();

    QWidget *central = new QWidget(this);
    mainLayout = new QVBoxLayout(central);

    // Top buttons row (New, Commands Menu, Save)
    QHBoxLayout *topButtonsLayout = new QHBoxLayout();
    
    newButton = new QPushButton(tr("New"));
    allCommandsButton = new QPushButton(tr("Commands Menu"));
    saveButton = new QPushButton(tr("Save"));
    settingsButton = new QPushButton(tr("Settings"));
    
    // Add stretch before and after the buttons to center them
    topButtonsLayout->addStretch();
    topButtonsLayout->addWidget(newButton);
    topButtonsLayout->addWidget(saveButton);
    topButtonsLayout->addWidget(allCommandsButton);
    topButtonsLayout->addWidget(settingsButton);
    topButtonsLayout->addStretch(); 

    connect(newButton, &QPushButton::clicked, this, &MainWindow::onNewClicked);
    connect(allCommandsButton, &QPushButton::clicked, this, &MainWindow::onAllCommandsClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(SettingsManager::instance(), &SettingsManager::themeChanged, 
        this, &MainWindow::applyThemeAndFont);
    connect(SettingsManager::instance(), &SettingsManager::fontChanged, 
        this, &MainWindow::applyThemeAndFont);

    mainLayout->addLayout(topButtonsLayout);

    // Directory chooser
    chooseDirButton = new QPushButton(tr("Current Directory: (Not Set)"));
    connect(chooseDirButton, &QPushButton::clicked, this, &MainWindow::onChooseDirClicked);
    mainLayout->addWidget(chooseDirButton, 0, Qt::AlignCenter);

    // Command editor
    commandEdit = new QTextEdit();
    commandEdit->setPlaceholderText(tr("Enter command here..."));
    connect(commandEdit, &QTextEdit::textChanged, this, &MainWindow::onCommandTextChanged);
    mainLayout->addWidget(commandEdit);

    // Start button
    startButton = new QPushButton(tr("Validate Command"));
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    mainLayout->addWidget(startButton);

    // Dynamic button container
    buttonContainer = new QWidget();
    dynamicButtonLayout = new QVBoxLayout(buttonContainer);
    mainLayout->addWidget(buttonContainer);

    // Execute and Clear button
    executeButton = new QPushButton(tr("Run Command"));
    executeButton->hide();
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);

    clearButton = new QPushButton(tr("Clear Command"));
    clearButton->hide();

    // Layout for Execute and Clear side by side
    QHBoxLayout *execLayout = new QHBoxLayout;
    execLayout->addWidget(executeButton);
    execLayout->addWidget(clearButton);
    execLayout->addStretch();

    mainLayout->addLayout(execLayout);

    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    setCentralWidget(central);

    resize(1280, 720);

    // Setup keyboard shortcuts
    setupKeyboardShortcuts();

    // Apply initial theme and font
    applyThemeAndFont();
}

QIcon createColoredIcon(const QString& svgPath, const QColor& color) {
    // Read the SVG file
    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon();
    }
    
    QString svgContent = file.readAll();
    file.close();
    
    // Replace currentColor with actual color
    svgContent.replace("currentColor", color.name());
    
    // Create pixmap from modified SVG
    QSvgRenderer renderer(svgContent.toUtf8());
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    
    return QIcon(pixmap);
}

MainWindow::~MainWindow() {
    // Remove this command from opened commands when window closes
    if (!currentCommandName.isEmpty()) {
        openedCommands.remove(currentCommandName);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Check if there are unsaved changes (either in a saved command or new text)
    bool hasUnsavedWork = false;
    QString promptMessage;
    
    if (!currentCommandName.isEmpty() && hasUnsavedChanges) {
        // Saved command with changes
        hasUnsavedWork = true;
        promptMessage = QString(tr("You have unsaved changes in \"%1\". Do you want to save before closing?")).arg(currentCommandName);
    } else if (currentCommandName.isEmpty() && hasAnyChanges && !commandEdit->toPlainText().trimmed().isEmpty()) {
        // New unsaved command with text
        hasUnsavedWork = true;
        promptMessage = tr("You have unsaved work. Do you want to save before closing?");
    }
    
    // Check if minimize to tray is enabled
    if (SettingsManager::instance()->getMinimizeToTray()) {
        int mainWindowCount = 0;
        int totalAppWindows = 0;
        
        QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
        for (QWidget* widget : topLevelWidgets) {
            if (widget->isVisible() && !qobject_cast<QMenu*>(widget)) {
                totalAppWindows++;
                
                MainWindow* mainWin = qobject_cast<MainWindow*>(widget);
                if (mainWin) {
                    mainWindowCount++;
                }
            }
        }
        
        if (mainWindowCount == 1 && totalAppWindows == 1) {
            // Handle unsaved work before minimizing to tray
            if (hasUnsavedWork) {
                QMessageBox msgBox(this);
                msgBox.setWindowTitle(tr("Unsaved Changes"));
                msgBox.setText(promptMessage);
                msgBox.setIcon(QMessageBox::Warning);
                
                QPushButton *saveButton = msgBox.addButton(tr("Save and Minimize"), QMessageBox::AcceptRole);
                QPushButton *dontSaveButton = msgBox.addButton(tr("Don't Save"), QMessageBox::DestructiveRole);
                QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
                
                msgBox.setDefaultButton(saveButton);
                msgBox.exec();
                
                if (msgBox.clickedButton() == saveButton) {
                    onSaveClicked();
                    // Check if save was successful (user didn't cancel save dialog)
                    if (currentCommandName.isEmpty() && !commandEdit->toPlainText().trimmed().isEmpty()) {
                        // User cancelled the save dialog
                        event->ignore();
                        return;
                    }
                } else if (msgBox.clickedButton() == cancelButton) {
                    event->ignore();
                    return;
                }
                // If "Don't Save", continue with minimize
            }
            
            if (!currentCommandName.isEmpty()) {
                openedCommands.remove(currentCommandName);
            }
            
            hide();
            event->ignore();
            return;
        }
    }
    
    // Normal close behavior with unsaved changes check
    if (hasUnsavedWork) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Unsaved Changes"));
        msgBox.setText(promptMessage);
        msgBox.setIcon(QMessageBox::Warning);
        
        QPushButton *saveButton = msgBox.addButton(tr("Save and Close"), QMessageBox::AcceptRole);
        QPushButton *dontSaveButton = msgBox.addButton(tr("Don't Save"), QMessageBox::DestructiveRole);
        QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
        
        msgBox.setDefaultButton(saveButton);
        msgBox.exec();
        
        if (msgBox.clickedButton() == saveButton) {
            onSaveClicked();
            // Check if save was successful
            if (currentCommandName.isEmpty() && !commandEdit->toPlainText().trimmed().isEmpty()) {
                // User cancelled the save dialog
                event->ignore();
                return;
            }
            if (!currentCommandName.isEmpty()) {
                openedCommands.remove(currentCommandName);
            }
            event->accept();
        } else if (msgBox.clickedButton() == dontSaveButton) {
            if (!currentCommandName.isEmpty()) {
                openedCommands.remove(currentCommandName);
            }
            event->accept();
        } else {
            event->ignore();
            return;
        }
    } else {
        if (!currentCommandName.isEmpty()) {
            openedCommands.remove(currentCommandName);
        }
        event->accept();
    }
    
    QMainWindow::closeEvent(event);
}

void MainWindow::setupSystemTray() {
    // Create tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/app_icon.png"));
    trayIcon->setToolTip(tr("CMD Manager"));
    
    // Create tray menu
    trayMenu = new QMenu(this);
    
    QAction *showAction = new QAction("Show", this);
    connect(showAction, &QAction::triggered, [this]() {
        show();
        raise();
        activateWindow();
    });
    
    QAction *newWindowAction = new QAction(tr("New Window"), this);
    connect(newWindowAction, &QAction::triggered, []() {
        MainWindow *newWindow = new MainWindow();
        newWindow->setAttribute(Qt::WA_DeleteOnClose);
        newWindow->show();
    });
    
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    trayMenu->addAction(showAction);
    trayMenu->addAction(newWindowAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayMenu);
    
    // Handle both single and double-click to restore window
    connect(trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            // If this window is hidden, show it
            if (isHidden()) {
                show();
                raise();
                activateWindow();
                trayIcon->hide();
            } else {
                // If visible but minimized, restore it
                if (isMinimized()) {
                    showNormal();
                }
                raise();
                activateWindow();
            }
        }
    });
}

int MainWindow::showUnsavedChangesDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Unsaved Changes"));
    msgBox.setText(QString(tr("You have unsaved changes in \"%1\". Do you want to save before closing?")).arg(currentCommandName));
    msgBox.setIcon(QMessageBox::Warning);
    
    QPushButton *saveButton = msgBox.addButton(tr("Save and Close"), QMessageBox::AcceptRole);
    QPushButton *dontSaveButton = msgBox.addButton(tr("Don't Save"), QMessageBox::DestructiveRole);
    QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    
    msgBox.setDefaultButton(saveButton);
    msgBox.exec();
    
    if (msgBox.clickedButton() == saveButton) {
        return QMessageBox::Save;
    } else if (msgBox.clickedButton() == dontSaveButton) {
        return QMessageBox::Discard;
    } else {
        return QMessageBox::Cancel;
    }
}

void MainWindow::onNewClicked() {
    MainWindow *newWindow = new MainWindow();
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}

void MainWindow::onSaveClicked() {
    QString commandText = commandEdit->toPlainText().trimmed();
    if (commandText.isEmpty()) {
        QMessageBox::warning(this, tr("No Command"), tr("Please enter a command to save."));
        return;
    }
    
    QString name;
    bool isExistingCommand = !currentCommandName.isEmpty();
    
    if (isExistingCommand) {
        // Updating existing command - show confirmation
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Update Command"),
            QString(tr("Do you want to update the existing command \"%1\" with these changes?")).arg(currentCommandName),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
        name = currentCommandName;
    } else {
        // Creating new command - ask for name
        bool ok;
        name = QInputDialog::getText(this, tr("Save Command"), 
                                    tr("Enter a name for this command:"), 
                                    QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty()) {
            return;
        }
        name = name.trimmed();
    }
    
    saveCommandToJson(name, commandText, fileRows);
    
    if (!isExistingCommand) {
        // If this was a new command, set it as the current command
        setCurrentCommandName(name);
        openedCommands.insert(name);
    } else {
        // Reset change tracking for existing command
        hasUnsavedChanges = false;
        storeOriginalState();
        updateWindowTitle();
    }

    // Reset "any changes" after successful save
    hasAnyChanges = false;
    
    QMessageBox::information(this, tr("Command Saved"), 
                           QString(tr("Command '%1' has been saved successfully.")).arg(name));
}

void MainWindow::onAllCommandsClicked() {
    CommandsMenuDialog *dialog = new CommandsMenuDialog(nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->updateCommandsList();
    dialog->show();
}

void MainWindow::onCommandTextChanged() {
    // Track if the user has made any changes to the text area
    if (!commandEdit->toPlainText().trimmed().isEmpty()) {
      hasAnyChanges = true;
    }
    // Check if we're in execute mode (file rows are shown)
    if (executeButton->isVisible()) {
        QString currentText = commandEdit->toPlainText();
        QString normalizedCurrent = currentText.simplified(); // Remove extra whitespace
        QString normalizedTemplate = commandTemplate.simplified();
        
        // If the command has changed beyond just whitespace/newlines, reset to start mode
        if (normalizedCurrent != normalizedTemplate) {
            // Reset to start mode
            executeButton->hide();
            clearButton->hide();
            startButton->show();
            
            // Safely clear file rows first
            for (FileRowWidget* row : fileRows) {
                if (row) {
                    // Disconnect signals to prevent further signal emissions
                    disconnect(row, nullptr, this, nullptr);
                    row->deleteLater(); // Use deleteLater instead of direct delete
                }
            }
            fileRows.clear();
            
            // Hide and remove file groups
            if (filesGroup) {
                mainLayout->removeWidget(filesGroup);
                filesGroup->deleteLater(); // Use deleteLater instead of direct delete
                filesGroup = nullptr;
                filesGroupAdded = false;
            }
            
            // Clear command template
            commandTemplate.clear();
        }
    }
    
    // Check for changes for unsaved changes tracking
    checkForChanges();
}

void MainWindow::onFileChanged() {
    checkForChanges();
}

void MainWindow::checkForChanges() {
    if (currentCommandName.isEmpty()) return;
    
    bool changed = hasChanges();
    if (changed != hasUnsavedChanges) {
        hasUnsavedChanges = changed;
        updateWindowTitle();
    }
}

bool MainWindow::hasChanges() const {
    if (currentCommandName.isEmpty()) return false;
    
    // Check command text
    if (commandEdit->toPlainText() != originalCommandText) return true;
    
    // Check directory
    if (currentDir != originalDirectory) return true;
    
    // Check files
    if (fileRows.size() != originalFilesData.size()) return true;
    
    for (int i = 0; i < fileRows.size(); ++i) {
        if (i >= originalFilesData.size()) return true;
        
        QJsonObject originalFile = originalFilesData[i].toObject();
        FileRowWidget *row = fileRows[i];
        
        if (row->getPlaceholder() != originalFile["placeholder"].toString() ||
            row->getSelectedFile() != originalFile["selectedFile"].toString() ||
            row->getRole() != originalFile["role"].toString()) {
            return true;
        }
    }
    
    return false;
}

void MainWindow::storeOriginalState() {
    originalCommandText = commandEdit->toPlainText();
    originalDirectory = currentDir;
    
    originalFilesData = QJsonArray();
    for (FileRowWidget *row : fileRows) {
        QJsonObject fileData;
        fileData["placeholder"] = row->getPlaceholder();
        fileData["selectedFile"] = row->getSelectedFile();
        fileData["role"] = row->getRole();
        originalFilesData.append(fileData);
    }
}

void MainWindow::updateWindowTitle() {
    QString title = tr("CMD Manager");
    if (!currentCommandName.isEmpty()) {
        title += QString(" - %1").arg(currentCommandName);
        if (hasUnsavedChanges) {
            title += " *";
        }
    }
    setWindowTitle(title);
}

void MainWindow::clearCommandsInternal() {
    // Clear command text
    commandEdit->clear();

    // Safely delete all file rows
    for (FileRowWidget* row : fileRows) {
        if (row) {
            disconnect(row, nullptr, this, nullptr);
            row->deleteLater();
        }
    }
    fileRows.clear();

    // Remove and delete filesGroup if it exists
    if (filesGroup) {
        mainLayout->removeWidget(filesGroup);
        filesGroup->deleteLater();
        filesGroup = nullptr;
    }

    // Reset buttons
    executeButton->hide();
    clearButton->hide();
    startButton->show();
    filesGroupAdded = false;
    
    // Clear command template
    commandTemplate.clear();
    
    // Clear command name and reset change tracking
    currentCommandName.clear();
    hasUnsavedChanges = false;
    hasAnyChanges = false;
    updateWindowTitle();
}

void MainWindow::onClearClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Clear Commands"),
                                  tr("Are you sure you want to clear the command and reset everything?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // Remove from opened commands if clearing
        if (!currentCommandName.isEmpty()) {
            openedCommands.remove(currentCommandName);
        }
        
        clearCommandsInternal();
        // Also reset directory when manually clearing
        currentDir.clear();
        updateDirectoryButton();
    }
}

void MainWindow::saveCommandToJson(const QString &name, const QString &command, const QList<FileRowWidget*> &fileRows) {
    QJsonObject commands = loadCommandsFromJson();
    
    QJsonObject commandData;
    commandData["command"] = command;
    commandData["directory"] = currentDir;
    
    // Add metadata
    if (commands.contains(name)) {
        // Preserve existing metadata when updating
        QJsonObject existing = commands[name].toObject();
        commandData["pinned"] = existing.contains("pinned") ? existing.value("pinned").toBool() : false;
        commandData["lastOpened"] = existing.contains("lastOpened") ? existing.value("lastOpened").toDouble() : 0.0;
    } else {
        // New command
        commandData["pinned"] = false;
        commandData["lastOpened"] = QDateTime::currentMSecsSinceEpoch();
    }
    
    QJsonArray filesArray;
    for (FileRowWidget *row : fileRows) {
        QJsonObject fileData;
        fileData["placeholder"] = row->getPlaceholder();
        fileData["selectedFile"] = row->getSelectedFile();
        fileData["role"] = row->getRole();
        filesArray.append(fileData);
    }
    commandData["files"] = filesArray;
    
    commands[name] = commandData;
    
    QString configPath = getConfigFilePath();
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(commands);
        file.write(doc.toJson());
        file.close();

        // Notify that commands have changed
        SettingsManager::instance()->commandsChanged();
    }
}

QJsonObject MainWindow::loadCommandsFromJson() {
    QString configPath = getConfigFilePath();
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc.object();
}

QString MainWindow::getConfigFilePath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + "/CMDManager/commands.json";
}

void MainWindow::loadCommand(const QJsonObject &commandData) {
    // Clear existing state without confirmation
    // Don't use clearCommandsInternal() here as it clears currentCommandName
    
    // Clear command text
    commandEdit->clear();

    // Safely delete all file rows
    for (FileRowWidget* row : fileRows) {
        if (row) {
            disconnect(row, nullptr, this, nullptr);
            row->deleteLater();
        }
    }
    fileRows.clear();

    // Remove and delete filesGroup if it exists
    if (filesGroup) {
        mainLayout->removeWidget(filesGroup);
        filesGroup->deleteLater();
        filesGroup = nullptr;
    }

    // Reset buttons
    executeButton->hide();
    clearButton->hide();
    startButton->show();
    filesGroupAdded = false;
    
    // Clear command template
    commandTemplate.clear();
    
    // Load command text
    commandEdit->setPlainText(commandData["command"].toString());
    
    // Load directory
    currentDir = commandData["directory"].toString();
    updateDirectoryButton();
    
    // Simulate clicking Start to create file rows
    onStartClicked();
    
    // Load file data
    QJsonArray filesArray = commandData["files"].toArray();
    for (int i = 0; i < filesArray.size() && i < fileRows.size(); ++i) {
        QJsonObject fileData = filesArray[i].toObject();
        FileRowWidget *row = fileRows[i];
        
        // Connect file change signals
        connect(row, &FileRowWidget::fileChanged, this, &MainWindow::onFileChanged);
        
        // Set role
        row->setRole(fileData["role"].toString());
        
        // Set selected file
        QString selectedFile = fileData["selectedFile"].toString();
        if (!selectedFile.isEmpty()) {
            row->setSelectedFile(selectedFile);
        }
    }

    // Apply current theme and font to all widgets (including newly created file rows)
    applyThemeAndFont();
    
    // Store original state for change detection
    storeOriginalState();
    
    // Reset unsaved changes flag after loading
    hasUnsavedChanges = false;
    hasAnyChanges = false;
    updateWindowTitle();
}

void MainWindow::updateDirectoryButton() {
    if (currentDir.isEmpty()) {
        chooseDirButton->setText(tr("Current Directory: (Not Set)"));
    } else {
        QFileInfo dirInfo(currentDir);
        QString displayText = dirInfo.fileName();
        if (displayText.isEmpty()) {
            displayText = currentDir;
        }
        
        if (displayText.length() > 30) {
            displayText = "..." + displayText.right(27);
        }
        
        chooseDirButton->setText(QString(tr("Current Directory: %1")).arg(displayText));
        chooseDirButton->setToolTip(currentDir);
    }
}

void MainWindow::onChooseDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), currentDir);
    if (!dir.isEmpty()) {
        currentDir = dir;
        updateDirectoryButton();
        checkForChanges(); // Check for changes when directory changes
    }
}

void MainWindow::onStartClicked() {
    // Safely clear old rows if they exist
    for (FileRowWidget* row : fileRows) {
        if (row) {
            disconnect(row, nullptr, this, nullptr);
            row->deleteLater();
        }
    }
    fileRows.clear();

    QString cmdText = commandEdit->toPlainText();
    if (cmdText.isEmpty()) {
        QMessageBox::warning(this, tr("No Command"), tr("Please enter a command first."));
        return;
    }

    // Clean up existing files group
    if (filesGroup) {
        mainLayout->removeWidget(filesGroup);
        filesGroup->deleteLater();
        filesGroup = nullptr;
        filesGroupAdded = false;
    }

    // Detect placeholder files in the command
    QRegularExpression fileRegex(R"(\b[\w.-]+\.[A-Za-z0-9]+\b|<[\w-]+>)");
    QRegularExpressionMatchIterator it = fileRegex.globalMatch(cmdText);

    QStringList detectedFiles;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString fileName = match.captured(0);
        if (!detectedFiles.contains(fileName)) {
            detectedFiles.append(fileName);
        }
    }

    // Build the layout for file rows
    QVBoxLayout *filesLayout = new QVBoxLayout;

    for (const QString &file : detectedFiles) {
        FileRowWidget *row = new FileRowWidget(file, this);
        connect(row, &FileRowWidget::fileChanged, this, &MainWindow::onFileChanged);
        filesLayout->addWidget(row);
        fileRows.append(row);
    }

    filesLayout->addStretch();

    // Create a group box to hold rows
    filesGroup = new QGroupBox(tr("Files"));
    filesGroup->setLayout(filesLayout);

    // Remove the Start button, replace with Execute
    startButton->hide();
    executeButton->show();
    clearButton->show();

    // Add group to the main layout
    mainLayout->addWidget(filesGroup);
    filesGroupAdded = true;

    // Apply the current theme stylesheet to ensure styling is correct
    SettingsManager* settings = SettingsManager::instance();
    QString currentStyleSheet = settings->getCurrentThemeStyleSheet();
    QFont appFont(settings->getFontFamily(), settings->getFontSize());
    
    // Apply to the group box and all file rows
    filesGroup->setStyleSheet(currentStyleSheet);
    for (FileRowWidget *row : fileRows) {
        row->setStyleSheet(currentStyleSheet);
        row->setFont(appFont);
        // Also apply to children
        QList<QWidget*> children = row->findChildren<QWidget*>();
        for (QWidget* child : children) {
            child->setStyleSheet(currentStyleSheet);
            child->setFont(appFont);
        }
    }

    // Also apply to the files group box
    if (filesGroup) {
        filesGroup->setFont(appFont);
    }

    // Store the command template (store the actual text, not simplified)
    commandTemplate = cmdText;
    
    // Check for changes after creating file rows
    checkForChanges();
}

void MainWindow::onExecuteClicked() {
    QString finalCmd = buildFinalCommand();

    // Show command preview if enabled in settings
    if (SettingsManager::instance()->getShowCommandPreview()) {
        if (!showCommandPreview(finalCmd)) {
            // User cancelled
            return;
        }
    }
    
    // Replace placeholders with properly quoted file paths
    for (FileRowWidget *row : fileRows) {
        if (!row->getSelectedFile().isEmpty()) {
            QString filePath = row->getSelectedFile();
            // Escape any single quotes in the file path and wrap in single quotes
            filePath.replace("'", "'\"'\"'"); // Replace ' with '"'"'
            QString quotedPath = "'" + filePath + "'";
            finalCmd.replace(row->getPlaceholder(), quotedPath);
        }
    }

    // Create independent terminal dialog
    QDialog *terminal = new QDialog(nullptr); // No parent - makes it independent
    terminal->setAttribute(Qt::WA_DeleteOnClose);
    terminal->setWindowTitle(tr("CMD Manager - Command Execution"));
    QVBoxLayout *layout = new QVBoxLayout(terminal);

    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);

    SettingsManager* settings = SettingsManager::instance();
    QString currentStyleSheet = settings->getCurrentThemeStyleSheet();
    terminal->setStyleSheet(currentStyleSheet);

    // Apply terminal settings
    QPair<QColor, QColor> colors = SettingsManager::getTerminalColors(settings->getTerminalColorScheme());
    QString terminalStyle = QString(
        "QTextEdit { "
        "background-color: %1; "
        "color: %2; "
        "font-family: '%3'; "
        "font-size: %4pt; "
        "border: none; "
        "}"
    ).arg(colors.first.name())
     .arg(colors.second.name())
     .arg(settings->getTerminalFontFamily())
     .arg(settings->getTerminalFontSize());
    output->setStyleSheet(terminalStyle);

    layout->addWidget(output);

    QPushButton *closeBtn = new QPushButton(tr("Close"));
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, terminal, &QDialog::close);

    QString workingDir = currentDir.isEmpty() ? QDir::homePath() : currentDir;
    if (SettingsManager::instance()->getShowCommandLabel()) {
        output->append(tr("Working Directory: ") + workingDir);
        output->append(tr("Executing Command:"));
        output->append(finalCmd);
        output->append(tr("======================= START COMMAND ======================="));
    }

    QProcess *terminalProcess = new QProcess(terminal); // Use local process
    connect(terminalProcess, &QProcess::readyReadStandardOutput, [=]() {
        QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardOutput());
        output->append(data);
        output->moveCursor(QTextCursor::End);
    });
    connect(terminalProcess, &QProcess::readyReadStandardError, [=]() {
        QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardError());
        output->append(data);
        output->moveCursor(QTextCursor::End);
    });
    connect(terminalProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (SettingsManager::instance()->getShowCommandLabel()) {
            output->append(tr("======================== END COMMAND ========================"));
        }
        output->append(QString(tr("Process finished with exit code: %1")).arg(exitCode));
        if (exitStatus == QProcess::CrashExit) {
            output->append(tr("Process crashed!"));
        }
        output->moveCursor(QTextCursor::End);

        // Play sound if enabled in settings
        if (SettingsManager::instance()->getPlayCompletionSound()) {
            QTimer::singleShot(1000, []() {
                Utils::playNotificationSound();  // Play Notification sound on complete
            });
        }

        // Auto-close if enabled
        if (SettingsManager::instance()->getAutoCloseTerminal()) {
            QTimer::singleShot(1000, terminal, &QDialog::close); // Close after 1 second
        }
    });

    terminalProcess->setWorkingDirectory(workingDir);
    
    // Execute the command
    terminalProcess->start("/bin/bash", QStringList() << "-c" << finalCmd);
    
    if (!terminalProcess->waitForStarted(3000)) {
        output->append(tr("Error: Failed to start the process!"));
    }

    terminal->resize(800, 500);
    terminal->show(); // Use show() instead of exec() to make it non-modal
}

void MainWindow::onInputButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int idx = buttonIndexMap[btn];
    QString ext = QFileInfo(placeholders[idx].original).suffix();
    QString file = QFileDialog::getOpenFileName(this, tr("Select Input File"), currentDir,
                                               QString("*.%1").arg(ext));
    if (!file.isEmpty()) {
        placeholders[idx].chosenPath = file;
        btn->setText(QString(tr("Input %1: %2")).arg(idx + 1).arg(QFileInfo(file).fileName()));
    }
}

void MainWindow::onOutputButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int idx = buttonIndexMap[btn];
    QString ext = QFileInfo(placeholders[idx].original).suffix();
    QString file = QFileDialog::getSaveFileName(this, tr("Select Output File"), currentDir,
                                               QString("*.%1").arg(ext));
    if (!file.isEmpty()) {
        placeholders[idx].chosenPath = file;
        btn->setText(QString(tr("Output %1: %2")).arg(idx + 1).arg(QFileInfo(file).fileName()));
    }
}

void MainWindow::parseCommandForFiles(const QString &cmd) {
    placeholders.clear();
    QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);

    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i].contains('.')) {
            bool isOutput = false;
            if (i > 0 && parts[i-1] != "-i") {
                isOutput = true;
            }
            placeholders.append({parts[i], "", isOutput});
        }
    }
}

QString MainWindow::buildFinalCommand() const {
    QString finalCmd = commandTemplate;

    // Replace placeholders with properly quoted file paths
    for (FileRowWidget *row : fileRows) {
        if (!row->getSelectedFile().isEmpty()) {
            QString filePath = row->getSelectedFile();
            // Escape any single quotes in the file path and wrap in single quotes
            filePath.replace("'", "'\"'\"'");
            QString quotedPath = "'" + filePath + "'";
            finalCmd.replace(row->getPlaceholder(), quotedPath);
        }
    }
    
    return finalCmd;
}

bool MainWindow::containsDangerousCommands(const QString &command) const {
    QStringList dangerousPatterns = {
        "rm ", "rm\t", "rm\n",
        "rmdir ", "rmdir\t",
        "dd ", "dd\t",
        "mkfs", 
        "format",
        ":(){:|:&};:",  // Fork bomb
        "chmod -R 777",
        "chmod 777",
        "> /dev/",
        "mv ", "mv\t",  // Moving files can be dangerous
        "shred",
        "wipefs",
        "fdisk",
        "parted",
        "shutdown",
        "reboot",
        "init 0",
        "init 6",
        "kill -9",
        "killall",
        "pkill"
    };
    
    QString lowerCmd = command.toLower();
    
    for (const QString &pattern : dangerousPatterns) {
        if (lowerCmd.contains(pattern.toLower())) {
            return true;
        }
    }
    
    return false;
}

QStringList MainWindow::getDangerousCommands(const QString &command) const {
    QStringList dangerous;
    QStringList dangerousPatterns = {
        "rm", "rmdir", "dd", "mkfs", "format", "fork bomb",
        "chmod 777", "mv", "shred", "wipefs", "fdisk", "parted",
        "shutdown", "reboot", "init", "kill", "killall", "pkill"
    };
    
    QString lowerCmd = command.toLower();
    
    QMap<QString, QString> patternMap;
    patternMap["rm "] = "rm (delete files/directories)";
    patternMap["rmdir "] = "rmdir (remove directories)";
    patternMap["dd "] = "dd (disk write - can destroy data)";
    patternMap["mkfs"] = "mkfs (format filesystem)";
    patternMap["format"] = "format (format disk)";
    patternMap[":(){:|:&};:"] = "Fork bomb (system crash)";
    patternMap["chmod -r 777"] = "chmod 777 (dangerous permissions)";
    patternMap["chmod 777"] = "chmod 777 (dangerous permissions)";
    patternMap["> /dev/"] = "Writing to device files";
    patternMap["mv "] = "mv (move/rename files)";
    patternMap["shred"] = "shred (secure delete)";
    patternMap["wipefs"] = "wipefs (erase filesystem signatures)";
    patternMap["fdisk"] = "fdisk (partition editor)";
    patternMap["parted"] = "parted (partition editor)";
    patternMap["shutdown"] = "shutdown (system shutdown)";
    patternMap["reboot"] = "reboot (system restart)";
    patternMap["init 0"] = "init 0 (system shutdown)";
    patternMap["init 6"] = "init 6 (system reboot)";
    patternMap["kill -9"] = "kill -9 (force kill process)";
    patternMap["killall"] = "killall (kill all processes by name)";
    patternMap["pkill"] = "pkill (kill processes by pattern)";
    
    for (auto it = patternMap.begin(); it != patternMap.end(); ++it) {
        if (lowerCmd.contains(it.key().toLower())) {
            if (!dangerous.contains(it.value())) {
                dangerous.append(it.value());
            }
        }
    }
    
    return dangerous;
}

bool MainWindow::showCommandPreview(const QString &command) {
    QDialog *previewDialog = new QDialog(this);
    previewDialog->setWindowTitle(tr("Command Preview"));
    previewDialog->setModal(true);
    previewDialog->resize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(previewDialog);
    
    // Check for dangerous commands
    bool isDangerous = containsDangerousCommands(command);
    QStringList dangerousCmds = getDangerousCommands(command);
    
    if (isDangerous) {
        QLabel *warningIcon = new QLabel();
        warningIcon->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxWarning).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QString warningLabelStr = QString("<b>⚠ %1</b>")
          .arg(tr("Warning: Potentially Dangerous Command Detected!"));
        
        QLabel *warningLabel = new QLabel(warningLabelStr);
        warningLabel->setStyleSheet("QLabel { color: #ff6b6b; font-size: 14pt; }");
        
        QHBoxLayout *warningLayout = new QHBoxLayout();
        warningLayout->addWidget(warningIcon);
        warningLayout->addWidget(warningLabel);
        warningLayout->addStretch();
        
        layout->addLayout(warningLayout);

        QString dangerousLabelStr = QString("<b>%1</b>").arg(tr("Detected dangerous operations:"));
        
        QLabel *dangerousLabel = new QLabel(dangerousLabelStr);
        layout->addWidget(dangerousLabel);
        
        QTextEdit *dangerousList = new QTextEdit();
        dangerousList->setReadOnly(true);
        dangerousList->setMaximumHeight(80);
        dangerousList->setText("• " + dangerousCmds.join("\n• "));
        layout->addWidget(dangerousList);
        
        QLabel *cautionLabel = new QLabel(tr("Please review the command carefully before executing."));
        cautionLabel->setStyleSheet("QLabel { color: #ff6b6b; font-style: italic; }");
        layout->addWidget(cautionLabel);
    } else {
        QString safeLabelStr = QString("✓ %1").arg(tr("No obvious dangerous commands detected"));
        QLabel *safeLabel = new QLabel(safeLabelStr);
        safeLabel->setStyleSheet("QLabel { color: #51cf66; font-weight: bold; }");
        layout->addWidget(safeLabel);
    }
    
    // Working directory
    QString dirLabelStr = QString("<b>%1</b>").arg(tr("Working Directory:"));
    QLabel *dirLabel = new QLabel(dirLabelStr);
    layout->addWidget(dirLabel);
    
    QString workingDir = currentDir.isEmpty() ? QDir::homePath() : currentDir;
    QLineEdit *dirEdit = new QLineEdit(workingDir);
    dirEdit->setReadOnly(true);
    layout->addWidget(dirEdit);
    
    // Command preview
    QString cmdLabelStr = QString("<b>%1</b>").arg(tr("Command to Execute:"));
    QLabel *cmdLabel = new QLabel(cmdLabelStr);
    layout->addWidget(cmdLabel);
    
    QTextEdit *commandPreview = new QTextEdit();
    commandPreview->setReadOnly(true);
    commandPreview->setPlainText(command);
    commandPreview->setFont(QFont("Courier", 10));
    layout->addWidget(commandPreview);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *executeBtn = new QPushButton(tr("Execute"));
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    
    if (isDangerous) {
        executeBtn->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; font-weight: bold; }");
    } else {
        executeBtn->setStyleSheet("QPushButton { background-color: #51cf66; color: white; font-weight: bold; }");
    }
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(executeBtn);
    
    layout->addLayout(buttonLayout);
    
    connect(executeBtn, &QPushButton::clicked, previewDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, previewDialog, &QDialog::reject);
    
    int result = previewDialog->exec();
    previewDialog->deleteLater();
    
    return result == QDialog::Accepted;
}

void MainWindow::clearDynamicButtons() {
    QLayoutItem *item;
    while ((item = dynamicButtonLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    placeholders.clear();
    buttonIndexMap.clear();
}

CommandsMenuDialog::CommandsMenuDialog(QWidget *parent) 
  : QDialog(parent), currentSortMode(SortByName), currentSearchText("") {
    setWindowTitle(tr("Commands Menu"));
    setModal(false); // Make it non-modal
    setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    resize(600, 400);
    
    // Center the dialog on screen if no parent
    if (!parent) {
        setWindowFlags(Qt::Window); // Make it a top-level window
        
        // Center on screen using QScreen (modern Qt way)
        if (QApplication::primaryScreen()) {
            QRect screenGeometry = QApplication::primaryScreen()->geometry();
            int x = (screenGeometry.width() - width()) / 2;
            int y = (screenGeometry.height() - height()) / 2;
            move(x, y);
        }
    }
    
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Search and sort controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    QLabel *searchLabel = new QLabel(tr("Search:"));
    searchBox = new QLineEdit();
    searchBox->setPlaceholderText(tr("Search Commands..."));
    
    QLabel *sortLabel = new QLabel(tr("Sort By:"));
    sortCombo = new QComboBox();
    sortCombo->addItem(tr("Name"), SortByName);
    sortCombo->addItem(tr("Recently Opened"), SortByRecentlyOpened);
    
    controlsLayout->addWidget(searchLabel);
    controlsLayout->addWidget(searchBox, 1);
    controlsLayout->addWidget(sortLabel);
    controlsLayout->addWidget(sortCombo);
    
    layout->addLayout(controlsLayout);

    
    commandsList = new QListWidget(this);
    layout->addWidget(new QLabel(tr("Saved Commands:")));
    commandsList->setAlternatingRowColors(false);
    commandsList->setMouseTracking(true);
    commandsList->viewport()->installEventFilter(this);
    layout->addWidget(commandsList, 1);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    
    newButton = new QPushButton(tr("New"));
    openButton = new QPushButton(tr("Open"));
    runButton = new QPushButton(tr("Run"));
    pinButton = new QPushButton(tr("Pin/Unpin"));
    deleteButton = new QPushButton(tr("Delete"));
    
    buttonsLayout->addWidget(newButton);
    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(runButton);
    buttonsLayout->addWidget(pinButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addStretch();
    
    layout->addLayout(buttonsLayout);
    
    connect(newButton, &QPushButton::clicked, this, &CommandsMenuDialog::onNewClicked);
    connect(openButton, &QPushButton::clicked, this, &CommandsMenuDialog::onCommandSelected);
    connect(runButton, &QPushButton::clicked, this, &CommandsMenuDialog::onRunCommand);
    connect(pinButton, &QPushButton::clicked, this, &CommandsMenuDialog::onPinToggle);
    connect(deleteButton, &QPushButton::clicked, this, &CommandsMenuDialog::onDeleteCommand);
    connect(commandsList, &QListWidget::itemDoubleClicked, this, &CommandsMenuDialog::onCommandSelected);

    connect(searchBox, &QLineEdit::textChanged, this, &CommandsMenuDialog::onSearchTextChanged);
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CommandsMenuDialog::onSortChanged);

    // Connect to commands changed signal for auto-refresh
    connect(SettingsManager::instance(), &SettingsManager::commandsChanged, this, &CommandsMenuDialog::refreshCommandsList);
    
    refreshCommandsList();

    // Apply current theme after dialog is created
    SettingsManager* settings = SettingsManager::instance();
    setStyleSheet(settings->getCurrentThemeStyleSheet());
    
    QFont appFont(settings->getFontFamily(), settings->getFontSize());
    setFont(appFont);
    
    // Apply to all the child widgets
    QList<QWidget*> widgets = findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        widget->setFont(appFont);
    }
}

void CommandsMenuDialog::onNewClicked() {
    MainWindow *newWindow = new MainWindow();
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}

void CommandsMenuDialog::refreshCommandsList() {
    currentSearchText = searchBox->text();
    currentSortMode = static_cast<SortMode>(sortCombo->currentData().toInt());
    updateCommandsList();
}

void CommandsMenuDialog::updateCommandsList() {
    commandsList->clear();
    QJsonObject commands = loadCommandsFromJson();
    
    // Create list of command items with metadata
    struct CommandItem {
        QString name;
        QString command;
        bool pinned;
        qint64 lastOpened;
    };
    
    QList<CommandItem> items;
    for (auto it = commands.begin(); it != commands.end(); ++it) {
        QJsonObject cmdData = it.value().toObject();
        QString commandText = cmdData["command"].toString();
        
        // Apply search filter
        if (!currentSearchText.isEmpty()) {
            bool matchesName = it.key().contains(currentSearchText, Qt::CaseInsensitive);
            bool matchesCommand = commandText.contains(currentSearchText, Qt::CaseInsensitive);
            if (!matchesName && !matchesCommand) {
                continue;
            }
        }
        
        CommandItem item;
        item.name = it.key();
        item.command = commandText;
        item.pinned = cmdData.contains("pinned") ? cmdData.value("pinned").toBool() : false;
        item.lastOpened = cmdData.contains("lastOpened") ? cmdData.value("lastOpened").toDouble() : 0.0;
        items.append(item);
    }
    
    // Sort items
    if (currentSortMode == SortByRecentlyOpened) {
        std::sort(items.begin(), items.end(), [](const CommandItem &a, const CommandItem &b) {
            // Pinned items always first
            if (a.pinned != b.pinned) return a.pinned > b.pinned;
            // Then by last opened (most recent first)
            return a.lastOpened > b.lastOpened;
        });
    } else { // SortByName
        std::sort(items.begin(), items.end(), [](const CommandItem &a, const CommandItem &b) {
            // Pinned items always first
            if (a.pinned != b.pinned) return a.pinned > b.pinned;
            // Then alphabetically
            return a.name.toLower() < b.name.toLower();
        });
    }

    // Get theme color for icon
    SettingsManager* settings = SettingsManager::instance();
    QColor iconColor;
    switch (settings->getTheme()) {
        case SettingsManager::Light:
            iconColor = QColor("#333333");
            break;
        case SettingsManager::Contrast:
            iconColor = QColor("#FFFFFF");
            break;
        default: // Dark
            iconColor = QColor("#E0E0E0");
            break;
    }

    // Add items to list
    for (const CommandItem &item : items) {
        QListWidgetItem *listItem = new QListWidgetItem(item.name);

        QString pinnedText = "";
        
        // Add pin icon for pinned items
        if (item.pinned) {
            QIcon pinIcon = createColoredIcon(":/images/pin_icon.svg", iconColor);
            listItem->setIcon(pinIcon);
            pinnedText = tr("Pinned");
            
            // Bold font for pinned items
            QFont font = listItem->font();
            font.setBold(true);
            listItem->setFont(font);
        }

        qint64 timestamp = item.lastOpened;
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
        QString formattedDate = dateTime.toString("MMM d, yyyy hh:mm AP");

        
        // Create rich tooltip with command preview
        QString tooltip = QString(
            "<div style='"
            "background-color: #1e1e1e;"
            "color: #d4d4d4;"
            "font-family: &quot;Fira Code&quot;, &quot;Courier New&quot;, monospace;"
            "font-size: 14px;"
            "padding: 12px 16px;"
            "border-radius: 8px;"
            "box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);"
            "max-width: 600px;"
            "white-space: pre-wrap;"
            "line-height: 1.5;'>"
            "<div style='margin-bottom: 8px;'>"
            "<strong style='color: #9cdcfe;'>%1</strong> &nbsp;•&nbsp; "
            "<span style='color: %2;'>%3</span> &nbsp;•&nbsp; "
            "<span style='color: #808080;'>Last opened: %4</span>"
            "</div>"
            "<div>%5</div>"
            "</div>"
        )
          .arg(item.name)
          .arg(iconColor.name())
          .arg(pinnedText)
          .arg(formattedDate)
          .arg(item.command.toHtmlEscaped().replace("\n", "<br/>"));
        
        listItem->setToolTip(tooltip);
        
        commandsList->addItem(listItem);
    }
}

void CommandsMenuDialog::onSearchTextChanged(const QString &text) {
    currentSearchText = text;
    updateCommandsList();
}

void CommandsMenuDialog::onSortChanged(int index) {
    currentSortMode = static_cast<SortMode>(sortCombo->itemData(index).toInt());
    updateCommandsList();
}

void CommandsMenuDialog::onPinToggle() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a command to pin / unpin."));
        return;
    }
    
    QString commandName = currentItem->text();
    togglePin(commandName);
    updateCommandsList();
    
    // Reselect the item
    for (int i = 0; i < commandsList->count(); ++i) {
        if (commandsList->item(i)->text().contains(commandName)) {
            commandsList->setCurrentRow(i);
            break;
        }
    }
}

void CommandsMenuDialog::togglePin(const QString &commandName) {
    QJsonObject commands = loadCommandsFromJson();
    
    if (commands.contains(commandName)) {
        QJsonObject commandData = commands[commandName].toObject();
        bool currentlyPinned = commandData.contains("pinned") ? commandData.value("pinned").toBool() : false;
        commandData["pinned"] = !currentlyPinned;
        commands[commandName] = commandData;
        saveCommandsToJson(commands);

        // Refresh all commands menu after pin or unpin
        SettingsManager::instance()->commandsChanged();
    }
}

void CommandsMenuDialog::onCommandSelected() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a command to open."));
        return;
    }
    
    QString commandName = currentItem->text();
    
    if (MainWindow::openedCommands.contains(commandName)) {
        QMessageBox::warning(this, tr("Command Already Open"), 
                            QString(tr("The command '%1' is already opened in another window.")).arg(commandName));
        return;
    }
    
    QJsonObject commands = loadCommandsFromJson();
    
    if (commands.contains(commandName)) {
        QJsonObject commandData = commands[commandName].toObject();

        // Update last opened timestamp with CURRENT time
        commandData["lastOpened"] = QDateTime::currentMSecsSinceEpoch();
        commands[commandName] = commandData;
        saveCommandsToJson(commands);

        MainWindow *newWindow = new MainWindow();
        newWindow->setAttribute(Qt::WA_DeleteOnClose);
        newWindow->setCurrentCommandName(commandName);
        MainWindow::openedCommands.insert(commandName);
        newWindow->loadCommand(commandData);
        newWindow->show();
        
        // Refresh list to show updated order if sorted by recently opened
        if (currentSortMode == SortByRecentlyOpened) {
            refreshCommandsList();
        }
    }
}

void CommandsMenuDialog::onDeleteCommand() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a command to delete."));
        return;
    }
    
    QString commandName = currentItem->text();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete Command"),
                                  QString(tr("Are you sure you want to delete the command '%1'?")).arg(commandName),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject commands = loadCommandsFromJson();

        commands.remove(commandName);
        saveCommandsToJson(commands);

        // Emit signal to refresh all open command menus
        SettingsManager::instance()->commandsChanged();

        refreshCommandsList();
        QMessageBox::information(this, tr("Command Deleted"), 
                                QString(tr("Command '%1' has been deleted.")).arg(commandName));
    }
}

QJsonObject CommandsMenuDialog::loadCommandsFromJson() {
    QString configPath = getConfigFilePath();
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc.object();
}

void CommandsMenuDialog::saveCommandsToJson(const QJsonObject &commands) {
    QString configPath = getConfigFilePath();
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(commands);
        file.write(doc.toJson());
        file.close();
    }
}

QString CommandsMenuDialog::getConfigFilePath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + "/CMDManager/commands.json";
}

void MainWindow::onSettingsClicked() {
    SettingsDialog *dialog = new SettingsDialog(this);
    connect(dialog, &QDialog::accepted, this, &MainWindow::applyThemeAndFont);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::applyThemeAndFont() {
    SettingsManager* settingsManager = SettingsManager::instance();
    
    // Get current theme stylesheet
    QString currentStyleSheet = settingsManager->getCurrentThemeStyleSheet();
    
    // Apply stylesheet to main window
    setStyleSheet("");  // Clear first
    setStyleSheet(currentStyleSheet);
    
    // Apply font
    QFont appFont(settingsManager->getFontFamily(), settingsManager->getFontSize());
    setFont(appFont);
    
    // Explicitly apply stylesheet to Files group and all FileRowWidgets
    if (filesGroup) {
        filesGroup->setStyleSheet("");  // Clear
        filesGroup->setStyleSheet(currentStyleSheet);  // Reapply
        filesGroup->setFont(appFont);
    }
    
    for (FileRowWidget *row : fileRows) {
        if (row) {
            row->setStyleSheet("");  // Clear
            row->setStyleSheet(currentStyleSheet);  // Reapply
            row->setFont(appFont);
            
            // Apply to all children of FileRowWidget
            QList<QWidget*> rowChildren = row->findChildren<QWidget*>();
            for (QWidget* child : rowChildren) {
                child->setStyleSheet("");
                child->setStyleSheet(currentStyleSheet);
                child->setFont(appFont);
            }
        }
    }
    
    // Force update all widgets
    QList<QWidget*> widgets = findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        widget->setFont(appFont);
        widget->update();
    }
    
    // Update entire window
    update();
}

void CommandsMenuDialog::onRunCommand() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a command to run."));
        return;
    }
    
    QString commandName = currentItem->text();
    QJsonObject commands = loadCommandsFromJson();
    
    if (commands.contains(commandName)) {
        QJsonObject commandData = commands[commandName].toObject();

        // Update last opened timestamp with current time
        commandData["lastOpened"] = QDateTime::currentMSecsSinceEpoch();
        commands[commandName] = commandData;
        saveCommandsToJson(commands);

        executeCommand(commandData);

        // Refresh list to show order if sorted by recently opened is selected
        if (currentSortMode == SortByRecentlyOpened) {
            refreshCommandsList();
        }
    }
}

void CommandsMenuDialog::executeCommand(const QJsonObject &commandData) {
    QString command = commandData["command"].toString();
    QString directory = commandData["directory"].toString();
    QJsonArray filesArray = commandData["files"].toArray();
    
    if (command.isEmpty()) {
        QMessageBox::warning(this, tr("Empty Command"), tr("The selected command is empty."));
        return;
    }

        // Check if instant run is enabled
    if (SettingsManager::instance()->getInstantRunFromMenu()) {
        // Instant run: use saved file paths directly
        QString finalCmd = command;

        // Replace placeholders with saved file paths
        for (int i = 0; i < filesArray.size(); ++i) {
            QJsonObject fileData = filesArray[i].toObject();
            QString placeholder = fileData["placeholder"].toString();
            QString savedFile = fileData["selectedFile"].toString();
            
            if (!savedFile.isEmpty()) {
                QString filePath = savedFile;
                filePath.replace("'", "'\"'\"'");
                QString quotedPath = "'" + filePath + "'";
                finalCmd.replace(placeholder, quotedPath);
            }
        }
        
        // Create terminal execution window immediately
        QDialog *terminal = new QDialog(nullptr);
        terminal->setAttribute(Qt::WA_DeleteOnClose);
        terminal->setWindowTitle(tr("CMD Manager - Command Execution"));
        
        QVBoxLayout *termLayout = new QVBoxLayout(terminal);

        QTextEdit *output = new QTextEdit();
        output->setReadOnly(true);

        SettingsManager* settings = SettingsManager::instance();
        QString currentStyleSheet = settings->getCurrentThemeStyleSheet();
        terminal->setStyleSheet(currentStyleSheet);

        // Apply terminal settings
        QPair<QColor, QColor> colors = SettingsManager::getTerminalColors(settings->getTerminalColorScheme());
        
        QString terminalStyle = QString(
            "QTextEdit { "
            "background-color: %1; "
            "color: %2; "
            "font-family: '%3'; "
            "font-size: %4pt; "
            "border: none; "
            "}"
        ).arg(colors.first.name())
         .arg(colors.second.name())
         .arg(settings->getTerminalFontFamily())
         .arg(settings->getTerminalFontSize());
        output->setStyleSheet(terminalStyle);

        termLayout->addWidget(output);

        QPushButton *closeBtn = new QPushButton(tr("Close"));
        termLayout->addWidget(closeBtn);
        connect(closeBtn, &QPushButton::clicked, terminal, &QDialog::close);

        // Check if command label should be shown
        QString workingDir = directory.isEmpty() ? QDir::homePath() : directory;
        if (SettingsManager::instance()->getShowCommandLabel()) {
            output->append(tr("Working Directory: ") + workingDir);
            output->append(tr("Executing Command:"));
            output->append(finalCmd);
            output->append(tr("======================= START COMMAND ======================="));
        }

        QProcess *terminalProcess = new QProcess(terminal);
        connect(terminalProcess, &QProcess::readyReadStandardOutput, [=]() {
            QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardOutput());
            output->append(data);
            output->moveCursor(QTextCursor::End);
        });
        connect(terminalProcess, &QProcess::readyReadStandardError, [=]() {
            QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardError());
            output->append(data);
            output->moveCursor(QTextCursor::End);
        });
        connect(terminalProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [=](int exitCode, QProcess::ExitStatus exitStatus) {
            if (SettingsManager::instance()->getShowCommandLabel()) {
                output->append(tr("======================== END COMMAND ========================"));
            }
            output->append(QString(tr("Process finished with exit code: %1")).arg(exitCode));
            if (exitStatus == QProcess::CrashExit) {
                output->append(tr("Process crashed!"));
            }
            output->moveCursor(QTextCursor::End);

            // Play sound if enabled in settings
            if (SettingsManager::instance()->getPlayCompletionSound()) {
                QTimer::singleShot(1000, []() {
                    Utils::playNotificationSound();  // Play Notification sound on complete
                });
            }

            // Auto-close if enabled
            if (SettingsManager::instance()->getAutoCloseTerminal()) {
                QTimer::singleShot(1000, terminal, &QDialog::close); // Close after 1 second
            }
        });

        terminalProcess->setWorkingDirectory(workingDir);
        terminalProcess->start("/bin/bash", QStringList() << "-c" << finalCmd);
        
        if (!terminalProcess->waitForStarted(3000)) {
            output->append(tr("Error: Failed to start the process!"));
        }

        terminal->resize(800, 500);
        terminal->show();
        
        return; // Exit early, skip file configuration dialog
    }
    
    // Create a dialog to show file mappings and allow user to select files
    QDialog *fileDialog = new QDialog(this);
    fileDialog->setWindowTitle(tr("Configure Files for Execution"));
    fileDialog->setModal(true);
    fileDialog->resize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(fileDialog);
    
    // Add command info
    layout->addWidget(new QLabel(QString(tr("Command: %1")).arg(command)));
    layout->addWidget(new QLabel(QString(tr("Directory: %1")).arg(directory.isEmpty() ? tr("(Default)") : directory)));
    
    // Create file row widgets for each file in the command
    QList<FileRowWidget*> fileRows;
    
    if (filesArray.isEmpty()) {
        // If no file data saved, detect files from command text
        QRegularExpression fileRegex(R"(\b[\w.-]+\.[A-Za-z0-9]+\b|<[\w-]+>)");
        QRegularExpressionMatchIterator it = fileRegex.globalMatch(command);
        
        QStringList detectedFiles;
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString fileName = match.captured(0);
            if (!detectedFiles.contains(fileName)) {
                detectedFiles.append(fileName);
            }
        }
        
        for (const QString &file : detectedFiles) {
            FileRowWidget *row = new FileRowWidget(file, fileDialog);
            fileRows.append(row);
            layout->addWidget(row);
        }
    } else {
        // Use saved file data
        for (int i = 0; i < filesArray.size(); ++i) {
            QJsonObject fileData = filesArray[i].toObject();
            QString placeholder = fileData["placeholder"].toString();
            
            FileRowWidget *row = new FileRowWidget(placeholder, fileDialog);
            row->setRole(fileData["role"].toString());
            
            QString selectedFile = fileData["selectedFile"].toString();
            if (!selectedFile.isEmpty()) {
                row->setSelectedFile(selectedFile);
            }
            
            fileRows.append(row);
            layout->addWidget(row);
        }
    }
    
    // Add buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *executeBtn = new QPushButton(tr("Execute"));
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(executeBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);
    
    connect(cancelBtn, &QPushButton::clicked, fileDialog, &QDialog::reject);
    connect(executeBtn, &QPushButton::clicked, [=]() {
        // Build the final command with file substitutions
        QString finalCmd = command;

        // Replace placeholders with properly quoted file paths
        for (FileRowWidget *row : fileRows) {
            if (!row->getSelectedFile().isEmpty()) {
                QString filePath = row->getSelectedFile();
                // Escape any single quotes in the file path and wrap in single quotes
                filePath.replace("'", "'\"'\"'"); // Replace ' with '"'"'
                QString quotedPath = "'" + filePath + "'";
                finalCmd.replace(row->getPlaceholder(), quotedPath);
            }
        }
        
        fileDialog->accept();
        
        // Create terminal execution window
        QDialog *terminal = new QDialog(nullptr); // Independent window
        terminal->setAttribute(Qt::WA_DeleteOnClose);
        terminal->setWindowTitle(tr("CMD Manager - Command Execution"));
        
        QVBoxLayout *termLayout = new QVBoxLayout(terminal);

        QTextEdit *output = new QTextEdit();
        output->setReadOnly(true);

        SettingsManager* settings = SettingsManager::instance();
        QString currentStyleSheet = settings->getCurrentThemeStyleSheet();
        terminal->setStyleSheet(currentStyleSheet);
        
        // Apply terminal settings
        QPair<QColor, QColor> colors = SettingsManager::getTerminalColors(settings->getTerminalColorScheme());
        
        QString terminalStyle = QString(
            "QTextEdit { "
            "background-color: %1; "
            "color: %2; "
            "font-family: '%3'; "
            "font-size: %4pt; "
            "border: none; "
            "}"
        ).arg(colors.first.name())
         .arg(colors.second.name())
         .arg(settings->getTerminalFontFamily())
         .arg(settings->getTerminalFontSize());
        output->setStyleSheet(terminalStyle);

        termLayout->addWidget(output);

        QPushButton *closeBtn = new QPushButton(tr("Close"));
        termLayout->addWidget(closeBtn);
        connect(closeBtn, &QPushButton::clicked, terminal, &QDialog::close);

        // Display the command that will be executed
        QString workingDir = directory.isEmpty() ? QDir::homePath() : directory;
        if (SettingsManager::instance()->getShowCommandLabel()) {
            output->append(tr("Working Directory: ") + workingDir);
            output->append(tr("Executing Command:"));
            output->append(finalCmd);
            output->append(tr("======================= START COMMAND ======================="));
        }

        QProcess *terminalProcess = new QProcess(terminal);
        connect(terminalProcess, &QProcess::readyReadStandardOutput, [=]() {
            QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardOutput());
            output->append(data);
            output->moveCursor(QTextCursor::End);
        });
        connect(terminalProcess, &QProcess::readyReadStandardError, [=]() {
            QString data = QString::fromLocal8Bit(terminalProcess->readAllStandardError());
            output->append(data);
            output->moveCursor(QTextCursor::End);
        });
        connect(terminalProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [=](int exitCode, QProcess::ExitStatus exitStatus) {
            if (SettingsManager::instance()->getShowCommandLabel()) {
                output->append(tr("======================== END COMMAND ========================"));
            }
            output->append(QString(tr("Process finished with exit code: %1")).arg(exitCode));
            if (exitStatus == QProcess::CrashExit) {
                output->append(tr("Process crashed!"));
            }
            output->moveCursor(QTextCursor::End);

            // Play sound if enabled in settings
            if (SettingsManager::instance()->getPlayCompletionSound()) {
                QTimer::singleShot(1000, []() {
                    Utils::playNotificationSound();  // Play Notification Sound on Complete
                });
            }

            // Auto-close if enabled
            if (SettingsManager::instance()->getAutoCloseTerminal()) {
                QTimer::singleShot(1000, terminal, &QDialog::close); // Close after 1 second
            }

        });

        terminalProcess->setWorkingDirectory(workingDir);
        
        // Execute the command
        terminalProcess->start("/bin/bash", QStringList() << "-c" << finalCmd);
        
        if (!terminalProcess->waitForStarted(3000)) {
            output->append(tr("Error: Failed to start the process!"));
        }

        terminal->resize(800, 500);
        terminal->show();
    });
    
    fileDialog->exec();
    fileDialog->deleteLater();
}

void MainWindow::setupKeyboardShortcuts() {
    SettingsManager* settings = SettingsManager::instance();
    
    // Create shortcuts
    newCommandShortcut = new QShortcut(this);
    saveCommandShortcut = new QShortcut(this);
    openCommandsShortcut = new QShortcut(this);
    startExecuteShortcut = new QShortcut(QKeySequence("F5"), this); // F5 for Start+Execute
    
    // Connect shortcuts to actions
    connect(newCommandShortcut, &QShortcut::activated, this, &MainWindow::onNewClicked);
    connect(saveCommandShortcut, &QShortcut::activated, this, &MainWindow::onSaveClicked);
    connect(openCommandsShortcut, &QShortcut::activated, this, &MainWindow::onAllCommandsClicked);
    connect(startExecuteShortcut, &QShortcut::activated, this, &MainWindow::onStartExecuteShortcut);
    
    // Set initial shortcuts from settings
    updateKeyboardShortcuts();
    
    // Update shortcuts when settings change
    connect(settings, &SettingsManager::shortcutsChanged, this, &MainWindow::updateKeyboardShortcuts);
}

void MainWindow::updateKeyboardShortcuts() {
    SettingsManager* settings = SettingsManager::instance();
    
    newCommandShortcut->setKey(QKeySequence(settings->getNewCommandShortcut()));
    saveCommandShortcut->setKey(QKeySequence(settings->getSaveCommandShortcut()));
    openCommandsShortcut->setKey(QKeySequence(settings->getOpenCommandsShortcut()));
    startExecuteShortcut->setKey(QKeySequence(settings->getStartExecuteShortcut()));
    
    // Set tooltips to show shortcuts
    newButton->setToolTip(QString(tr("New Command (%1)")).arg(settings->getNewCommandShortcut()));
    saveButton->setToolTip(QString(tr("Save Command (%1)")).arg(settings->getSaveCommandShortcut()));
    allCommandsButton->setToolTip(QString(tr("All Commands (%1)")).arg(settings->getOpenCommandsShortcut()));
    
    QString startExecuteShortcut = settings->getStartExecuteShortcut();
    startButton->setToolTip(QString(tr("Validate Command (%1)")).arg(startExecuteShortcut));
    executeButton->setToolTip(QString(tr("Run Command (%1)")).arg(startExecuteShortcut));
}

void MainWindow::onStartExecuteShortcut() {
    if (startButton->isVisible()) {
        // If Start button is visible, click it first
        onStartClicked();
        
        // Then automatically execute after a short delay
        QTimer::singleShot(100, [this]() {
            if (executeButton->isVisible()) {
                onExecuteClicked();
            }
        });
    } else if (executeButton->isVisible()) {
        // If only Execute button is visible, just execute
        onExecuteClicked();
    }
}

