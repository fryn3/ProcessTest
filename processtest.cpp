#include "processtest.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>

ProcessTest::ProcessTest(QString nameProcess, QList<InOutModel::Test_t> data, QObject *parent)
    : QObject(parent), _nameProcess(nameProcess), _data(data)
{
    if (QMetaType::type("QList<InOutModel::Test_t>") == QMetaType::UnknownType) {
        qRegisterMetaType<QList<InOutModel::Test_t>>("QList<InOutModel::Test_t>");
    }
}

void ProcessTest::start() {
    _stop = false;
    using stIO = InOutModel::StatusIO;
    QFileInfo fi(_nameProcess);
    if (!fi.exists()) {
        emit emitMsg("File don't found");
        emit finished(_data);
        return;
    }
    for (auto i = 0; i < _data.size(); ++i) {
        auto &d = _data[i];
        d.status = stIO::Bad;
        QProcess p;
        QElapsedTimer t;
        if (fi.suffix() == "exe") {
            p.start(fi.absoluteFilePath());
        } else if (fi.suffix() == "py") {
            p.start("py.exe", QStringList() << fi.absoluteFilePath());
        } else {
            qDebug() << "errFormat:" << fi.suffix();
            return;
        }

        t.start();
        bool isDone;
        qint64 elapsed;
        QElapsedTimer wait;
        isDone = false; elapsed = 0;
        wait.restart();
        while (!_stop && elapsed < waitProcess) {
            // "/ 3." - 3 части ожидания
            emit progress(int((double(i) + elapsed / 3. / waitProcess) * 1000 /_data.size()));
            if (p.waitForStarted(470)) { isDone = true; break; }
            elapsed = wait.elapsed();
        }
        if (_stop) {
            p.close();
            emit emitMsg(QString("%1: %2").arg(i + 1).arg("signal stop"));
            emit finished(_data);
            return;
        }
        if (!isDone) {
            p.close();
            emit emitMsg(QString("%1: %2").arg(i + 1).arg("Can't started the program"));
            continue;
        }
        if (!d.in.isEmpty()) {
            p.write(d.in);
            p.write("\r\n");
            isDone = false; elapsed = 0;
            wait.restart();
            while (!_stop && elapsed < waitProcess) {
                emit progress(int((double(i) + 1./3 + elapsed / 3. / waitProcess) * 1000 /_data.size()));
                if (p.waitForBytesWritten(470)) { isDone = true; break; }
                elapsed = wait.elapsed();
            }
            if (_stop) {
                p.close();
                emit emitMsg(QString("%1: %2").arg(i + 1).arg("signal stop"));
                emit finished(_data);
                return;
            }
            if (!isDone) {
                p.close();
                emit emitMsg(QString("%1: %2").arg(i + 1).arg("Can't write to the program"));
                continue;
            }
        }
        if (!d.out.isEmpty()) {
            isDone = false; elapsed = 0;
            wait.restart();
            while (!_stop && elapsed < waitProcess) {
                emit progress(int((double(i) + 2./3 + elapsed / 3. / waitProcess) * 1000 /_data.size()));
                if (p.waitForReadyRead(470)) { isDone = true; break; }
                elapsed = wait.elapsed();
            }
            if (_stop) {
                p.close();
                emit emitMsg(QString("%1: %2").arg(i + 1).arg("signal stop"));
                emit finished(_data);
                return;
            }
            if (!isDone) {
                p.close();
                emit emitMsg(QString("%1: %2").arg(i + 1).arg("Can't read from the program"));
                continue;
            }
        }
        d.realOut = p.readAll();
        d.elapsed = t.elapsed();
        auto err = p.readAllStandardError();
        if (!err.isEmpty()) {
            p.close();
            emit emitMsg(QString("%1: %2").arg(i + 1).arg(QString(err)));
            continue;
        }
        QTextStream sOut(d.out), sOutReal(d.realOut);
        bool isGood = true;
        while (!sOut.atEnd() && !sOutReal.atEnd() && isGood) {
            auto str1 = sOut.readLine();
            while (str1.isEmpty()) {
                str1 = sOut.readLine();
            }
            auto str2 = sOutReal.readLine();
            while (str2.isEmpty()) {
                str2 = sOut.readLine();
            }
            if (str1 != str2) {
                isGood = false;
            }
        }
        while (!sOut.atEnd() && isGood) {
            if (!sOut.readLine().isEmpty()) {
                isGood = false;
            }
        }
        while (!sOutReal.atEnd() && isGood) {
            if (!sOutReal.readLine().isEmpty()) {
                isGood = false;
            }
        }
        if (isGood) {
            d.status = stIO::Good;
        }
        p.close();
    }
    emit progress(1000);
    emit finished(_data);
}

void ProcessTest::stop()
{
    _stop = true;
}

//void ProcessTest::goToFinish(int ind, QProcess &p, QString msg)
//{
//    p.close();
//    emit emitMsg(QString("%1: %2").arg(ind + 1).arg(msg));
//    emit finished(_data);
//}
