#include "FilesParser.h"

FilesParser::FilesParser(QObject *parent, QString inputDirPath)
    : QObject{parent}
{
    m_inputDirPath = inputDirPath;
    m_currentSpectrumIndex = 0;
}

void FilesParser::setInputDirPath(const QString &newInputDirPath)
{
    m_inputDirPath = newInputDirPath;
}
