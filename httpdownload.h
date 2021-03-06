#ifndef HTTPDOWNLOAD_H
#define HTTPDOWNLOAD_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QProgressDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

namespace Ui {
class HttpDownload;
}

class HttpDownload : public QDialog
{
    Q_OBJECT

public:
    explicit HttpDownload(QWidget *parent = 0);
    ~HttpDownload();

public:
    void startRequest(QUrl _url);
private slots:
    void on_downloadButton_clicked();

    void on_quitButton_clicked();

    void on_urlEdit_returnPressed();

    // slot for readyRead() signal
    void httpReadyRead();

    // slot for finished() signal from reply
    void httpDownloadFinished();

    // slot for downloadProgress()
    void updateDownloadProgress(qint64, qint64);

    void enableDownloadButton();
    void cancelDownload();

private:
    Ui::HttpDownload *ui;
    QUrl _url;
    QNetworkAccessManager *_pManager;
    QNetworkReply *_pReply;
    QProgressDialog *_pProgressDialog;
    QFile *_pFile;
    bool _httpRequestAborted;
    qint64 _fileSize;
};

#endif // HTTPDOWNLOAD_H
