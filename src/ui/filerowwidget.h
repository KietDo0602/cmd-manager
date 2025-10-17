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
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QString>

class FileRowWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit FileRowWidget(const QString &placeholder, QWidget *parent = nullptr);
        
        QString getPlaceholder() const { return placeholderFile; }

        QString getSelectedFile() const { return selectedFile; }
        void setSelectedFile(const QString &file) { 
            selectedFile = file; 
            updateUI(); 
        }

        QString getRole() const { return roleCombo->currentData().toString(); }
        void setRole(const QString &role);
        void forceStyleUpdate();
        
    signals:
        void fileChanged();

    protected:
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dropEvent(QDropEvent *event) override;

    private slots:
        void onChooseFile();
        void onCreateFile();
        void onClearFile();
        void onRoleChanged(const QString &role);
        void setFileFromPath(const QString &filePath);

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
