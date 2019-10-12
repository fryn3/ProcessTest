#include "inoutmodel.h"

#include <QColor>

QVariant InOutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) { return QVariant(); }
    if (orientation == Qt::Vertical) { return  section + 1; }
    switch (Colums(section)) {
    case Colums::In:
        return "IN";
    case Colums::Out:
        return "OUT";
    case Colums::Status:
        return "Status";
    case Colums::Elapsed:
        return "Elapsed (ms)";
    case Colums::RealOut:
        return "RealOut";
    default:
        return QVariant();
    }
}

QVariant InOutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::BackgroundColorRole)) { return QVariant(); }
    if (role == Qt::BackgroundColorRole) {
        if (Colums(index.column()) == Colums::Status) {
            switch (_data.at(index.row()).status) {
            case StatusIO::None:
                return QVariant();
            case StatusIO::Good:
                return QColor(Qt::darkGreen);
            case StatusIO::Bad:
                return QColor(Qt::darkRed);
            }
        } else {
            QVariant();
        }
    }
    switch (Colums(index.column())) {
    case Colums::In:
        return _data.at(index.row()).in;
    case Colums::Out:
        return _data.at(index.row()).out;
    case Colums::Status:
        switch (_data.at(index.row()).status) {
        case StatusIO::None:
            return "None";
        case StatusIO::Good:
            return "Good";
        case StatusIO::Bad:
            return "Bad";
        }
    case Colums::Elapsed:
        return  _data.at(index.row()).elapsed;
    case Colums::RealOut:
        return _data.at(index.row()).realOut;
    default:
        return QVariant();
    }
}

void InOutModel::add(QByteArray in, QByteArray out)
{
    beginInsertRows(QModelIndex(), _data.size(), _data.size());
    _data.append({in, out});
    endInsertRows();
}

void InOutModel::remove(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    _data.removeAt(index);
    endRemoveRows();
}

void InOutModel::setData(const QList<Test_t> &data)
{
    beginResetModel();
    _data = data;
    endResetModel();
}
