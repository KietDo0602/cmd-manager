#include "mainwindow.h"

#include <QScreen>
#include <QGroupBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QTextStream>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCloseEvent>

// Initialize static member
QSet<QString> MainWindow::openedCommands;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), process(new QProcess(this)) {

    setWindowTitle("CMD Manager");

    QWidget *central = new QWidget(this);
    mainLayout = new QVBoxLayout(central);

    // Top buttons row (New, All Commands, Save)
    QHBoxLayout *topButtonsLayout = new QHBoxLayout();
    
    newButton = new QPushButton("New");
    allCommandsButton = new QPushButton("All Commands");
    saveButton = new QPushButton("Save");
    
    newButton->setFixedWidth(100);
    allCommandsButton->setFixedWidth(120);
    saveButton->setFixedWidth(100);
    
    // Add stretch before and after the buttons to center them
    topButtonsLayout->addStretch();
    topButtonsLayout->addWidget(newButton);
    topButtonsLayout->addWidget(allCommandsButton);
    topButtonsLayout->addWidget(saveButton);
    topButtonsLayout->addStretch(); 

    connect(newButton, &QPushButton::clicked, this, &MainWindow::onNewClicked);
    connect(allCommandsButton, &QPushButton::clicked, this, &MainWindow::onAllCommandsClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    
    mainLayout->addLayout(topButtonsLayout);

    // Directory chooser
    chooseDirButton = new QPushButton("Current Directory: (Not Set)");
    connect(chooseDirButton, &QPushButton::clicked, this, &MainWindow::onChooseDirClicked);
    mainLayout->addWidget(chooseDirButton, 0, Qt::AlignCenter);

    // Command editor
    commandEdit = new QTextEdit();
    commandEdit->setPlaceholderText("Enter command here...");
    connect(commandEdit, &QTextEdit::textChanged, this, &MainWindow::onCommandTextChanged);
    mainLayout->addWidget(commandEdit);

    // Start button
    startButton = new QPushButton("Start");
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    mainLayout->addWidget(startButton);

    // Dynamic button container
    buttonContainer = new QWidget();
    dynamicButtonLayout = new QVBoxLayout(buttonContainer);
    mainLayout->addWidget(buttonContainer);

    // Execute and Clear button
    executeButton = new QPushButton("Execute");
    executeButton->hide();
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);

    clearButton = new QPushButton("Clear Commands");
    clearButton->setFixedWidth(140);
    clearButton->hide();

    // Layout for Execute and Clear side by side
    QHBoxLayout *execLayout = new QHBoxLayout;
    execLayout->addWidget(executeButton);
    execLayout->addWidget(clearButton);
    execLayout->addStretch();

    mainLayout->addLayout(execLayout);

    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    setCentralWidget(central);
    setStyleSheet(R"(
      QMainWindow {
          background-color: #2B2B2B;
      }

      QWidget {
          background-color: #2B2B2B;
          color: #E0E0E0;
      }

      QToolBar {
          background: #323232;
          border: none;
          spacing: 6px;
      }
      QToolBar QToolButton {
          background: #3C3C3C;
          color: #E0E0E0;
          border-radius: 4px;
          padding: 6px 10px;
      }
      QToolBar QToolButton:hover {
          background: #505050;
      }

      QPushButton {
          background-color: #3C3C3C;
          border: 1px solid #555;
          border-radius: 4px;
          color: #E0E0E0;
          padding: 6px 12px;
      }
      QPushButton:hover {
          background-color: #505050;
          border: 1px solid #666;
      }
      QPushButton:pressed {
          background-color: #2E2E2E;
      }

      QLineEdit {
          background: #1E1E1E;
          border: 1px solid #555;
          border-radius: 4px;
          padding: 6px;
          color: #E0E0E0;
          selection-background-color: #0078D7;
      }

      QTextEdit {
          background: #1E1E1E;
          border: 1px solid #555;
          border-radius: 4px;
          color: #E0E0E0;
          padding: 6px;
      }

      QLabel {
          color: #E0E0E0;
          font-weight: 500;
      }

      QGroupBox {
          font-weight: bolder;
          color: #E0E0E0;
          font-weight: 500;
      }

      QMenuBar {
          background-color: #2B2B2B;
          color: #E0E0E0;
      }
      QMenuBar::item:selected {
          background: #505050;
      }

      QStatusBar {
          background: #2B2B2B;
          color: #AAAAAA;
      }
    )");

    resize(1280, 720);
}

