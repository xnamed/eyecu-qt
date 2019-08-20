#ifndef HTTPUPLOADSERVICE_H
#define HTTPUPLOADSERVICE_H

#include <interfaces/ihttpupload.h>
#include <interfaces/istanzaprocessor.h>
#include <QNetworkAccessManager>

struct UploadUrl
{
    QString put;
    QString get;
};

class HttpUploadService:
        public QObject,
        public IHttpUploadService,
        public IStanzaRequestOwner
{
    Q_OBJECT
    Q_INTERFACES(IHttpUploadService IStanzaRequestOwner)
public:
    HttpUploadService(const Jid &AService, int ASizeLimit);
    ~HttpUploadService();

    virtual QObject *instance() { return this; }
    virtual Jid serviceJid() const;
    virtual int sizeLimit() const;
    virtual int uploadFile(const Jid &AStreamJid, const Jid &AService, QIODevice *ADevice, const QByteArray &AContentType, const QString &AFileName);
    //IStanzaRequestOwner
    virtual void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza);
protected:
    bool requestUploadSlot(const Jid &AStreamJid, const Jid &AService, qint64 ASize, const QByteArray &AContentType, const QString &AFileName, int AId);
    void startUpload(const QString &APut, int AId);
protected slots:
    void onUploadFinished(int AId);
signals:
    void httpUploadServiceError(const QString &AId, const XmppError &AError);
    void httpUploadError(int AId, const QString &AError);
    void httpUploadFinished(int AId, const QString &AUrl);
private:
    Jid FServiceJid;
    int FSizeLimit;
    IStanzaProcessor       *FStanzaProcessor;
    QNetworkAccessManager  *FNetworkAccessManager;
    QMap<QString, QString> FIqUploadRequests;
    QHash<int, QIODevice *> FIODevices;
    QHash<int, UploadUrl *> FUploadUrl;
};

#endif // HTTPUPLOADSERVICE_H
