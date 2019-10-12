#ifndef PROCESSTEST_H
#define PROCESSTEST_H

#include <QObject>
#include <QProcess>

#include "inoutmodel.h"

class ProcessTest : public QObject
{
    Q_OBJECT
public:
    explicit ProcessTest(QString nameProcess, QList<InOutModel::Test_t> data, QObject *parent = nullptr);

signals:
    void emitMsg(QString msg);
    void finished(QList<InOutModel::Test_t>);
    void progress(int p);
public slots:
    void start();
    void stop();
protected:
    QString _nameProcess;
    QList<InOutModel::Test_t> _data;
    qint64 waitProcess = 5000;
    bool _stop = false;
};

#endif // PROCESSTEST_H
