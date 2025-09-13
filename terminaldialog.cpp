#include "terminaldialog.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QTextCursor>

TerminalDialog::TerminalDialog(const QString &cmd, QWidget *parent)
    : QDialog(parent), process(new QProcess(this)) {

    setWindowTitle("CMD Manager");
    resize(800, 600);

    auto *layout = new QVBoxLayout(this);
    terminalOutput = new QTextEdit(this);
    terminalOutput->setReadOnly(true);
    layout->addWidget(terminalOutput);

    process->setProcessChannelMode(QProcess::MergedChannels);

    // Buffer to store process output
    QString *buffer = new QString;

    connect(process, &QProcess::readyReadStandardOutput, this, [this, buffer]() {
        buffer->append(QString::fromLocal8Bit(process->readAllStandardOutput()));
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, buffer]() {
        buffer->append(QString::fromLocal8Bit(process->readAllStandardError()));
    });

    // Timer to flush buffer to QTextEdit periodically
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, buffer]() {
        if (!buffer->isEmpty()) {
            terminalOutput->moveCursor(QTextCursor::End);
            terminalOutput->insertPlainText(*buffer);
            *buffer = "";
            // Limit max lines
            if (terminalOutput->document()->blockCount() > 10000) {
                terminalOutput->clear();
                terminalOutput->insertPlainText("[Log truncated]\n");
            }
        }
    });
    timer->start(100);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, buffer](int exitCode, QProcess::ExitStatus) {
        buffer->append(QString("\nProcess finished with code %1\n").arg(exitCode));
    });

    process->start("bash", {"-c", cmd});
}
