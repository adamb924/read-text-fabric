#ifndef DATABASEADAPTER_H
#define DATABASEADAPTER_H

#include <QtSql>
#include <QHash>
#include <QSet>


class DatabaseAdapter{
public:
    explicit DatabaseAdapter(const QString & filename);
    ~DatabaseAdapter();

    //! \brief Closes the database
    void close();

    void createTable(const QString & tableName, QSet<QString> columns , QHash<QString, QString> columnTypes = QHash<QString,QString>());
    void addTableColumn(const QString & table, const QString & column, const QString &columnType);

    void insertNodeData(unsigned int _id, const QString & columnDefinition, const QString &columnType, const QVariant &value);
    void insertEdgeData(const QString & table, unsigned int from, unsigned int to, const QVariant &value = QVariant());

    void setOtypeRanges(QHash<QString, QPair<unsigned int, unsigned int> > oTypeRanges);
    QString getOTypeFromNode(unsigned int node) const;

    void beginTransaction();
    void commitTransaction();

private:
    QString mFilename;
    QHash<QString,QSet<QString>> mTableColumns;
    QHash<QString,QPair<unsigned int,unsigned int>> mOTypeRanges;
};

#endif // DATABASEADAPTER_H
