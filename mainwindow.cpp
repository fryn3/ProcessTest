#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QElapsedTimer>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();
    ui->tv->setModel(&_model);
    ui->tv->setSelectionBehavior(QAbstractItemView::SelectRows);
}

MainWindow::~MainWindow()
{
    if (_thread) {
        _thread->quit();
        _thread->wait();
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.beginGroup(SETTINGS);
    {
        settings.setValue(GEOMETRY, saveGeometry());
        settings.setValue(WIN_STATE, saveState());
        settings.setValue(SELECT_FILE, ui->leSelectFile->text());
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(SETTINGS);
    {
        restoreGeometry(settings.value(GEOMETRY).toByteArray());
        restoreState(settings.value(WIN_STATE).toByteArray());
        ui->leSelectFile->setText(settings.value(SELECT_FILE).toString());
        if (!ui->leSelectFile->text().isEmpty()) {
            ui->btnStart->setEnabled(true);
        }
    }
}

void MainWindow::on_btnSelectFile_clicked()
{
    QString strProg = QFileDialog::getOpenFileName(this, "Select program", QFileInfo(ui->leSelectFile->text()).path(), "*.exe *.py");
    if (strProg.isEmpty()) { return; }
    ui->leSelectFile->setText(strProg);
    ui->btnStart->setEnabled(true);
}

void MainWindow::on_btnAddTest_clicked()
{
    if (ui->teIn->toPlainText().isEmpty() || ui->teOut->toPlainText().isEmpty()) {
        ui->statusBar->showMessage("Full all fields", 5000);
        return;
    }
    _model.add(ui->teIn->toPlainText().toLatin1(), ui->teOut->toPlainText().toLatin1());
    ui->tv->resizeRowsToContents();
    ui->tv->resizeColumnsToContents();
}

void MainWindow::on_btnRemoveTest_clicked()
{
    QItemSelectionModel *select = ui->tv->selectionModel();
    if (!select->hasSelection()) {
        ui->statusBar->showMessage("Not selected row", 5000);
        return;
    }
    auto sR = select->selectedRows();
    // Для удаления с последней строки
    std::reverse(sR.begin(), sR.end());
    for (const auto& indM: sR) {
        _model.remove(indM.row());
    }
    ui->tv->resizeColumnsToContents();
}

void MainWindow::on_btnStart_clicked()
{
    QList<QWidget*> lWidgets {
        ui->btnSelectFile, ui->btnAddTest, ui->btnRemoveTest, ui->btnStart,
    };
    for (auto & w: lWidgets) { w->setEnabled(false); }
    ui->btnStop->setEnabled(true);
    ui->teLog->clear();
    _processTest = new ProcessTest(ui->leSelectFile->text(), _model.getDataIO());
    connect(ui->btnStop, &QPushButton::clicked, _processTest, &ProcessTest::stop);
    connect(_processTest, &ProcessTest::progress, ui->progressBar, &QProgressBar::setValue);
    _thread = new QThread(this);
    _processTest->moveToThread(_thread);
    connect(_thread, &QThread::started, _processTest, &ProcessTest::start);
    connect(_thread, &QThread::finished, _processTest, &ProcessTest::deleteLater);
    connect(_processTest, &ProcessTest::finished, this, &MainWindow::testsFinished);
    connect(_processTest, &ProcessTest::emitMsg, this, [this] (QString msg) {
        ui->teLog->append(msg);
    });
    connect(_processTest, &ProcessTest::finished, this, [this] {
        _thread->quit();
        _thread->wait();
        _thread->deleteLater();
        _thread = nullptr;
    });
    _thread->start();
}

void MainWindow::testsFinished(QList<InOutModel::Test_t> data)
{
    _model.setData(data);
    ui->tv->resizeRowsToContents();
    ui->tv->resizeColumnsToContents();
    QList<QWidget*> lWidgets {
        ui->btnSelectFile, ui->btnAddTest, ui->btnRemoveTest, ui->btnStart,
    };
    for (auto & w: lWidgets) { w->setEnabled(true); }
    ui->btnStop->setEnabled(false);
    if (ui->teLog->hasFocus()) {
        ui->btnStart->setFocus();
    }
}
