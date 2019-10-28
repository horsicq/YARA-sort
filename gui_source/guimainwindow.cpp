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

#include "guimainwindow.h"
#include "ui_guimainwindow.h"

GuiMainWindow::GuiMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GuiMainWindow)
{
    ui->setupUi(this);

    setWindowTitle(QString("%1 v%2").arg(X_APPLICATIONNAME).arg(X_APPLICATIONVERSION));

    options={0};

    QString sSettingsFile=QApplication::applicationDirPath()+QDir::separator()+QString("%1.ini").arg(X_APPLICATIONNAME);
    QSettings settings(sSettingsFile,QSettings::IniFormat);

    ui->lineEditDirectoryName->setText(settings.value("DirectoryName",QDir::currentPath()).toString());
    ui->lineEditRules->setText(settings.value("Rules",QDir::currentPath()).toString());
    ui->lineEditOut->setText(settings.value("ResultName",QDir::currentPath()).toString());

    options.bContinue=settings.value("Continue",false).toBool();
    QString sDatabaseName=settings.value("DatabaseName",":memory:").toString();
//    QString sDatabaseName=settings.value("DatabaseName","C:\\tmp_build\\yara.db").toString();

    if(!ScanProgress::createDatabase(&options.dbSQLLite,sDatabaseName))
    {
        QMessageBox::critical(this,tr("Error"),tr("Cannot open SQLITE database"));
        exit(1);
    }
}

GuiMainWindow::~GuiMainWindow()
{
    QString sSettingsFile=QApplication::applicationDirPath()+QDir::separator()+QString("%1.ini").arg(X_APPLICATIONNAME);
    QSettings settings(sSettingsFile,QSettings::IniFormat);

    settings.setValue("DirectoryName",ui->lineEditDirectoryName->text());
    settings.setValue("Rules",ui->lineEditRules->text());
    settings.setValue("ResultName",ui->lineEditOut->text());

    delete ui;
}

void GuiMainWindow::on_pushButtonExit_clicked()
{
    this->close();
}

void GuiMainWindow::on_pushButtonOpenDirectory_clicked()
{
    QString sInitDirectory=ui->lineEditDirectoryName->text();

    QString sDirectoryName=QFileDialog::getExistingDirectory(this,tr("Open directory..."),sInitDirectory,QFileDialog::ShowDirsOnly);

    if(!sDirectoryName.isEmpty())
    {
        ui->lineEditDirectoryName->setText(sDirectoryName);
    }
}

void GuiMainWindow::on_pushButtonOut_clicked()
{
    QString sInitDirectory=ui->lineEditOut->text();

    QString sDirectoryName=QFileDialog::getExistingDirectory(this,tr("Open directory..."),sInitDirectory,QFileDialog::ShowDirsOnly);

    if(!sDirectoryName.isEmpty())
    {
        ui->lineEditOut->setText(sDirectoryName);
    }
}

void GuiMainWindow::on_pushButtonScan_clicked()
{
    _scan();
}

void GuiMainWindow::_scan()
{
    options.nCopyCount=ui->spinBoxCopyCount->value();
    options.sResultDirectory=ui->lineEditOut->text();
    options.sRules=ui->lineEditRules->text();
    options.bSubdirectories=ui->checkBoxScanSubdirectories->isChecked();

    DialogScanProgress ds(this);
    connect(&ds,SIGNAL(errorMessage(QString)),this,SLOT(errorMessage(QString)));

    ds.setData(ui->lineEditDirectoryName->text(),&options);

    ds.exec();

//    DialogStaticScan ds(this);
//    connect(&ds, SIGNAL(scanFileStarted(QString)),this,SLOT(scanFileStarted(QString)),Qt::DirectConnection);
//    connect(&ds, SIGNAL(scanResult(SpecAbstract::SCAN_RESULT)),this,SLOT(scanResult(SpecAbstract::SCAN_RESULT)),Qt::DirectConnection);
//    ds.setData(ui->lineEditDirectoryName->text(),&options);
//    ds.exec();
}

void GuiMainWindow::on_pushButtonInfo_clicked()
{
    QMessageBox::information(this,tr("Info"),tr("Bugreports: horsicq@gmail.com"));
}

void GuiMainWindow::on_pushButtonRules_clicked()
{
    QString sInitDirectory=ui->lineEditRules->text();

    QString sFileName=QFileDialog::getOpenFileName(this,tr("Open YARA rules file..."),sInitDirectory,"YARA rules files (*.yar)");

    if(!sFileName.isEmpty())
    {
        ui->lineEditRules->setText(sFileName);
    }
}

void GuiMainWindow::errorMessage(QString sText)
{
    QMessageBox::critical(this,"Error",sText);
}
