#ifndef VSBAK_H
#define VSBAK_H

#include <QWidget>
#include <QProcess>
#include <QTime>

namespace Ui {
class vsbak;
}

class vsbak : public QWidget
{
    Q_OBJECT

public:
    explicit vsbak(QWidget *parent = 0);
    ~vsbak();

private slots:

    void on_readyReadStandardError();

    void on_readyReadStandardOutput();

    void on_uploadProgress(qint64 readBytes, qint64 totalBytes);

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_checkBox_clicked();

    void on_lineEdit_3_editingFinished();

    void on_lineEdit_4_editingFinished();

    void on_pushButton_3_clicked();

    void on_toolButton_3_clicked();

    void on_lineEdit_editingFinished();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_5_editingFinished();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

private:
    Ui::vsbak *ui;
    QProcess *cmd;

    QString config_file_path;
    QString dir_src;
    QString dir_dest;
    QString dir_restore;
    QString gpg_key;
    QString arc_user;
    QString arc_path;
    QString exclude_from;
    QDateTime lastBackupTime;

    bool signal_disp_output;
    bool signal_disp_error;

    QString backupInfo();
    void outputInfo(QString out, QColor color = QColor(Qt::darkBlue));
    void configFileWrite(QString path, QString str);
    void executeCommand(QString line, QString command, QStringList argument = QStringList(), bool sig_out = true, bool sig_err = true);
};

#endif // VSBAK_H
