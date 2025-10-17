#include "filerowwidget.h"
#include <QHBoxLayout>
#include <QFileDialog>

FileRowWidget::FileRowWidget(const QString &placeholder, QWidget *parent)
    : QWidget(parent), placeholderFile(placeholder) {

    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);

    // Set focus policy so widget can receive events
    setFocusPolicy(Qt::StrongFocus);

    fileLabel = new QLabel(">> " + placeholder, this);

    roleCombo = new QComboBox(this);

    // Store untranslated values as item data
    roleCombo->addItem(tr("Input"), "input");
    roleCombo->addItem(tr("Output"), "output");

    actionButton = new QPushButton(tr("Choose File"), this);

    clearButton = new QPushButton(tr("Clear File"), this);
    clearButton->hide();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(fileLabel);
    layout->addWidget(new QLabel(tr("Type:")));
    layout->addWidget(roleCombo);
    layout->addWidget(actionButton);
    layout->addWidget(clearButton);
    layout->addStretch();

    setLayout(layout);

    connect(roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        QString role = roleCombo->itemData(index).toString();
        onRoleChanged(role);
    });

    connect(roleCombo, &QComboBox::currentTextChanged, this, [this](const QString &role) {
        actionButton->setText(role == "input" ? tr("Choose File") : tr("Create File"));
    });

    connect(actionButton, &QPushButton::clicked, this, [this]() {
        QString role = roleCombo->currentData().toString();
        if (role == "input")
            onChooseFile();
        else
            onCreateFile();
    });

    connect(clearButton, &QPushButton::clicked, this, &FileRowWidget::onClearFile);

    // Enable drag and drop
    setAcceptDrops(true);
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

void FileRowWidget::onRoleChanged(const QString &role) {
    actionButton->setText(role == "input" ? tr("Choose File") : tr("Create File"));
    
    // Enable/disable drag and drop based on role
    if (role == "input") {
        setAcceptDrops(true);
        setToolTip("Drag and drop files here or click 'Choose File'");
    } else {
        setAcceptDrops(false);
        setToolTip("");
    }
}


void FileRowWidget::dropEvent(QDropEvent *event) {
    // Only accept drops for Input type
    if (roleCombo->currentData().toString() != "input") {
        event->ignore();
        return;
    }

    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);

                if (fileInfo.exists() && fileInfo.isFile()) {
                    selectedFile = filePath;
                    updateUI();
                    emit fileChanged();
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }

    event->ignore();
}

void FileRowWidget::dragEnterEvent(QDragEnterEvent *event) {
    // Only accept drops for Input type
    if (roleCombo->currentData().toString() != "input") {
        event->ignore();
        return;
    }

    // Accept if drag contains at least one local file
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                event->acceptProposedAction();
                return;
            }
        }
    }

    event->ignore();
}



void FileRowWidget::setFileFromPath(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    
    // Check if file exists
    if (!fileInfo.exists()) {
        return;
    }
    
    // Check if it's a file (not a directory)
    if (!fileInfo.isFile()) {
        return;
    }
    
    selectedFile = filePath;
    updateUI();
    emit fileChanged();
}

void FileRowWidget::setRole(const QString &role) {
    if (role == "input") {
        roleCombo->setCurrentIndex(0);
    } else {
        roleCombo->setCurrentIndex(1);
    }
}

void FileRowWidget::forceStyleUpdate() {
    // Force style update for this widget and all children
    style()->unpolish(this);
    style()->polish(this);
    update();
    
    QList<QWidget*> children = findChildren<QWidget*>();
    for (QWidget* child : children) {
        child->style()->unpolish(child);
        child->style()->polish(child);
        child->update();
    }
}

