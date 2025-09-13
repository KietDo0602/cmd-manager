#include "mainwindow.h"

#include <QGroupBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QTextStream>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), process(new QProcess(this)) {

    setWindowTitle("CMD Manager");

    QWidget *central = new QWidget(this);
    mainLayout = new QVBoxLayout(central);

    // Directory chooser
    chooseDirButton = new QPushButton("Current Directory");
    connect(chooseDirButton, &QPushButton::clicked, this, &MainWindow::onChooseDirClicked);
    mainLayout->addWidget(chooseDirButton, 0, Qt::AlignCenter);

    // Command editor
    commandEdit = new QTextEdit();
    commandEdit->setPlaceholderText("Enter command here...");
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
    clearButton->setFixedWidth(120);
    clearButton->hide();

    // Layout for Execute and Clear side by side
    QHBoxLayout *execLayout = new QHBoxLayout;
    execLayout->addWidget(executeButton);
    execLayout->addWidget(clearButton);
    execLayout->addStretch();

    mainLayout->addLayout(execLayout);

    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    setCentralWidget(central);
    resize(900, 600);
}

MainWindow::~MainWindow() {}

void MainWindow::onStartClicked() {
    // Clear old rows if they exist
    qDeleteAll(fileRows);
    fileRows.clear();

    // Get the command text
    QString cmdText = commandEdit->toPlainText();
    if (cmdText.isEmpty()) {
        QMessageBox::warning(this, "No Command", "Please enter a command first.");
        return;
    }

    // Detect placeholder files in the command
    // Example: input.mp4, data.csv, config.json, output.txt
    QRegularExpression fileRegex(R"(([\w,\s-]+\.[A-Za-z0-9]+))");
    QRegularExpressionMatchIterator it = fileRegex.globalMatch(cmdText);

    QStringList detectedFiles;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString fileName = match.captured(1);
        if (!detectedFiles.contains(fileName)) {
            detectedFiles.append(fileName);
        }
    }

    // Build the layout for file rows
    QVBoxLayout *filesLayout = new QVBoxLayout;

    for (const QString &file : detectedFiles) {
        FileRowWidget *row = new FileRowWidget(file, this);
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

    // Add this group to the main layout (if not already added)
    if (!filesGroupAdded) {
        mainLayout->addWidget(filesGroup);
        filesGroupAdded = true;
    }

    // Store the command template
    commandTemplate = cmdText;
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

void MainWindow::onExecuteClicked() {
    QString finalCmd = commandTemplate;
    for (FileRowWidget *row : fileRows) {
        if (!row->getSelectedFile().isEmpty()) {
            finalCmd.replace(row->getPlaceholder(), "\"" + row->getSelectedFile() + "\"");
        }
    }


    // Show terminal output in a dialog
    QDialog *terminal = new QDialog(this);
    terminal->setWindowTitle("CMD Manager");
    QVBoxLayout *layout = new QVBoxLayout(terminal);

    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);
    layout->addWidget(output);

    QPushButton *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, terminal, &QDialog::close);

    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, [=]() {
        output->append(process->readAllStandardOutput());
    });
    connect(process, &QProcess::readyReadStandardError, [=]() {
        output->append(process->readAllStandardError());
    });

    process->setWorkingDirectory(currentDir.isEmpty() ? "/" : currentDir);
    process->start("bash", {"-c", finalCmd});

    terminal->resize(800, 500);
    terminal->exec();
}

void MainWindow::onChooseDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choose Directory");
    if (!dir.isEmpty()) currentDir = dir;
}

void MainWindow::parseCommandForFiles(const QString &cmd) {
    placeholders.clear();
    QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);

    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i].contains('.')) {
            bool isOutput = false;
            if (i > 0 && parts[i-1] != "-i") { // simple rule: if previous isn't -i, mark as output
                isOutput = true;
            }
            placeholders.append({parts[i], "", isOutput});
        }
    }
}

QString MainWindow::buildFinalCommand() const {
    QString cmd = commandEdit->toPlainText();
    for (const auto &ph : placeholders) {
        // Use chosen path if set, otherwise original filename
        QString replacement = ph.chosenPath.isEmpty() ? ph.original : ph.chosenPath;
        QString quoted = "\"" + replacement + "\"";  // Quote for safety
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

void MainWindow::onClearClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Commands",
                                  "Are you sure you want to clear the command and reset everything?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // Clear command text
        commandEdit->clear();

        // Delete all file rows
        qDeleteAll(fileRows);
        fileRows.clear();

        // Remove and delete filesGroup if it exists
        if (filesGroup) {
            layout()->removeWidget(filesGroup);
            delete filesGroup;
            filesGroup = nullptr;
        }

        // Reset buttons
        executeButton->hide();
        clearButton->hide();
        startButton->show();
        filesGroupAdded = false;
    }
}
