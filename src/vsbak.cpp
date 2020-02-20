#include "vsbak.h"
//#include "ui_vsbak.h"
#include "vsbak_ui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <algorithm>
#include <QRegExp>
#include <QDebug>
/* library for uplaod file with ftp */
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QEventLoop>

#define CONFIG_FILE_NAME "vsbak_config.ini"

vsbak::vsbak(QWidget *parent) : QWidget(parent), ui(new Ui::vsbak)
{
    ui->setupUi(this);

    // the config file is ~/.config/vsbak/vsbak_config.ini
    config_file_path = QDir::homePath() + "/.config/vsbak/" + CONFIG_FILE_NAME;
   //config_file_path = QCoreApplication::applicationDirPath() + '/' + CONFIG_FILE_NAME;

    // read config.ini.
    QFile cfile(config_file_path);
    if(cfile.exists()){
        QSettings *configIniRead = new QSettings(config_file_path, QSettings::IniFormat);
        dir_src = configIniRead->value("/main/bak_src").toString();
        dir_dest = configIniRead->value("/main/bak_dest").toString();
        dir_restore = dir_src;
        arc_user = configIniRead->value("/main/arc_user").toString();
        arc_path = configIniRead->value("/main/arc_path").toString();
        gpg_key = configIniRead->value("/main/gpg_key").toString();
        exclude_from = configIniRead->value("exclude/exclude").toString();
        delete configIniRead;
    }else{
        QString err = QString("No file :  %1!").arg(config_file_path);
        exclude_from = "--exclude *.iso --exclude .gvfs/** --exclude .thumbnails/** --exclude .trash/** --exclude .Trash/** --exclude *-gitsvn/** --exclude *-cvs/** --exclude *-svn/** --exclude *-git/** --exclude *-sf/** --exclude *.vc --exclude *.tc --exclude .cache/** --exclude .ccache/** --exclude ./Downloads/** --exclude ./.wine/** --exclude ./Documents/Dropbox/**";
        configFileWrite("exclude/exclude",exclude_from);
        outputInfo(err,QColor(Qt::darkRed));
    }

    // init bash terminal
    cmd = new QProcess(this);
    cmd->setWorkingDirectory(dir_src);

    connect(cmd, &QProcess::readyReadStandardError, this, &vsbak::on_readyReadStandardError);
    connect(cmd, &QProcess::readyReadStandardOutput, this, &vsbak::on_readyReadStandardOutput);

    // hide groupbutton
    ui->label_4->setText("gpg_key");
    ui->lineEdit_6->setText(gpg_key);
    ui->groupBox_edit_config->hide();

    // init lineEdit
    ui->lineEdit->setText(dir_src);
    ui->lineEdit_2->setText(dir_dest);
    ui->lineEdit_3->setText(arc_user);
    ui->lineEdit_4->setText(arc_path);
    ui->lineEdit_5->setText(dir_restore);
    ui->lineEdit_7->setEchoMode(QLineEdit::PasswordEchoOnEdit);

    // init radiobutton: choice full
    ui->radioButton->setChecked(true);
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(true);
    ui->checkBox_2->setEnabled(false);
    ui->checkBox_3->setEnabled(false);

    // hide progress bar
    ui->progressBar->setVisible(false);

    // get backup info
    backupInfo();

    // output info
    outputInfo("welcome to vsbak  ^_^\n");
}

vsbak::~vsbak()
{
    delete ui;

    if(cmd->state() == QProcess::Running){
        cmd->terminate();
        cmd->waitForFinished();
    }
}


void vsbak::on_readyReadStandardError()
{
    if(signal_disp_error){
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->setTextColor(QColor(Qt::red));
        ui->textBrowser->insertPlainText(cmd->readAllStandardError().data());
    }
}

void vsbak::on_readyReadStandardOutput()
{
    if(signal_disp_output){
        ui->textBrowser->moveCursor(QTextCursor::End);
        ui->textBrowser->setTextColor(QColor(Qt::black));
        ui->textBrowser->insertPlainText(cmd->readAllStandardOutput().data());
    }
}

// progress bar function for upload
void vsbak::on_uploadProgress(qint64 readBytes, qint64 totalBytes)
{
    ui->progressBar->setVisible(readBytes != totalBytes);
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(readBytes);
}

