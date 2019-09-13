#ifndef TFFILE_H
#define TFFILE_H

#include <QString>
#include <QFileInfo>
#include <QTextStream>

class Reader;
class DatabaseAdapter;

class TFFile
{
    friend Reader;
public:
    explicit TFFile(const QFileInfo & info);
    ~TFFile();

    QFileInfo info() const;

    /// @node, @edge, @config
    QString fileType() const;

    /// int, str
    QString valueType() const;

    /// integer, string
    QString valueTypeForSql() const;

    /// filename minus the extension
    QString label() const;

    void addDataToDatabase( DatabaseAdapter * db );

    static unsigned int max(QSet<unsigned int> set);
    static QString unescape(QString string);
    static QSet<unsigned int> nodeRangeToSet(const QString & range);

private:
    void addNodesToDatabase(DatabaseAdapter * db, QTextStream * stream );
    void addEdgesToDatabase(DatabaseAdapter *db, QTextStream * stream );
    void addConfigToDatabase(DatabaseAdapter * db, QTextStream * stream );

    /// read the first line of the file (@node, @edge) and return the string
    QString readFileType(QTextStream *stream);

    /// read the values of @valueType from header
    QString readValueType(QTextStream * stream);

    void skipOverHeader(QTextStream * stream);

    bool getHasEdgeValues(QTextStream *stream);

private:
    QFileInfo mInfo;
    QString mFileType, mValueType;
    bool mHasEdgeValues;
};

QDebug operator<<(QDebug dbg, const TFFile &key);

#endif // TFFILE_H
