#ifndef INOUTMODEL_H
#define INOUTMODEL_H

#include <QAbstractListModel>

class InOutModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Colums { In, Out, Status, Elapsed, RealOut, CNT, };
    enum class StatusIO { None, Good, Bad, };
    struct Test_t {
        Test_t(QByteArray _in, QByteArray _out, StatusIO _st = StatusIO::None, qint64 _elapsed = 0)
            : in(_in), out(_out), status(_st), elapsed(_elapsed) { }
        QByteArray in, out, realOut;
        StatusIO status = StatusIO::None;
        qint64 elapsed = 0;
    };
    InOutModel() {  }
    int rowCount(const QModelIndex &) const override { return _data.size(); }
    int columnCount(const QModelIndex &) const override { return int(Colums::CNT); }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void add(QByteArray in, QByteArray out);
    void remove(int index);

    QList<Test_t> getDataIO() const { return _data; }

    void setData(const QList<Test_t> &data);

protected:
    QList<Test_t> _data;
};

#endif // INOUTMODEL_H