/*******************************************************
 * qt slot functions
********************************************************/
void vsbak::on_lineEdit_editingFinished()
{
    QString change_text = ui->lineEdit->text();
    if(dir_src != change_text){
        QDir dir(change_text);
        if(dir.exists()){
            dir_src = change_text;
            cmd->setWorkingDirectory(dir_src);
            configFileWrite("/main/bak_src", dir_src);
            outputInfo(QString("change source dir success: %1\n").arg(dir_src));
        }else
            outputInfo("Fail! please retry to change the src dir of Backup!\n",QColor(Qt::darkRed));
    }
}

void vsbak::on_lineEdit_2_editingFinished()
{
    QString change_text = ui->lineEdit_2->text();
    if(dir_dest != change_text){
        QDir dir(change_text);
        if (dir.exists()){
            dir_dest = change_text;
            configFileWrite("/main/bak_dest", dir_dest);
            outputInfo(QString("change dest dir success: %1\n").arg(dir_dest));
        }else
            outputInfo("Fail! please retry to change the dest dir of Backup!\n",QColor(Qt::darkRed));
    }
}

void vsbak::on_lineEdit_3_editingFinished()
{
    QString changed_words = ui->lineEdit_3->text();
    if(changed_words != arc_user){
        arc_user = changed_words;
        configFileWrite("/main/arc_user", changed_words);
        outputInfo(QString("change arc_user succeed: %1\n").arg(changed_words));
    }
}

void vsbak::on_lineEdit_4_editingFinished()
{
    QString changed_words = ui->lineEdit_4->text();
    if(changed_words != arc_path){
        arc_path = changed_words;
        configFileWrite("/main/arc_path", changed_words);
        outputInfo(QString("change arc_path succeed: %1\n").arg(changed_words));
    }
}

void vsbak::on_lineEdit_5_editingFinished()
{
    QString change_text = ui->lineEdit_5->text();
    if(dir_restore != change_text ){
        QDir dir(change_text);
        if (dir.exists()){
            dir_restore = change_text;
            outputInfo(QString("change restore dir success: %1\n").arg(dir_restore));
        }else
            outputInfo("Fail! please retry to change the dir of Restore!\n",QColor(Qt::darkRed));
    }
}

void vsbak::on_pushButton_6_clicked()
{
    ui->textBrowser->clear();
}

void vsbak::on_pushButton_5_clicked()
{
    outputInfo(backupInfo());
}

void vsbak::on_pushButton_4_clicked()
{
    if(ui->groupBox_edit_config->isVisible()){
        ui->pushButton_4->setText("more>>>");
        ui->groupBox_edit_config->hide();
    }
    else{
        ui->pushButton_4->setText("more<<<");
        ui->groupBox_edit_config->show();
    }
}

void vsbak::on_pushButton_9_clicked()
{
    QString show =  QString("cat %1").arg(config_file_path);
    executeCommand("show config file",show);
}

void vsbak::on_pushButton_7_clicked()
{
    if(ui->label_4->text() == "gpg_key"){
        ui->label_4->setText("exclude");
        ui->lineEdit_6->setText(exclude_from);
    }else{
        ui->label_4->setText("gpg_key");
        ui->lineEdit_6->setText(gpg_key);
    }
}

void vsbak::on_pushButton_8_clicked()
{
    QString words = ui->lineEdit_6->text();
    if(ui->label_4->text() == "gpg_key"){
        gpg_key = words;
        configFileWrite("main/gpg_key", words);
        outputInfo(QString("gpg_key has eidted: %1 \n").arg(words));
    }else{
        exclude_from = words;
        configFileWrite("exclude/exclude",words);
        outputInfo(QString("exclude has eidted: %1 \n").arg(words));
    }
}

// change dir_src
void vsbak::on_toolButton_clicked()
{
    QString dirname;
    dirname = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir_src, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dirname.isEmpty()) return;
    else if (dirname != dir_src) {
        dir_src = dirname;
        ui->lineEdit->setText(dir_src);
        configFileWrite("/main/bak_src", dir_src);
        cmd->setWorkingDirectory(dir_src);
        outputInfo(QString("change source dir success: %1\n").arg(dir_src));
    }
}

// change dir_dest
void vsbak::on_toolButton_2_clicked()
{
    QString dirname;
    dirname = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir_dest, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dirname.isEmpty()) return;
    else if(dirname != dir_dest) {
        dir_dest = dirname;
        ui->lineEdit_2->setText(dir_dest);
        configFileWrite("/main/bak_dest", dir_dest);
        outputInfo(QString("change dest dir success: %1\n").arg(dir_dest));
    }
    backupInfo();
}


