#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QVector>
#include <QMap>

struct FilePlaceholder {
    QString original;   // Original filename placeholder
    QString chosenPath; // User-selected path
    bool isOutput;      // Whether this is an output file
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onExecuteClicked();
    void onInputButtonClicked();
    void onOutputButtonClicked();
    void onChooseDirClicked();

private:
    void parseCommandForFiles(const QString &cmd);
    QString buildFinalCommand() const;
    void clearDynamicButtons();

    QTextEdit *commandEdit;
    QPushButton *startButton;
    QPushButton *executeButton;
    QPushButton *chooseDirButton;

    QVBoxLayout *mainLayout;
    QWidget *buttonContainer;
    QVBoxLayout *dynamicButtonLayout;

    QVector<FilePlaceholder> placeholders;
    QMap<QPushButton*, int> buttonIndexMap;

    QString currentDir;
    QProcess *process;
};

#endif // MAINWINDOW_H

