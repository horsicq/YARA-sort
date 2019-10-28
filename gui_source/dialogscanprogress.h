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

#ifndef DIALOGSCANPROGRESS_H
#define DIALOGSCANPROGRESS_H

#include <QDialog>
#include <QTimer>
#include <QThread>
#include <QDateTime>
#include "scanprogress.h"

namespace Ui {
class DialogScanProgress;
}

class DialogScanProgress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogScanProgress(QWidget *parent = 0);
    ~DialogScanProgress();

    void setData(QString sDirectoryName,ScanProgress::SCAN_OPTIONS *pOptions);

private slots:
    void on_pushButtonCancel_clicked();
    void onCompleted(qint64 nElapsed);
    void onSetProgressMaximum(int nValue);
    void onSetProgressValue(int nValue);
    void timerSlot();

signals:
    void errorMessage(QString sText);

private:
    Ui::DialogScanProgress *ui;
    ScanProgress *pScan;
    QThread *pThread;
    bool bIsRun;
    QTimer *pTimer;
};

#endif // DIALOGSCANPROGRESS_H
