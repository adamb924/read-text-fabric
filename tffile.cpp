#include "tffile.h"

#include <QDebug>

#include "databaseadapter.h"

TFFile::TFFile(const QFileInfo & info) :
    mInfo(info)
{
    QFile file(mInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File could not be opened: " << mInfo.absoluteFilePath();
    }
    QTextStream * stream = new QTextStream(&file);
    stream->setCodec("UTF-8");

    /// these must be called in order
    mFileType = readFileType(stream);
    mValueType = readValueType(stream);
    mHasEdgeValues = getHasEdgeValues(stream);
    delete stream;
}

TFFile::~TFFile()
{
}

QFileInfo TFFile::info() const
{
    return mInfo;
}

QString TFFile::fileType() const
{
    return mFileType;
}

QString TFFile::valueType() const
{
    return mValueType;
}

QString TFFile::valueTypeForSql() const
{
    if( mValueType == "str" ) {
        return "text";
    } else if( mValueType == "int" ) {
        return "integer";
    } else {
        return "generate-an-error";
    }
}

QString TFFile::label() const
{
    return mInfo.baseName();
}

void TFFile::addDataToDatabase(DatabaseAdapter * db)
{
    QFile file(mInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File could not be opened: " << mInfo.absoluteFilePath();
    }
    QTextStream * stream = new QTextStream(&file);
    stream->setCodec("UTF-8");

    if( mFileType == "@node" ) {
        addNodesToDatabase(db, stream);
    } else if( mFileType == "@edge" ) {
        addEdgesToDatabase(db, stream);
    } else if( mFileType == "@config" ) {
        addConfigToDatabase(db, stream);
    }
    delete stream;
}

void TFFile::addNodesToDatabase(DatabaseAdapter *db, QTextStream * stream)
{
    skipOverHeader(stream);
    db->beginTransaction();

    unsigned int implicitNode = 0;

    QString column = label();
    QString columnType = valueTypeForSql();

    QString value;
    while (!stream->atEnd()) {
        QString line = stream->readLine();
        QStringList dataLine = line.split("\t"); /// it might be tab-delimited
        QSet<unsigned int> nodeSet;
        if( dataLine.count() == 2 ) { /// if it is tab delimited, the first thing is the node number, the second is the data
            nodeSet = nodeRangeToSet( dataLine.at(0) );
            value = unescape(dataLine.at(1));
            implicitNode = max(nodeSet);
            QSetIterator<unsigned int> i(nodeSet);
            while(i.hasNext()) {
                db->insertNodeData( i.next(), column, columnType, value );
            }
        } else if( dataLine.count() == 1 ) {
            implicitNode++;
            value = unescape(dataLine.at(0));
            db->insertNodeData( implicitNode, column, columnType, value );
        } else {
            qDebug() << "Data line count error: " << dataLine.count();
        }
    }
    db->commitTransaction();
}

void TFFile::addEdgesToDatabase(DatabaseAdapter * db, QTextStream * stream)
{
    Q_UNUSED(db)
    skipOverHeader(stream);

    unsigned int lineCount = 0;
    unsigned int implicitNode = 0;

    QSet<unsigned int> from_index, to_index;
    QString value;
    while (!stream->atEnd()) {
        QString line = stream->readLine();
        if(line.isEmpty()) {
            break;
        } else {
            lineCount++;

            QStringList dataLine = line.split("\t");
            if( dataLine.count() == 3 ) {
                from_index = nodeRangeToSet( dataLine.at(0) );
                to_index = nodeRangeToSet( dataLine.at(1) );
                value = unescape(dataLine.at(2));
                implicitNode = max(from_index);
            } else if( dataLine.count() == 2 ) {
                // check the first node anyway
                if( mHasEdgeValues ) {
                    implicitNode++;
                    from_index.clear();
                    from_index << implicitNode;
                    to_index = nodeRangeToSet( dataLine.at(0) );
                    value = unescape(dataLine.at(1));
                } else {
                    // check the second node only if there are no values and this node should be interpreted as a node
                    from_index = nodeRangeToSet( dataLine.at(0));
                    to_index = nodeRangeToSet(dataLine.at(1));
                    value = "";
                    implicitNode = max(from_index);
                }
            } else if( dataLine.count() == 1 ) {
                implicitNode++;
                from_index.clear();
                from_index << implicitNode;
                to_index = nodeRangeToSet(dataLine.at(0));
                value = "";
            } else {
                qDebug() << "Edge line count error: " << dataLine.count();
            }

            QSetIterator<unsigned int> i(from_index);
            while(i.hasNext()) {
                unsigned int current_i = i.next();
                QSetIterator<unsigned int> j(to_index);
                while(j.hasNext()) {
                    db->insertEdgeData(label(), current_i, j.next(), value);
                }
            }
        }
    }
}

