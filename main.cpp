#include <QApplication>
#include "mainwindow.h"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QProcess>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QListWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCloseEvent>
#include <QGroupBox>
#include <QDebug>
#include <regex>


class TerminalWidget : public QWidget {
    Q_OBJECT
    public:
        TerminalWidget(QWidget* parent = nullptr) : QWidget(parent) {
            QVBoxLayout* l = new QVBoxLayout(this);
            output = new QPlainTextEdit(this);
            output->setReadOnly(true);
            l->addWidget(output);
            QHBoxLayout* hb = new QHBoxLayout();
            btnClose = new QPushButton("Close", this);
            btnKill = new QPushButton("Kill", this);
            hb->addStretch();
            hb->addWidget(btnKill);
            hb->addWidget(btnClose);
            l->addLayout(hb);
            process = new QProcess(this);
            connect(process, &QProcess::readyReadStandardOutput, this, &TerminalWidget::readStdOut);
            connect(process, &QProcess::readyReadStandardError, this, &TerminalWidget::readStdErr);
            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &TerminalWidget::finished);
            connect(btnClose, &QPushButton::clicked, this, &TerminalWidget::onCloseClicked);
            connect(btnKill, &QPushButton::clicked, this, &TerminalWidget::onKillClicked);
        }

        void start(const QString& cmd, const QStringList& args, const QString& workdir) {
            output->appendPlainText(QString("Starting in: %1\nCommand: %2 %3\n---\n").arg(workdir).arg(cmd).arg(args.join(' ')));
            process->setWorkingDirectory(workdir);
            process->start(cmd, args);
            if (!process->waitForStarted(3000)) {
                output->appendPlainText("Failed to start process.\n");
            }
        }

        bool isRunning() const { return process->state() != QProcess::NotRunning; }

    signals:
        void requestClose(TerminalWidget*);

    private slots:
        void readStdOut() { output->appendPlainText(QString::fromLocal8Bit(process->readAllStandardOutput())); }
        void readStdErr() { output->appendPlainText(QString::fromLocal8Bit(process->readAllStandardError())); }
        void finished(int code, QProcess::ExitStatus status) {
            output->appendPlainText(QString("\nProcess finished with code %1, status %2\n").arg(code).arg((int)status));
        }
        void onCloseClicked() {
            if (isRunning()) {
                auto res = QMessageBox::question(this, "Process running", "Process is still running. Close and kill it?", QMessageBox::Yes|QMessageBox::No);
                if (res == QMessageBox::No) return;
                process->kill();
                process->waitForFinished(2000);
            }
            emit requestClose(this);
        }
        void onKillClicked() {
            if (isRunning()) {
                process->kill();
                output->appendPlainText("\nProcess killed by user.\n");
            }
        }

    private:
        QPlainTextEdit* output;
        QProcess* process;
        QPushButton* btnClose;
        QPushButton* btnKill;
};


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties for taskbar grouping
    app.setApplicationName("CMD Manager");
    app.setApplicationDisplayName("CMD Manager");
    app.setOrganizationName("Kiet Do");
    app.setApplicationVersion("1.0");

    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
