#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "inoutmodel.h"
#include "processtest.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    const QString SETTINGS = "/settings";
    const QString GEOMETRY = "/geometry";
    const QString WIN_STATE = "/windowState";
    const QString SELECT_FILE = "/selectFile";
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void readSettings();
protected slots:
    void testsFinished(QList<InOutModel::Test_t> data);
    void on_btnSelectFile_clicked();
    void on_btnAddTest_clicked();
    void on_btnRemoveTest_clicked();
    void on_btnStart_clicked();

private:
    Ui::MainWindow *ui;
    InOutModel _model;
    ProcessTest *_processTest = nullptr;
    QThread *_thread = nullptr;
};

#endif // MAINWINDOW_H
