#ifndef HTTPUPLOADSERVICE_H
#define HTTPUPLOADSERVICE_H

#include <interfaces/ihttpupload.h>
#include <interfaces/istanzaprocessor.h>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

struct UploadRequest
{
	QUrl put;
	QUrl get;
	QIODevice *device;
};

class HttpUploadService:
        public QObject,
        public IHttpUploadService,
        public IStanzaRequestOwner
{
    Q_OBJECT
    Q_INTERFACES(IHttpUploadService IStanzaRequestOwner)

public:
	HttpUploadService(const Jid &AStreamJid, const Jid &AServiceJid, int ASizeLimit, QObject *APrent=nullptr);
    ~HttpUploadService();

	virtual QObject *instance() override;
	virtual const Jid& streamJid() const override;
	virtual const Jid& serviceJid() const override;
	virtual int sizeLimit() const override;
	virtual int uploadFile(QIODevice *ADevice, const QByteArray &AContentType=QByteArray(), QString AFileName=QString()) override;
    //IStanzaRequestOwner
	virtual void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza) override;
protected:
    bool requestUploadSlot(const Jid &AStreamJid, const Jid &AService, qint64 ASize, const QByteArray &AContentType, const QString &AFileName, int AId);
	void startUpload(const QUrl &APutUrl, QIODevice *ADevice);
protected slots:
	void onUploadFinished();
signals:
	void httpUploadServiceError(const QString &AId, const XmppError &AError) override;
	void httpUploadError(int AId, const QString &AError) override;
	void httpUploadFinished(int AId, const QUrl &AUrl) override;
private:
	const Jid FStreamJid;
	const Jid FServiceJid;
	const int FSizeLimit;
    IStanzaProcessor       *FStanzaProcessor;
    QNetworkAccessManager  *FNetworkAccessManager;
	QMap<QString, int> FIqUploadRequests;
	QHash<int, UploadRequest> FRequests;
	QHash<QNetworkReply*, int> FReplies;
};

#endif // HTTPUPLOADSERVICE_H