void TFFile::addConfigToDatabase(DatabaseAdapter * db, QTextStream * stream)
{
    Q_UNUSED(db)
    Q_UNUSED(stream)
    /// TODO: ?
}

unsigned int TFFile::max(QSet<unsigned int> set)
{
    unsigned int max = 0;
    QSetIterator<unsigned int> i(set);
    while( i.hasNext() ) {
        unsigned int thisOne = i.next();
        if( thisOne > max ) {
            max = thisOne;
        }
    }
    return max;
}

QString TFFile::unescape(QString string)
{
    // vaguely cheating...
    QString str = string;
    str.replace("\\\\",QChar(0xffff)); // change \\ to an unused code
    str.replace("\\t","\t");
    str.replace("\\n","\n");
    str.replace(QChar(0xffff),"\\");
    return str;
}

QSet<unsigned int> TFFile::nodeRangeToSet(const QString &range)
{
    /*
     * Testing
        qDebug() << nodeRangeToSet("45");
        qDebug() << nodeRangeToSet("5-13");
        qDebug() << nodeRangeToSet("1-3,5-10,15,23-37");
        qDebug() << nodeRangeToSet("1-5,2-7");
        qDebug() << nodeRangeToSet("3-1");
    */
    QSet<unsigned int> set;
    QStringList ranges = range.split(",");
    foreach(QString range, ranges) {
        QStringList pairString = range.split("-");
        QPair<unsigned int,unsigned int> pair(0,0);
        if(pairString.count() == 1) {
            pair.first = pairString.at(0).toUInt();
            pair.second = pairString.at(0).toUInt();
        } else if(pairString.count() == 2) {
            pair.first = pairString.at(0).toUInt();
            pair.second = pairString.at(1).toUInt();
        } else {
            qDebug() << "Error in pair format.";
        }
        if( pair.first > pair.second ) {
            unsigned int temp;
            temp = pair.first;
            pair.first = pair.second;
            pair.second = temp;
        }
        for(unsigned int i=pair.first; i<=pair.second; i++) {
            set << i;
        }
    }
    return set;
}

QString TFFile::readFileType(QTextStream * stream)
{
    stream->reset();
    return stream->readLine();
}

QString TFFile::readValueType(QTextStream * stream)
{
    stream->seek(0);
    QRegExp rx("^@valueType=(.*)$");
    QString ln;
    while( !stream->atEnd() ) {
        ln = stream->readLine();
        int pos = rx.indexIn(ln);
        if (pos > -1) {
            return rx.cap(1);
        }
    }
    return "";
}

void TFFile::skipOverHeader(QTextStream * stream)
{
    stream->seek(0);
    QString ln;
    do {
        ln = stream->readLine();
    } while( ln.length() > 0 && ln.at(0) == "@" );
}

bool TFFile::getHasEdgeValues(QTextStream * stream)
{
    stream->reset();
    QString ln;
    do {
        ln = stream->readLine();
        if( ln == "@edgeValues" ) {
            return true;
        }
    } while( !ln.isEmpty() );
    return false;
}

QDebug operator<<(QDebug dbg, const TFFile &key)
{
    dbg.nospace() << "TFFile(" << key.info().fileName() << ", " << key.fileType() << ", " << key.valueType() << ")";
    return dbg.maybeSpace();
}
