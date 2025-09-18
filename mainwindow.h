#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "filerowwidget.h"

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>
#include <QListWidget>
#include <QDialog>
#include <QLabel>
#include <QSet>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>

struct FilePlaceholder {
    QString original;
    QString chosenPath;
    bool isOutput;
};

class CommandsMenuDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Make loadCommand public so CommandsMenuDialog can access it
    void loadCommand(const QJsonObject &commandData);
    
    // Static set to track opened commands
    static QSet<QString> openedCommands;
    
    // Get current command name (for tracking)
    QString getCurrentCommandName() const { return currentCommandName; }
    
    // Set current command name (for tracking)
    void setCurrentCommandName(const QString &name) { 
        currentCommandName = name; 
        hasUnsavedChanges = false;
        storeOriginalState();
        updateWindowTitle();
    }

private slots:
    void onStartClicked();
    void onExecuteClicked();
    void onChooseDirClicked();
    void onClearClicked();
    void onNewClicked();
    void onSaveClicked();
    void onAllCommandsClicked();
    void onInputButtonClicked();
    void onOutputButtonClicked();
    void onCommandTextChanged();
    void onFileChanged();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // UI Components
    QVBoxLayout *mainLayout;
    QMenu *fileMenu;
    QAction *settingsAction;
    QPushButton *newButton;
    QPushButton *allCommandsButton;
    QPushButton *saveButton;
    QPushButton *chooseDirButton;
    QTextEdit *commandEdit;
    QPushButton *startButton;
    QPushButton *executeButton;
    QPushButton *clearButton;
    QWidget *buttonContainer;
    QVBoxLayout *dynamicButtonLayout;
    QGroupBox *filesGroup = nullptr;
    
    // File management
    QList<FileRowWidget*> fileRows;
    bool filesGroupAdded = false;
    
    // Command processing
    QString commandTemplate;
    QString currentDir;
    QString currentCommandName; // Track current command name
    QProcess *process;
    QList<FilePlaceholder> placeholders;
    QMap<QPushButton*, int> buttonIndexMap;
    
    // Change tracking
    bool hasUnsavedChanges = false;
    QString originalCommandText;
    QString originalDirectory;
    QJsonArray originalFilesData;
    
    // Persistence methods
    void saveCommandToJson(const QString &name, const QString &command, const QList<FileRowWidget*> &fileRows);
    QJsonObject loadCommandsFromJson();
    QString getConfigFilePath();
    void parseCommandForFiles(const QString &cmd);
    QString buildFinalCommand() const;
    void clearDynamicButtons();
    void clearCommandsInternal(); // Clear without confirmation dialog
    void updateDirectoryButton(); // Update directory button text
    void updateWindowTitle();
    void storeOriginalState();
    void checkForChanges();
    bool hasChanges() const;
    int showUnsavedChangesDialog();
};

class CommandsMenuDialog : public QDialog
{
    Q_OBJECT

public:
    CommandsMenuDialog(QWidget *parent = nullptr);

private slots:
    void onCommandSelected();
    void onDeleteCommand();
    void onNewClicked();

private:
    QListWidget *commandsList;
    QPushButton *newButton;
    QPushButton *openButton;
    QPushButton *deleteButton;
    QPushButton *closeButton;
    
    void refreshCommandsList();
    QJsonObject loadCommandsFromJson();
    void saveCommandsToJson(const QJsonObject &commands);
    QString getConfigFilePath();
};

#endif // MAINWINDOW_H