// choice restore dir
void vsbak::on_toolButton_3_clicked()
{
    QString dirname;
    dirname = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir_restore, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dirname.isEmpty()) return;
    else if(dirname != dir_restore)  {
        dir_restore = dirname;
        ui->lineEdit_5  ->setText(dirname);
        outputInfo(QString("change restore dir success: %1\n").arg(dir_restore));
    }
}

// checkBox_encrypting
void vsbak::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked()){
        ui->checkBox_2->setEnabled(true);
        ui->checkBox_3->setEnabled(true);
    }
    else{
        ui->checkBox_2->setEnabled(false);
        ui->checkBox_3->setEnabled(false);
    }
}

/*******************************************************
 * function backup
 * push button
********************************************************/
void vsbak::on_pushButton_clicked()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString targz, cmd_find, cmd_tar;

    // if choice full backup
    if(ui->radioButton->isChecked()){
        cmd_find = "find ./ -type f | grep -v \'Permission denied\' > TEPLIST";
        targz = current_date_time.toString("yyyy-MM-dd_hh.mm.ss") + ".full.tar.gz";
    }else{
        // if choice inc backup
        if(lastBackupTime.isNull())
            cmd_find = "find ./ -type f | grep -v \'Permission denied\' > TEPLIST";
        else{
            int minDiff = (current_date_time.toTime_t()-lastBackupTime.toTime_t()) / 60 + 1;
            cmd_find = QString("find ./ -mmin -%1 -type f | grep -v \'Permission denied\' > TEPLIST").arg(minDiff);
        }
        targz = current_date_time.toString("yyyy-MM-dd_hh.mm.ss") + ".inc.tar.gz";
    }

    QStringList cmd_pipe;
    cmd_pipe << "-c" << cmd_find;
    executeCommand(NULL, "bash", cmd_pipe)  ;

    cmd_tar = QString("tar --verbose --totals --exclude-vcs %1 --files-from=TEPLIST -czf %2/%3").arg(exclude_from).arg(dir_dest).arg(targz);
    executeCommand("backup",cmd_tar);

    executeCommand(NULL, "rm TEPLIST");

    /* Encrypting */
    if(ui->checkBox->isChecked()){
        QString cmd_isKey = "gpg -k " + gpg_key;
        executeCommand(NULL, cmd_isKey, QStringList(), false, false);          // find or the key exists
        QString cmd_enc;
        QString out = QString::fromLocal8Bit(cmd->readAllStandardOutput());
        if(out.contains(QString("<%1>").arg(gpg_key))) {
            outputInfo(out,QColor(Qt::black));
            cmd_enc = QString("gpg -e -r %1 -o %2/%3.gpg %2/%3").arg(gpg_key).arg(dir_dest).arg(targz);
        }
        else {
            cmd_enc = QString("gpg -c -o %1/%2.gpg %1/%2").arg(dir_dest).arg(targz);
        }
        cmd->close();
        executeCommand(NULL, cmd_enc);            // encrypting the tar file

        if(false == ui->checkBox_2->isChecked()){
            executeCommand(NULL, QString("rm %1/%2.gpg").arg(dir_dest).arg(targz));
            outputInfo(QString("delete %1/%2.gpg\n").arg(dir_dest).arg(targz));
        }

        if(false == ui->checkBox_3->isChecked()){
            executeCommand(NULL, QString("rm %1/%2").arg(dir_dest).arg(targz));
            outputInfo(QString("delete %1/%2\n").arg(dir_dest).arg(targz));
        }
    }

    outputInfo("backup finished!\n");
    backupInfo();
}


