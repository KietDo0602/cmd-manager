#include "filerowwidget.h"
#include <QHBoxLayout>
#include <QFileDialog>

FileRowWidget::FileRowWidget(const QString &placeholder, QWidget *parent)
    : QWidget(parent), placeholderFile(placeholder) {

    fileLabel = new QLabel(">> " + placeholder, this);
    roleCombo = new QComboBox(this);
    roleCombo->addItems({tr("Input"), tr("Output")});

    actionButton = new QPushButton(tr("Choose File"), this);
    clearButton = new QPushButton(tr("Clear File"), this);
    clearButton->hide();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(fileLabel);
    layout->addWidget(new QLabel("Type:"));
    layout->addWidget(roleCombo);
    layout->addWidget(actionButton);
    layout->addWidget(clearButton);
    layout->addStretch();

    setLayout(layout);

    connect(roleCombo, &QComboBox::currentTextChanged, this, [this](const QString &role) {
        actionButton->setText(role == tr("Input") ? tr("Choose File") : tr("Create File"));
    });

    connect(actionButton, &QPushButton::clicked, this, [this]() {
        if (roleCombo->currentText() == tr("Input"))
            onChooseFile();
        else
            onCreateFile();
    });

    connect(clearButton, &QPushButton::clicked, this, &FileRowWidget::onClearFile);
}

void FileRowWidget::onChooseFile() {
    QString file = QFileDialog::getOpenFileName(this, tr("Choose Input File"));
    if (!file.isEmpty()) {
        selectedFile = file;
        updateUI();
        emit fileChanged();
    }
}

void FileRowWidget::onCreateFile() {
    QString file = QFileDialog::getSaveFileName(this, tr("Create Output File"));
    if (!file.isEmpty()) {
        selectedFile = file;
        updateUI();
        emit fileChanged();
    }
}

void FileRowWidget::onClearFile() {
    selectedFile.clear();
    updateUI();
    emit fileChanged();
}

void FileRowWidget::updateUI() {
    if (!selectedFile.isEmpty()) {
        actionButton->hide();
        clearButton->show();
        fileLabel->setText(">> " + placeholderFile + " → " + selectedFile);
        fileLabel->setWordWrap(true);
    } else {
        actionButton->show();
        clearButton->hide();
        fileLabel->setText(">> " + placeholderFile);
        fileLabel->setWordWrap(true);
    }
}