MainWindow::~MainWindow() {
    // Remove this command from opened commands when window closes
    if (!currentCommandName.isEmpty()) {
        openedCommands.remove(currentCommandName);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (hasUnsavedChanges && !currentCommandName.isEmpty()) {
        int result = showUnsavedChangesDialog();
        
        switch (result) {
            case QMessageBox::Save: // Save and Close
                onSaveClicked();
                // Remove from opened commands after saving
                if (!currentCommandName.isEmpty()) {
                    openedCommands.remove(currentCommandName);
                }
                event->accept();
                break;
            case QMessageBox::Discard: // Don't Save
                // Remove from opened commands
                if (!currentCommandName.isEmpty()) {
                    openedCommands.remove(currentCommandName);
                }
                event->accept();
                break;
            case QMessageBox::Cancel: // Cancel
                event->ignore();
                return;
        }
    } else {
        // Remove from opened commands
        if (!currentCommandName.isEmpty()) {
            openedCommands.remove(currentCommandName);
        }
        event->accept();
    }
    
    QMainWindow::closeEvent(event);
}

int MainWindow::showUnsavedChangesDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Unsaved Changes");
    msgBox.setText(QString("You have unsaved changes in \"%1\". Do you want to save before closing?").arg(currentCommandName));
    msgBox.setIcon(QMessageBox::Warning);
    
    QPushButton *saveButton = msgBox.addButton("Save and Close", QMessageBox::AcceptRole);
    QPushButton *dontSaveButton = msgBox.addButton("Don't Save", QMessageBox::DestructiveRole);
    QPushButton *cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
    
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
        QMessageBox::warning(this, "No Command", "Please enter a command to save.");
        return;
    }
    
    QString name;
    bool isExistingCommand = !currentCommandName.isEmpty();
    
    if (isExistingCommand) {
        // Updating existing command - show confirmation
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Update Command",
            QString("Do you want to update the existing command \"%1\" with these changes?").arg(currentCommandName),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
        name = currentCommandName;
    } else {
        // Creating new command - ask for name
        bool ok;
        name = QInputDialog::getText(this, "Save Command", 
                                    "Enter a name for this command:", 
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
    
    QMessageBox::information(this, "Command Saved", 
                           QString("Command '%1' has been saved successfully.").arg(name));
}

void MainWindow::onAllCommandsClicked() {
    CommandsMenuDialog *dialog = new CommandsMenuDialog(nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::onCommandTextChanged() {
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
    QString title = "CMD Manager";
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
    updateWindowTitle();
}

void MainWindow::onClearClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Commands",
                                  "Are you sure you want to clear the command and reset everything?",
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
    
    // Store original state for change detection
    storeOriginalState();
    
    // Reset unsaved changes flag after loading
    hasUnsavedChanges = false;
    updateWindowTitle();
}

void MainWindow::updateDirectoryButton() {
    if (currentDir.isEmpty()) {
        chooseDirButton->setText("Current Directory: (Not Set)");
    } else {
        QFileInfo dirInfo(currentDir);
        QString displayText = dirInfo.fileName();
        if (displayText.isEmpty()) {
            displayText = currentDir;
        }
        
        if (displayText.length() > 30) {
            displayText = "..." + displayText.right(27);
        }
        
        chooseDirButton->setText(QString("Current Directory: %1").arg(displayText));
        chooseDirButton->setToolTip(currentDir);
    }
}

void MainWindow::onChooseDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choose Directory", currentDir);
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
        QMessageBox::warning(this, "No Command", "Please enter a command first.");
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
    filesGroup = new QGroupBox("Files");
    filesGroup->setLayout(filesLayout);

    // Remove the Start button, replace with Execute
    startButton->hide();
    executeButton->show();
    clearButton->show();

    // Add this group to the main layout
    mainLayout->addWidget(filesGroup);
    filesGroupAdded = true;

    // Store the command template (store the actual text, not simplified)
    commandTemplate = cmdText;
    
    // Check for changes after creating file rows
    checkForChanges();
}

void MainWindow::onExecuteClicked() {
    QString finalCmd = commandTemplate;
    
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
    terminal->setWindowTitle("CMD Manager - Command Execution");
    QVBoxLayout *layout = new QVBoxLayout(terminal);

    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);
    output->setFont(QFont("Courier", 10));
    layout->addWidget(output);

    QPushButton *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, terminal, &QDialog::close);

    // Display the command that will be executed
    output->append("Working Directory: " + (currentDir.isEmpty() ? QDir::homePath() : currentDir));
    output->append("Executing command:");
    output->append(finalCmd);
    output->append("----------------------------------------");

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
        output->append("----------------------------------------");
        output->append(QString("Process finished with exit code: %1").arg(exitCode));
        if (exitStatus == QProcess::CrashExit) {
            output->append("Process crashed!");
        }
        output->moveCursor(QTextCursor::End);
    });

    QString workingDir = currentDir.isEmpty() ? QDir::homePath() : currentDir;
    terminalProcess->setWorkingDirectory(workingDir);
    
    // Execute the command
    terminalProcess->start("/bin/bash", QStringList() << "-c" << finalCmd);
    
    if (!terminalProcess->waitForStarted(3000)) {
        output->append("Error: Failed to start the process!");
    }

    terminal->resize(800, 500);
    terminal->show(); // Use show() instead of exec() to make it non-modal
}