/*******************************************************
 * function restore
 * push button 2
********************************************************/
void vsbak::on_pushButton_2_clicked()
{
    QString targz = ui->comboBox->currentText();

    if (targz.endsWith(".tar.gz.gpg")){        
        targz = targz.remove(".gpg");
        QFile tarFile(QString("%1/%2").arg(dir_dest).arg(targz));
        if(!tarFile.exists()){          // check if the tar file exists.if not ,do below.
            // find if key and gen exists.            
            QString cmd_enc;
            QString cmd_isKey = "gpg -k " + gpg_key;
            executeCommand(NULL, cmd_isKey, QStringList(), false, false);          // find or the key exists
            QString out = QString::fromLocal8Bit(cmd->readAllStandardOutput());
            if(out.contains(QString("<%1>").arg(gpg_key))){
//                outputInfo(out, QColor(Qt::black));
                cmd_enc = QString("gpg -d -r %1 -o %2/%3 %2/%3.gpg").arg(gpg_key).arg(dir_dest).arg(targz);
            } else cmd_enc = QString("gpg -d -o %1/%2 %1/%2.gpg").arg(dir_dest).arg(targz);
            cmd->close();
            executeCommand(NULL,cmd_enc);

            QString cmd_restore = QString("tar --verbose --totals -xzf %1/%2 -C %3").arg(dir_dest).arg(targz).arg(dir_restore);
            executeCommand("execute restore",cmd_restore);
            outputInfo("restore finished!\n");
            QString cmd_rm = QString("rm %1/%2").arg(dir_dest).arg(targz);
            executeCommand(NULL, cmd_rm);
            return;
        }
    }

    QString cmd_restore = QString("tar --verbose --totals -xzf %1/%2 -C %3").arg(dir_dest).arg(targz).arg(dir_restore);
    executeCommand("execute restore", cmd_restore);
    outputInfo("restore finished!\n");
}

/*******************************************************
 * function upload
 * push button 3
 * lftp -e "put $TARGZ_ENC -o $ARCPATH$TARGZ_ENC_BASE && quit" -u $ARCUSER archiv.luis.uni-hannover.de
********************************************************/
void vsbak::on_pushButton_3_clicked()
{

    QString ftp_passwd = ui->lineEdit_7->text();
    if(ftp_passwd.isEmpty()){
        outputInfo("Error: please input password!\n",QColor(Qt::red));
        return;
    }
//    else ui->lineEdit_7->clear();   // clear password for security
    QString upload_target = ui->comboBox->currentText();
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString work_path = cmd->workingDirectory();
    outputInfo(QString("%1  %2: upload %3\n").arg(work_path).arg(current_date_time.toString("yyyy/MM/dd hh:mm:ss")).arg(upload_target),QColor(Qt::darkGreen));

    /* set archive */
    QUrl url;
    url.setScheme("ftp");
    url.setUserName(arc_user);
    url.setPassword(ftp_passwd);
    url.setHost("archiv.luis.uni-hannover.de");
    url.setPort(21);
    url.setPath(QString("%1/%2").arg(arc_path).arg(upload_target));
    /* read date */
    QFile file(QString("%1/%2").arg(dir_dest).arg(upload_target));
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();
    /* upload */
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.put(request, data);

    ui->progressBar->show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(100);

    /* wait for uplaod finish */
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(on_uploadProgress(qint64, qint64)));
    eventLoop.exec();

    if(reply->error() != QNetworkReply::NoError){
        ui->progressBar->hide();
        outputInfo(QString("Error: %1\n").arg(reply->errorString()), QColor(Qt::red));
    }else {
        outputInfo("upload succeed!\n");
        /* delete tar file if need */
        if(ui->checkBox_4->isChecked()){
        QString cmd_rm = QString("rm %1").arg(upload_target);
        executeCommand(NULL,cmd_rm);
        outputInfo(QString("Delete: %1\n").arg(upload_target));
        backupInfo();
        }
    }

}


/*******************************************************
 * self-defining functions:
 * bool compareFileName (const QFileInfo & a, const QFileInfo & b)
 * void executeCommand(QString line, QString command, QStringList argument = QStringList(), bool dis = true);
 * QString backupInfo()
 * void outputInfo(QString out)
 * void configFileWrite(QString path, QString str)
********************************************************/
bool compareFileName(const QFileInfo & a, const QFileInfo & b)
{
    return a.fileName() > b.fileName();
}

