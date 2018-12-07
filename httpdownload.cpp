#include "httpdownload.h"
#include "ui_httpdownload.h"

#include <QDebug>

HttpDownload::HttpDownload(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HttpDownload)
{
    ui->setupUi(this);

    ui->statusLabel->setWordWrap(true);
    ui->downloadButton->setDefault(true);
    ui->quitButton->setAutoDefault(false);

    ui->urlEdit->setText("https://cn.bing.com/az/hprichbg/rb/TaisetsuShirakawago_ZH-CN11784781173_1920x1080.jpg");

    resize(300, 200);

    connect(ui->urlEdit, SIGNAL(textChanged(QString)), this, SLOT(enableDownloadButton()));
}

HttpDownload::~HttpDownload()
{
    delete ui;
}

void HttpDownload::on_downloadButton_clicked()
{
    _pManager = new QNetworkAccessManager(this);

    // get url
    _url = (ui->urlEdit->text());

    qDebug() << "url:" << _url.fileName();

    if (!_url.isValid())
        return;

    QString dirPath = QDir::tempPath().append("/BQVISION/");
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(dirPath))
            return;
    }

    QString filePath;
    filePath.append(dirPath).append(_url.fileName());

    _pFile = new QFile(filePath);
    if (_pFile->exists()) {
        if (QMessageBox::question(this, tr("HTTP"), tr("There already exists a file called %1 in the current directory. Overwrite?").arg(_url.host()),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            return;
        }

        QFile::remove(filePath);
    }

    if (!_pFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"), tr("Unable to save the file %1: %2.").arg(_url.host()).arg(_pFile->errorString()));

        delete _pFile;
        _pFile = NULL;

        return;
    }

    // used for progressDialog
    // This will be set true when canceled from progress dialog
    _httpRequestAborted = false;

    _pProgressDialog = new QProgressDialog(this);
    connect(_pProgressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

    _pProgressDialog->setWindowTitle(tr("HTTP"));
    _pProgressDialog->setLabelText(tr("Downloading %1.").arg(_url.host()));

    // download button disabled after requesting download
    ui->downloadButton->setEnabled(false);

    startRequest(_url);
}

void HttpDownload::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (_pFile)
        _pFile->write(_pReply->readAll());
}

void HttpDownload::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (_httpRequestAborted)
        return;

    _pProgressDialog->setMaximum(totalBytes);
    _pProgressDialog->setValue(bytesRead);
}

void HttpDownload::on_quitButton_clicked()
{
    this->close();
}

void HttpDownload::on_urlEdit_returnPressed()
{
    on_downloadButton_clicked();
}

void HttpDownload::enableDownloadButton()
{
    ui->downloadButton->setEnabled(!(ui->urlEdit->text()).isEmpty());
}

// During the download progress, it can be canceled
void HttpDownload::cancelDownload()
{
    ui->statusLabel->setText(tr("Download canceled."));
    _httpRequestAborted = true;

    if (_pReply)
        _pReply->abort();

    ui->downloadButton->setEnabled(true);
}

// When download finished or canceled, this will be called
void HttpDownload::httpDownloadFinished()
{
    // when canceled
    if (_httpRequestAborted) {
        if (_pFile) {
            _pFile->close();
            _pFile->remove();
            delete _pFile;
            _pFile = 0;
        }
        _pReply->deleteLater();
        _pProgressDialog->hide();
        _pProgressDialog->deleteLater();
        return;
    }

    // download finished normally
    _pProgressDialog->hide();
    _pProgressDialog->deleteLater();
    _pFile->flush();
    _pFile->close();

    // get redirection url
    QVariant redirectionTarget = _pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (_pReply->error()) {
        _pFile->remove();
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(_pReply->errorString()));
        ui->downloadButton->setEnabled(true);
    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = _url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            _url = newUrl;
            _pReply->deleteLater();
            _pFile->open(QIODevice::WriteOnly);
            _pFile->resize(0);
            startRequest(_url);
            return;
        }
    } else {
        ui->statusLabel->setText(tr("Downloaded to %1.").arg(_pFile->fileName()));
        ui->downloadButton->setEnabled(true);
    }

    _pReply->deleteLater();
    _pReply = 0;
    delete _pFile;
    _pFile = 0;
    _pManager = 0;
}

// This will be called when download button is clicked
void HttpDownload::startRequest(QUrl url)
{
    // get() method posts a request
    // to obtain the contents of the target request
    // and returns a new QNetworkReply object
    // opened for reading which emits
    // the readyRead() signal whenever new data arrives.
    _pReply = _pManager->get(QNetworkRequest(url));

    // Whenever more data is received from the network,
    // this readyRead() signal is emitted
    connect(_pReply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));

    // Also, downloadProgress() signal is emitted when data is received
    connect(_pReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDownloadProgress(qint64,qint64)));

    // This signal is emitted when the reply has finished processing.
    // After this signal is emitted,
    // there will be no more updates to the reply's data or metadata.
    connect(_pReply, SIGNAL(finished()), this, SLOT(httpDownloadFinished()));
}
