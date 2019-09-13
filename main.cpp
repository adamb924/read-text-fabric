#include <QCoreApplication>
#include <QtDebug>

#include "reader.h"

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    Reader r("C:/Users/Adam/text-fabric-data/etcbc/bhsa/tf/c", "bhsa.sqlite");
    r.loadData();

    return 0;
}