QString vsbak::backupInfo()
{
    // find all backup.tar.gz files
    QDir *curDir = new QDir(dir_dest);
    QStringList filter;
    filter << "*.tar.gz" << "*.tar.gz.gpg";
    curDir->setNameFilters(filter);
    QList<QFileInfo> *tarFileInfo = new QList<QFileInfo>(curDir->entryInfoList(filter));

    // sort all backup by time
    qSort(tarFileInfo->begin(),tarFileInfo->end(),compareFileName);

    // update comboBox display
    ui->comboBox->clear();
    ui->comboBox->setMaxVisibleItems(5);
    ui->comboBox->setInsertPolicy(QComboBox::InsertAtTop);
    for (int i=0; i< tarFileInfo->count(); i++)
        ui->comboBox->addItem(tarFileInfo->at(i).fileName());

    // match if full or inc
    QRegExp isFullBackup(".*full.*");
    QRegExp isIncBackup(".*inc.*");
    QString lastFullBackup,lastIncBackup;

    for (int i=0; i< tarFileInfo->count(); i++){
        if(lastFullBackup.isEmpty() && isFullBackup.exactMatch(tarFileInfo->at(i).fileName()))
            lastFullBackup = tarFileInfo->at(i).fileName();
        if(lastIncBackup.isEmpty() && isIncBackup.exactMatch(tarFileInfo->at(i).fileName()))
            lastIncBackup = tarFileInfo->at(i).fileName();
    }

    // get last backup time and output on ui
    QString out1,out2,out3;
    if(lastFullBackup.isEmpty() && lastIncBackup.isEmpty()){
        out1 = "last full backup : None";
        out2 = "last inc backup : None";
        out3 = "last backup : None";
    }
    if(!lastFullBackup.isEmpty() && lastIncBackup.isEmpty()){
        QDateTime lastFullTime = QDateTime::fromString(lastFullBackup.mid(0,19), "yyyy-MM-dd_hh.mm.ss");
        lastBackupTime = lastFullTime;
        out1 = QString("last full backup : %1").arg(lastFullTime.toString("dd/MM/yyyy hh:mm:ss"));
        out2 = QString("last inc backup : None");
        out3 = QString("last backup (full): %1").arg(lastFullTime.toString("dd/MM/yyyy hh:mm:ss"));
    }
    if(lastFullBackup.isEmpty() && !lastIncBackup.isEmpty()){
        QDateTime lastIncTime = QDateTime::fromString(lastIncBackup.mid(0,19), "yyyy-MM-dd_hh.mm.ss");
        lastBackupTime = lastIncTime;
        out1 = QString("last full backup : None");
        out2 = QString("last inc backup : %1").arg(lastIncTime.toString("dd/MM/yyyy hh:mm:ss"));
        out3 = QString("last backup (inc): %1").arg(lastIncTime.toString("dd/MM/yyyy hh:mm:ss"));
    }
    if(!lastFullBackup.isEmpty() && !lastIncBackup.isEmpty()){
        QDateTime lastFullTime = QDateTime::fromString(lastFullBackup.mid(0,19), "yyyy-MM-dd_hh.mm.ss");
        QDateTime lastIncTime = QDateTime::fromString(lastIncBackup.mid(0,19), "yyyy-MM-dd_hh.mm.ss");
        out1 = QString("last full backup : %1").arg(lastFullTime.toString("dd/MM/yyyy hh:mm:ss"));
        out2 = QString("last inc backup : %1").arg(lastIncTime.toString("dd/MM/yyyy hh:mm:ss"));
        if(lastFullBackup>lastIncBackup){
            out3 = QString("last backup (full): %1").arg(lastFullTime.toString("dd/MM/yyyy hh:mm:ss"));
            lastBackupTime = lastFullTime;
        }else{
            out3 = QString("last backup (inc): %1").arg(lastIncTime.toString("dd/MM/yyyy hh:mm:ss"));
            lastBackupTime = lastIncTime;
        }
    }

    return (out1+'\n'+out2+'\n'+out3+'\n');
}

void vsbak::executeCommand(QString line, QString command, QStringList argument ,bool sig_out, bool sig_err)
{
    signal_disp_output = sig_out;
    signal_disp_error = sig_err;
    if(line != NULL){
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString work_path = cmd->workingDirectory();
        ui->textBrowser->setTextColor(QColor(Qt::darkGreen));
        ui->textBrowser->insertPlainText(QString("%1  %2: %3\n").arg(work_path).arg(current_date_time.toString("yyyy/MM/dd hh:mm:ss")).arg(line));
    }

    if (argument.isEmpty())
        cmd->start(command);
    else
        cmd->start(command,argument);
    cmd->waitForFinished();
    if(sig_out && sig_err) cmd->close();
}

void vsbak::outputInfo(QString out, QColor color)
{
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->setTextColor(color);
    ui->textBrowser->insertPlainText(out);
}

void vsbak::configFileWrite(QString path, QString str)
{
    QSettings *configIniWrite = new QSettings(config_file_path, QSettings::IniFormat);
    configIniWrite->setValue(path, str);
    delete configIniWrite;
}

