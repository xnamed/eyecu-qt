#include "httpuploadservice.h"

#include <definitions/namespaces.h>
#include "utils/pluginhelper.h"
#include <utils/jid.h>
#include <utils/xmpperror.h>
#include "utils/logger.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>

#define SLOT_IQ_TIMEOUT         30000
#define DEFAULT_CONTENT_TYPE          "application/octet-stream"

HttpUploadService::HttpUploadService(const Jid &AStreamJid, const Jid &AServiceJid, int ASizeLimit, QObject *APrent):
	QObject(APrent),
	FStreamJid(AStreamJid),
	FServiceJid(AServiceJid),
    FSizeLimit(ASizeLimit),
	FStanzaProcessor(PluginHelper::pluginInstance<IStanzaProcessor>()),
	FNetworkAccessManager(new QNetworkAccessManager(this))
{

}

HttpUploadService::~HttpUploadService()
{}

QObject *HttpUploadService::instance()
{
	return this;
}

void HttpUploadService::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
    // Upload service responds with a slot
    if (FIqUploadRequests.contains(AStanza.id()))
    {
		int id = FIqUploadRequests[AStanza.id()];
        QDomElement slot=AStanza.firstElement("slot", NS_HTTP_UPLOAD);
        if (!slot.isNull())
        {
			UploadRequest &request = FRequests[id];
			request.put = QUrl::fromPercentEncoding(slot.firstChildElement("put").text().toLatin1());
			request.get = QUrl::fromPercentEncoding(slot.firstChildElement("get").text().toLatin1());
			qDebug() << "PUT:" << request.put;
			qDebug() << "GET:" << request.get;

			if (!request.put.isEmpty() || !request.get.isEmpty())
            {
				startUpload(request.put, request.device);
            }
            else
            {
                // Error requesting slot
            }
        }
        else
        {
            XmppStanzaError err(AStanza);
            LOG_STRM_WARNING(AStreamJid,QString("Failed requesting slot, id=%2: %3").arg(AStanza.id(),err.condition()));
            emit httpUploadServiceError(AStanza.id(),err);
        }
    }
}

int HttpUploadService::uploadFile(QIODevice *ADevice, const QByteArray &AContentType, QString AFileName)
{
	if (ADevice->isOpen() && ADevice->size()>0 && FSizeLimit > 0 && ADevice->size() <= FSizeLimit)
    {
		int id;
		for (id=qrand(); FRequests.contains(id); id++);

		if (AFileName.isEmpty()) // No file name specified
		{
			QFile *file = qobject_cast<QFile*>(ADevice);
			if (file)
				AFileName = QFileInfo(*file).fileName();
			else	// Not a file
				return 0;
		}
		if (requestUploadSlot(FStreamJid, FServiceJid, ADevice->size(), AContentType, AFileName, id))
		{
			UploadRequest &request = FRequests[id];
			request.device = ADevice;
			return id;
		}
    }
	return 0;
}

// Request a slot on the upload service
bool HttpUploadService::requestUploadSlot(const Jid &AStreamJid, const Jid &AService, qint64 ASize, const QByteArray &AContentType, const QString &AFileName, int AId)
{
    Stanza request(STANZA_KIND_IQ);
    request.setType(STANZA_TYPE_GET).setTo(AService.bare()).setId(QString::number(AId));
    request.addElement("request",NS_HTTP_UPLOAD);
    request.setAttribute("filename", AFileName);
    request.setAttribute("size", QString::number(ASize));
	request.setAttribute("content-type", AContentType.isEmpty() ? DEFAULT_CONTENT_TYPE : AContentType);
    if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,SLOT_IQ_TIMEOUT))
    {
        LOG_STRM_INFO(AStreamJid,QString("New upload slot request sent, to=%1").arg(AService.bare()));
		FIqUploadRequests.insert(request.id(), AId);
        return true;
    }
    else
    {
        LOG_STRM_WARNING(AStreamJid,QString("Failed to send upload slot request, to=%1").arg(AService.bare()));
    }

    return false;
}

void HttpUploadService::startUpload(const QUrl &APutUrl, QIODevice *ADevice)
{
    QNetworkRequest request;
	request.setUrl(APutUrl);
	request.setHeader(QNetworkRequest::ContentLengthHeader, ADevice->size());
	QNetworkReply *reply=FNetworkAccessManager->put(request, ADevice);
	connect(reply, SIGNAL(finished()), SLOT(onUploadFinished()));
 //   connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(onProgressChanged(qint64,qint64)))
}

void HttpUploadService::onUploadFinished()
{
    QNetworkReply *reply=qobject_cast<QNetworkReply *>(sender());
	int id = FReplies.take(reply);
    if (reply->error()==QNetworkReply::NoError)
    {
		emit httpUploadFinished(id, FRequests[id].get);
    }
    else
    {
		emit httpUploadError(id,reply->errorString());
    }
}

const Jid &HttpUploadService::streamJid() const
{
	return FStreamJid;
}

const Jid &HttpUploadService::serviceJid() const
{
    return FServiceJid;
}

int HttpUploadService::sizeLimit() const
{
    return FSizeLimit;
}
