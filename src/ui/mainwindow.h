#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "filerowwidget.h"
#include "settingsdialog.h"
#include "settingsmanager.h"
#include "utils.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QShortcut>
#include <QSvgRenderer>
#include <QTimer>
#include <QKeySequence>
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
#include <QLineEdit>
#include <QComboBox>
#include <QSet>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QDateTime>
#include <algorithm>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QIODevice>
#include <QPainter>

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
        void playNotificationSound();

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
        void onSettingsClicked();
        void applyThemeAndFont();

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        QSystemTrayIcon *trayIcon;
        QMenu *trayMenu;
        void setupSystemTray();
        bool hasAnyChanges;

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
        QPushButton *settingsButton;
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
        bool containsDangerousCommands(const QString &command) const;
        QStringList getDangerousCommands(const QString &command) const;
        bool showCommandPreview(const QString &command);


        void clearDynamicButtons();
        void clearCommandsInternal(); // Clear without confirmation dialog
        void updateDirectoryButton(); // Update directory button text
        void updateWindowTitle();
        void storeOriginalState();
        void checkForChanges();
        bool hasChanges() const;
        int showUnsavedChangesDialog();

        // Keyboard Shortcuts
        QShortcut *newCommandShortcut;
        QShortcut *saveCommandShortcut;
        QShortcut *openCommandsShortcut;
        QShortcut *startExecuteShortcut;
        
        void setupKeyboardShortcuts();
        void updateKeyboardShortcuts();
        void onStartExecuteShortcut();
};

class CommandsMenuDialog : public QDialog
{
    Q_OBJECT
    public:
        CommandsMenuDialog(QWidget *parent = nullptr);
        void updateCommandsList();

    private slots:
        void onCommandSelected();
        void onDeleteCommand();
        void onNewClicked();
        void onRunCommand();
        void onSearchTextChanged(const QString &text);
        void onSortChanged(int index);
        void onPinToggle();

    private:
        QListWidget *commandsList;
        QPushButton *newButton;
        QPushButton *openButton;
        QPushButton *runButton;
        QPushButton *deleteButton;
        QPushButton *closeButton;
        QPushButton *pinButton;
        QLineEdit *searchBox;
        QComboBox *sortCombo;

        enum SortMode {
          SortByName,
          SortByRecentlyOpened
        };

        SortMode currentSortMode;
        QString currentSearchText;
        
        void refreshCommandsList();
        QJsonObject loadCommandsFromJson();
        void saveCommandsToJson(const QJsonObject &commands);
        QString getConfigFilePath();
        void executeCommand(const QJsonObject &commands);
        void togglePin(const QString &commandName);
};

#endif // MAINWINDOW_H
