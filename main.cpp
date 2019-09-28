#include <QCoreApplication>
#include <QtDebug>

#include "reader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString dataPath, outputFile;

    if( argc == 1 ) {
        dataPath = "C:/Users/Adam/text-fabric-data/etcbc/bhsa/tf/c";
        outputFile = "bhsa.sqlite";
        qDebug() << "Beause you provided no arguments, the default values indicated below are being used. To specify where your the data are, and where you want the output file, use the command like this:";
        qDebug() << "read-text-fabric \"C:/path/to/text-fabric-data/etcbc/bhsa/tf/c\" \"mysqlitedatabase.sqlite\" ";
    } else {
       dataPath = QString::fromLocal8Bit(argv[1]);
       outputFile = QString::fromLocal8Bit(argv[2]);
    }

    qDebug() << "";
    qDebug() << "Path to data: " << dataPath;
    qDebug() << "SQLlite file to create: " << outputFile;

    Reader r(dataPath, outputFile);
    r.loadData();

    return 0;
}
