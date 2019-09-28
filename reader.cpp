#include "reader.h"
#include "databaseadapter.h"

#include <QString>
#include <QSetIterator>

Reader::Reader(const QString &folderPath, const QString &dbFilename) : mFolderPath(folderPath), mDbFilename(dbFilename)
{
    mDb = new DatabaseAdapter(mDbFilename);
    mFolder = QDir(folderPath);

    mFilesToSkip << "otype.tf" << "otext.tf";
}

Reader::~Reader()
{
    delete mDb;
}

void Reader::loadData()
{
    /// get the otypes
    processOtypeFile();

    mDb->setOtypeRanges( mOTypeRanges );

    /// load the files and read the relevant header-type information
    QFileInfoList fileList = mFolder.entryInfoList(QStringList("*.tf"),QDir::Files);
    foreach(QFileInfo info, fileList) {
        if( ! mFilesToSkip.contains( info.fileName() ) ) {
            mFiles << TFFile(info);
        }
    }

    /// create tables based on what was collected in the first pass
    createTables();

    /*
    for(int i=0; i<mFiles.count(); i++) {
        qDebug() << mFiles.at(i).label();
        mFiles[i].addDataToDatabase(mDb);
    }
    */
}

void Reader::processOtypeFile()
{
    QString path = mFolder.absoluteFilePath("otype.tf");
    QFile file( path );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File could not be opened: " << path;
    }

    QTextStream in(&file);

    // skip past header
    QString ln;
    do {
        ln = in.readLine();
    } while( ln.length() > 0 && ln.at(0) == "@" );

    QRegExp rx("^(.*)-(.*)\t(.*)$");
    do {
        ln = in.readLine();
        int pos = rx.indexIn(ln);
        if (pos > -1) {
            mOTypeRanges[ rx.cap(3) ] = QPair<int,int>( rx.cap(1).toInt(), rx.cap(2).toInt() );
        }
    } while( !in.atEnd() );
}

void Reader::createTables()
{
    /// NB: this able works differently from the others
    mDb->createOTypeTable();

    /// first the node tables
    foreach( QString otype, mOTypeRanges.keys() ) { /// word, book, chapter, clause... etc. Each will be a different table.
        QSet<QString> node_columns;
        QHash<QString, QString> node_columnTypes;
        QString dataType;

        node_columns << "_id";
        node_columnTypes["_id"] = "integer primary key";
        mDb->createTable( otype, node_columns, node_columnTypes );
    }

    /// then the edge tables
    for(int i=0; i<mFiles.count(); i++) {
        if( mFiles.at(i).fileType() == "@edge" ) {
            QSet<QString> edge_columns;
            QHash<QString, QString> edge_columnTypes;

            edge_columns << "value";
            if( mFiles.at(i).valueType() == "int" ) {
                edge_columnTypes["value"] = "integer";
            } else if( mFiles.at(i).valueType() == "str" ) {
                edge_columnTypes["value"] = "text";
            }

            edge_columns << "from_node" << "to_node";
            edge_columnTypes["from_node"] = "integer";
            edge_columnTypes["to_node"] = "integer";

            /// edge tables should be labled with the filename
            mDb->createTable( mFiles.at(i).label(), edge_columns, edge_columnTypes );
        }
    }

    /// TODO @config table?
}