void MainWindow::onInputButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int idx = buttonIndexMap[btn];
    QString ext = QFileInfo(placeholders[idx].original).suffix();
    QString file = QFileDialog::getOpenFileName(this, "Select Input File", currentDir,
                                               QString("*.%1").arg(ext));
    if (!file.isEmpty()) {
        placeholders[idx].chosenPath = file;
        btn->setText(QString("Input %1: %2").arg(idx + 1).arg(QFileInfo(file).fileName()));
    }
}

void MainWindow::onOutputButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int idx = buttonIndexMap[btn];
    QString ext = QFileInfo(placeholders[idx].original).suffix();
    QString file = QFileDialog::getSaveFileName(this, "Select Output File", currentDir,
                                               QString("*.%1").arg(ext));
    if (!file.isEmpty()) {
        placeholders[idx].chosenPath = file;
        btn->setText(QString("Output %1: %2").arg(idx + 1).arg(QFileInfo(file).fileName()));
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
    QString cmd = commandEdit->toPlainText();
    for (const auto &ph : placeholders) {
        QString replacement = ph.chosenPath.isEmpty() ? ph.original : ph.chosenPath;
        QString quoted = "\"" + replacement + "\"";
        cmd.replace(ph.original, quoted);
    }
    return cmd;
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

CommandsMenuDialog::CommandsMenuDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Commands Menu");
    setModal(false); // Make it non-modal
    setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    resize(500, 400);
    
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
    
    commandsList = new QListWidget(this);
    layout->addWidget(new QLabel("Saved Commands:"));
    layout->addWidget(commandsList);
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    
    newButton = new QPushButton("New");
    openButton = new QPushButton("Open");
    deleteButton = new QPushButton("Delete");
    closeButton = new QPushButton("Close");
    
    buttonsLayout->addWidget(newButton);
    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(closeButton);
    
    layout->addLayout(buttonsLayout);
    
    connect(newButton, &QPushButton::clicked, this, &CommandsMenuDialog::onNewClicked);
    connect(openButton, &QPushButton::clicked, this, &CommandsMenuDialog::onCommandSelected);
    connect(deleteButton, &QPushButton::clicked, this, &CommandsMenuDialog::onDeleteCommand);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(commandsList, &QListWidget::itemDoubleClicked, this, &CommandsMenuDialog::onCommandSelected);
    
    refreshCommandsList();
}

void CommandsMenuDialog::onNewClicked() {
    MainWindow *newWindow = new MainWindow();
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}

void CommandsMenuDialog::refreshCommandsList() {
    commandsList->clear();
    QJsonObject commands = loadCommandsFromJson();
    
    for (auto it = commands.begin(); it != commands.end(); ++it) {
        commandsList->addItem(it.key());
    }
}

void CommandsMenuDialog::onCommandSelected() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "No Selection", "Please select a command to open.");
        return;
    }
    
    QString commandName = currentItem->text();
    
    if (MainWindow::openedCommands.contains(commandName)) {
        QMessageBox::warning(this, "Command Already Open", 
                            QString("The command '%1' is already opened in another window.").arg(commandName));
        return;
    }
    
    QJsonObject commands = loadCommandsFromJson();
    
    if (commands.contains(commandName)) {
        QJsonObject commandData = commands[commandName].toObject();
        
        MainWindow *newWindow = new MainWindow();
        newWindow->setAttribute(Qt::WA_DeleteOnClose);
        newWindow->setCurrentCommandName(commandName);
        MainWindow::openedCommands.insert(commandName);
        newWindow->loadCommand(commandData);
        newWindow->show();
    }
}

void CommandsMenuDialog::onDeleteCommand() {
    QListWidgetItem *currentItem = commandsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "No Selection", "Please select a command to delete.");
        return;
    }
    
    QString commandName = currentItem->text();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Command",
                                  QString("Are you sure you want to delete the command '%1'?").arg(commandName),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject commands = loadCommandsFromJson();
        commands.remove(commandName);
        saveCommandsToJson(commands);
        refreshCommandsList();
        QMessageBox::information(this, "Command Deleted", 
                                QString("Command '%1' has been deleted.").arg(commandName));
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

