#ifndef FILEROWWIDGET_H
#define FILEROWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QString>

class FileRowWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileRowWidget(const QString &placeholder, QWidget *parent = nullptr);

    QString getPlaceholder() const { return placeholderFile; }
    QString getSelectedFile() const { return selectedFile; }
    QString getRole() const { return roleCombo->currentText(); }

signals:
    void fileChanged();

private slots:
    void onChooseFile();
    void onCreateFile();
    void onClearFile();

private:
    QString placeholderFile;
    QString selectedFile;

    QLabel *fileLabel;
    QComboBox *roleCombo;
    QPushButton *actionButton;
    QPushButton *clearButton;

    void updateUI();
};

#endif // FILEROWWIDGET_H
