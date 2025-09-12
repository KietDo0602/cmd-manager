#ifndef TERMINALDIALOG_H
#define TERMINALDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QTextEdit>

class TerminalDialog : public QDialog {
    Q_OBJECT
public:
    explicit TerminalDialog(const QString &cmd, QWidget *parent = nullptr);

private:
    QProcess *process;
    QTextEdit *terminalOutput;
};

#endif // TERMINALDIALOG_H
