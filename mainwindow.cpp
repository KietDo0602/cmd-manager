#include "mainwindow.h"
#include "terminaldialog.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), process(new QProcess(this)) {
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

    // Execute button
    executeButton = new QPushButton("Execute");
    executeButton->hide();
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);
    mainLayout->addWidget(executeButton);

    setCentralWidget(central);
    resize(900, 600);
}

MainWindow::~MainWindow() {}

void MainWindow::onStartClicked() {
    clearDynamicButtons();

    QString cmd = commandEdit->toPlainText().trimmed();
    if (cmd.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No command entered");
        return;
    }

    parseCommandForFiles(cmd);

    // Hide start, show execute
    startButton->hide();
    executeButton->show();

    // Create buttons for placeholders
    for (int i = 0; i < placeholders.size(); ++i) {
        QPushButton *btn = new QPushButton(
            placeholders[i].isOutput ? QString("Output %1").arg(i + 1) : QString("Input %1").arg(i + 1)
        );
        dynamicButtonLayout->addWidget(btn);
        buttonIndexMap[btn] = i;

        if (placeholders[i].isOutput)
            connect(btn, &QPushButton::clicked, this, &MainWindow::onOutputButtonClicked);
        else
            connect(btn, &QPushButton::clicked, this, &MainWindow::onInputButtonClicked);
    }
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
    QString finalCmd = buildFinalCommand();
    if (!finalCmd.isEmpty()) {
        TerminalDialog *dialog = new TerminalDialog(finalCmd, this);
        dialog->exec();
    }

    // Show terminal output in a dialog
    QDialog *terminal = new QDialog(this);
    terminal->setWindowTitle("Command Output");
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
        if (ph.chosenPath.isEmpty()) return "";

        // Quote the file path
        QString quoted = "\"" + ph.chosenPath + "\"";
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

