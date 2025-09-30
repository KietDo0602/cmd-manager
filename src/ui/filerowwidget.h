#ifndef FILEROWWIDGET_H
#define FILEROWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileInfo>
#include <QScreen>
#include <QGroupBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QTextStream>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QSize>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCloseEvent>

class FileRowWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit FileRowWidget(const QString &placeholder, QWidget *parent = nullptr);
        
        QString getPlaceholder() const { return placeholderFile; }
        QString getSelectedFile() const { return selectedFile; }
        QString getRole() const { return roleCombo->currentText(); }
        
        void setSelectedFile(const QString &file) { 
            selectedFile = file; 
            updateUI(); 
        }
        void setRole(const QString &role) { 
            roleCombo->setCurrentText(role); 
        }

    signals:
        void fileChanged();

    private slots:
        void onChooseFile();
        void onCreateFile();
        void onClearFile();

    private:
        void updateUI();
        
        QString placeholderFile;
        QString selectedFile;
        QLabel *fileLabel;
        QComboBox *roleCombo;
        QPushButton *actionButton;
        QPushButton *clearButton;
};

#endif // FILEROWWIDGET_H
