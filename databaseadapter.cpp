#include "databaseadapter.h"

DatabaseAdapter::DatabaseAdapter(const QString & filename) : mFilename(filename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", mFilename);
    db.setHostName("hostname");
    db.setDatabaseName(filename);
    if(!db.open())
    {
        qDebug() << "There was a problem in opening the database. The program said: " + db.lastError().databaseText() + " It is unlikely that you will solve this on your own. Rather you had better contact the developer.";
        return;
    }
    db.exec("PRAGMA TEMP_STORE = MEMORY;");
    db.exec("PRAGMA JOURNAL_MODE = OFF;");
    db.exec("PRAGMA SYNCHRONOUS = OFF;");
    db.exec("PRAGMA LOCKING_MODE = EXCLUSIVE;");
    db.exec("PRAGMA encoding=\"UTF-8\";");
}

DatabaseAdapter::~DatabaseAdapter()
{
    close();
}

void DatabaseAdapter::close()
{
    QSqlDatabase::removeDatabase(mFilename);
}

void DatabaseAdapter::insertNodeData(unsigned int _id, const QString &column, const QString &columnType, const QVariant &value)
{
    QString table = getOTypeFromNode(_id);
    if( !( mTableColumns.value(table).contains(column) ) ) {
        addTableColumn(table,column,columnType);
    }

    QSqlQuery q(QSqlDatabase::database(mFilename));
    QString queryString("INSERT INTO "+table+" ('"+column+"','_id') VALUES (:value,:id) ON CONFLICT (_id) DO UPDATE SET '"+column+"'=excluded.'"+column+"';");
    if( !q.prepare(queryString) ) {
        qWarning() << "DatabaseAdapter::insertNodeData" << q.lastError().text() << queryString;
        return;
    }
    q.bindValue(":value",value);
    q.bindValue(":id",_id);
    if( !q.exec() )
        qWarning() << "DatabaseAdapter::insertNodeData" << q.lastError().text() << q.executedQuery();
}

void DatabaseAdapter::insertEdgeData(const QString &table, unsigned int from, unsigned int to, const QVariant &value)
{
    QSqlQuery q(QSqlDatabase::database(mFilename));
    QString queryString("INSERT INTO "+table+" ('from_node','to_node','value') VALUES (:from,:to,:value);");
    if( !q.prepare(queryString) ) {
        qWarning() << "DatabaseAdapter::insertEdgeData" << q.lastError().text() << queryString;
        return;
    }
    q.bindValue(":from", from);
    q.bindValue(":to", to);
    q.bindValue(":value",value);
    if( !q.exec() )
        qWarning() << "DatabaseAdapter::insertNodeData" << q.lastError().text() << q.executedQuery();
}

void DatabaseAdapter::createTable(const QString & tableName, QSet<QString> columns, QHash<QString, QString> columnTypes )
{
    QSqlQuery q(QSqlDatabase::database(mFilename));

    if( !q.exec("drop table if exists " + tableName + ";") ) {
        qWarning() << "DatabaseAdapter::createTable" << q.lastError().text() << q.lastQuery();
    }

    QString query = "create table " + tableName + " ( ";
    QSetIterator<QString> i(columns);
    while(i.hasNext()) {
        QString c = i.next();
        query += " \"" + c + "\" " + columnTypes.value(c,"text");
        if( i.hasNext() ) {
            query += ", ";
        }
    }
    query += ");";

    if( !q.exec(query) ) {
        qWarning() << "DatabaseAdapter::createTable" << q.lastError().text() << query;
    }
}

void DatabaseAdapter::addTableColumn(const QString &table, const QString &column, const QString &columnType)
{
    QSqlQuery q(QSqlDatabase::database(mFilename));
    if( !q.exec("ALTER TABLE " + table + " ADD \"" + column + "\" "+columnType+";") ) {
        qWarning() << "DatabaseAdapter::addTableColumn" << q.lastError().text() << q.lastQuery();
        return;
    }
    mTableColumns[table] << column;
}

void DatabaseAdapter::setOtypeRanges(QHash<QString, QPair<unsigned int, unsigned int> > oTypeRanges)
{
    mOTypeRanges = oTypeRanges;
}

QString DatabaseAdapter::getOTypeFromNode(unsigned int node) const
{
    QHashIterator<QString,QPair<unsigned int,unsigned int>> i( mOTypeRanges );
    while (i.hasNext()) {
        i.next();
        if( node >= i.value().first && node <= i.value().second ) {
            return i.key();
        }
    }
    return "";
}

void DatabaseAdapter::beginTransaction()
{
    QSqlDatabase::database(mFilename).transaction();
}

void DatabaseAdapter::commitTransaction()
{
    QSqlDatabase::database(mFilename).commit();
}
