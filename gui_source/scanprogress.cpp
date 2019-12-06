// copyright (c) 2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "scanprogress.h"

ScanProgress::ScanProgress(QObject *parent) : QObject(parent)
{
    bIsStop=false;
    _pOptions=nullptr;
    currentStats=STATS();
    pElapsedTimer=nullptr;
}

void ScanProgress::setData(QString sDirectoryName, ScanProgress::SCAN_OPTIONS *pOptions)
{
    this->_sDirectoryName=sDirectoryName;
    this->_pOptions=pOptions;
}

quint32 ScanProgress::getFileCount(quint32 nCRC)
{
    quint32 nResult=0;

    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec(QString("SELECT FILECOUNT FROM records where FILECRC='%1'").arg(nCRC));

    if(query.next())
    {
        nResult=query.value("FILECOUNT").toString().trimmed().toUInt();
    }

    if(query.lastError().text().trimmed()!="")
    {
        qDebug(query.lastQuery().toLatin1().data());
        qDebug(query.lastError().text().toLatin1().data());
    }

    return nResult;
}

void ScanProgress::setFileCount(quint32 nCRC, quint32 nCount)
{
    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec(QString("INSERT OR REPLACE INTO records(FILECRC,FILECOUNT) VALUES('%1','%2')").arg(nCRC).arg(nCount));

    if(query.lastError().text().trimmed()!="")
    {
        qDebug(query.lastQuery().toLatin1().data());
        qDebug(query.lastError().text().toLatin1().data());
    }
}

void ScanProgress::setFileStat(QString sFileName, QString sTimeCount, QString sDate)
{
    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec(QString("INSERT OR REPLACE INTO files(FILENAME,TIMECOUNT,DATETIME) VALUES('%1','%2','%3')")
               .arg(sFileName)
               .arg(sTimeCount)
               .arg(sDate));

    // QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")

    if(query.lastError().text().trimmed()!="")
    {
        qDebug(query.lastQuery().toLatin1().data());
        qDebug(query.lastError().text().toLatin1().data());
    }
}

void ScanProgress::createTables()
{
    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec("DROP TABLE if exists records");
    query.exec("DROP TABLE if exists files");
    query.exec("CREATE TABLE if not exists records(FILECRC text,FILECOUNT text,PRIMARY KEY(FILECRC))");
    query.exec("CREATE TABLE if not exists files(FILENAME text,TIMECOUNT text,DATETIME text,PRIMARY KEY(FILENAME))");
}

QString ScanProgress::getCurrentFileName()
{
    QString sResult;

    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec(QString("SELECT FILENAME FROM files where TIMECOUNT='' AND DATETIME='' LIMIT 1"));

    if(query.next())
    {
        sResult=query.value("FILENAME").toString().trimmed();
    }

    if(query.lastError().text().trimmed()!="")
    {
        qDebug(query.lastQuery().toLatin1().data());
        qDebug(query.lastError().text().toLatin1().data());
    }

    return sResult;
}

qint64 ScanProgress::getNumberOfFile()
{
    qint64 nResult=0;

    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec(QString("SELECT COUNT(FILENAME) AS VALUE FROM files where TIMECOUNT='' AND DATETIME=''"));

    if(query.next())
    {
        nResult=query.value("VALUE").toULongLong();
    }

    return nResult;
}

void ScanProgress::findFiles(QString sDirectoryName)
{
    if(!bIsStop)
    {
        QFileInfo fi(sDirectoryName);

        if(fi.isFile())
        {
            currentStats.nTotal++;
            setFileStat(fi.absoluteFilePath(),"","");
        }
        else if(fi.isDir()&&(_pOptions->bSubdirectories))
        {
            QDir dir(sDirectoryName);

            QFileInfoList eil=dir.entryInfoList();
            
            int nCount=eil.count();

            for(int i=0; (i<nCount)&&(!bIsStop); i++)
            {
                QString sFN=eil.at(i).fileName();

                if((sFN!=".")&&(sFN!=".."))
                {
                    findFiles(eil.at(i).absoluteFilePath());
                }
            }
        }
    }
}

void ScanProgress::startTransaction()
{
    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec("BEGIN TRANSACTION");
}

void ScanProgress::endTransaction()
{
    QSqlQuery query(_pOptions->dbSQLLite);

    query.exec("COMMIT");
}

void ScanProgress::process()
{
    pElapsedTimer=new QElapsedTimer;
    pElapsedTimer->start();

    if(!(_pOptions->bContinue))
    {
        createTables();
    }
    currentStats.nTotal=0;
    currentStats.nCurrent=0;

    bIsStop=false;

    currentStats.sStatus=tr("Directory scan");

    if(!(_pOptions->bContinue))
    {
        startTransaction();

        findFiles(_sDirectoryName);

        endTransaction();
    }

    currentStats.nTotal=getNumberOfFile();

    QYara yara(this);
    if(yara.loadRulesFile(_pOptions->sRules))
    {
        for(int i=0; (i<currentStats.nTotal)&&(!bIsStop); i++)
        {
            currentStats.nCurrent=i+1;
            currentStats.sStatus=getCurrentFileName();

            if(currentStats.sStatus=="")
            {
                break;
            }

            setFileStat(currentStats.sStatus,"",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

            QYara::RESULT scanResult=yara.scanFile(currentStats.sStatus);

            int nCount=scanResult.listRecords.count();

            for(int i=0;i<nCount;i++)
            {
                QString sResult=XBinary::convertFileNameSymbols(scanResult.listRecords.at(i));

                quint32 nCRC=XBinary::getStringCustomCRC32(sResult);

                bool bCopy=true;

                int nCurrentCount=getFileCount(nCRC);

                if(_pOptions->nCopyCount)
                {
                    if(nCurrentCount>=_pOptions->nCopyCount)
                    {
                        bCopy=false;
                    }
                }

                if(bCopy)
                {
                    QString sFileName=_pOptions->sResultDirectory;

                    XBinary::createDirectory(sFileName);
                    sFileName+=QDir::separator()+sResult;
                    XBinary::createDirectory(sFileName);
                    sFileName+=QDir::separator()+XBinary::getBaseFileName(scanResult.sFileName);

                    if(XBinary::copyFile(scanResult.sFileName,sFileName))
                    {
                        setFileCount(nCRC,nCurrentCount+1);
                    }
                }
            }

            setFileStat(scanResult.sFileName,QString::number(scanResult.nScanTime),QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        }
    }
    else
    {
        emit errorMessage("Cannot load rules!");
    }

    emit completed(pElapsedTimer->elapsed());
    delete pElapsedTimer;
    pElapsedTimer=nullptr;

    bIsStop=false;
}

void ScanProgress::stop()
{
    bIsStop=true;
}

ScanProgress::STATS ScanProgress::getCurrentStats()
{
    if(pElapsedTimer)
    {
        currentStats.nElapsed=pElapsedTimer->elapsed();
    }

    return currentStats;
}

bool ScanProgress::createDatabase(QSqlDatabase *pDb, QString sDatabaseName)
{
    bool bResult=false;

    *pDb=QSqlDatabase::addDatabase("QSQLITE", "sqllite");
    pDb->setDatabaseName(sDatabaseName);

    if(pDb->open())
    {
        QSqlQuery query(*pDb);

        query.exec("PRAGMA journal_mode = WAL");
        query.exec("PRAGMA synchronous = NORMAL");

        bResult=true;
    }

    return bResult;
}
