#ifndef READER
#define READER

#include <QtSql>
#include <QHash>
#include <QSet>
#include <QPair>

#include "tffile.h"

class DatabaseAdapter;

class Reader{
public:
    Reader(const QString & folderPath, const QString & dbFilename);
    ~Reader();

    void loadData();

private:
    void processOtypeFile();
    void createTables();

    QString mFolderPath, mDbFilename;
    QStringList mFilesToSkip;
    QHash<QString,QPair<unsigned int,unsigned int>> mOTypeRanges;

    QList<TFFile> mFiles;

    DatabaseAdapter * mDb;
    QDir mFolder;
};


#endif // READER

