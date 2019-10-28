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

#include "dialogscanprogress.h"
#include "ui_dialogscanprogress.h"

DialogScanProgress::DialogScanProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogScanProgress)
{
    ui->setupUi(this);

    pScan=new ScanProgress;
    pThread=new QThread;

    pScan->moveToThread(pThread);

    connect(pThread, SIGNAL(started()), pScan, SLOT(process()));
    connect(pScan, SIGNAL(completed(qint64)), this, SLOT(onCompleted(qint64)));
    connect(pScan, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));
    bIsRun=false;

    pTimer=new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
}

DialogScanProgress::~DialogScanProgress()
{
    if(bIsRun)
    {
        pScan->stop();
    }

    pTimer->stop();

    pThread->quit();
    pThread->wait();

    delete ui;

    delete pThread;
    delete pScan;
}

void DialogScanProgress::setData(QString sDirectoryName, ScanProgress::SCAN_OPTIONS *pOptions)
{
    bIsRun=true;
    pScan->setData(sDirectoryName,pOptions);
    pThread->start();
    pTimer->start(1000);
    ui->progressBarTotal->setMaximum(100);
}

void DialogScanProgress::on_pushButtonCancel_clicked()
{
    if(bIsRun)
    {
        pScan->stop();
        pTimer->stop();
        bIsRun=false;
    }
}

void DialogScanProgress::onCompleted(qint64 nElapsed)
{
    bIsRun=false;
    this->close();
}

void DialogScanProgress::onSetProgressMaximum(int nValue)
{
    ui->progressBarTotal->setMaximum(nValue);
}

void DialogScanProgress::onSetProgressValue(int nValue)
{
    ui->progressBarTotal->setMaximum(nValue);
}

void DialogScanProgress::timerSlot()
{
    ScanProgress::STATS stats=pScan->getCurrentStats();

    ui->labelTotal->setText(QString::number(stats.nTotal));
    ui->labelCurrent->setText(QString::number(stats.nCurrent));
    ui->labelCurrentStatus->setText(stats.sStatus);

    if(stats.nTotal)
    {
        ui->progressBarTotal->setValue((int)((stats.nCurrent*100)/stats.nTotal));
    }

    QDateTime dt;
    dt.setMSecsSinceEpoch(stats.nElapsed);
    QString sDateTime=dt.time().addSecs(-60*60).toString("hh:mm:ss");

    ui->labelTime->setText(sDateTime);
}
