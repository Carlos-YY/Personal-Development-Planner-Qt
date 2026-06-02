#ifndef RESUMEEXPORTER_H
#define RESUMEEXPORTER_H

#include "datastore.h"
#include <QString>

class ResumeExporter
{
public:
    static bool exportMarkdown(const DataStore &store, const QString &filePath);
};

#endif // RESUMEEXPORTER_H
