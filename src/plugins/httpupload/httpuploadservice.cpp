#include "httpuploadservice.h"

#include <definitions/namespaces.h>
#include "utils/pluginhelper.h"
#include <utils/jid.h>
#include <utils/xmpperror.h>
#include "utils/logger.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>

#define SLOT_IQ_TIMEOUT         30000
#define DEFAULT_CONTENT_TYPE          "application/octet-stream"

HttpUploadService::HttpUploadService(const Jid &AService, int ASizeLimit):
    FServiceJid(AService),
    FSizeLimit(ASizeLimit),
    FStanzaProcessor(PluginHelper::pluginInstance<IStanzaProcessor>())
{
    FNetworkAccessManager = new QNetworkAccessManager(this);
}

HttpUploadService::~HttpUploadService()
{}

void HttpUploadService::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
    // Upload service responds with a slot
    if (FIqUploadRequests.contains(AStanza.id()))
    {
        QDomElement slot=AStanza.firstElement("slot", NS_HTTP_UPLOAD);
        if (!slot.isNull())
        {
            QString put=slot.firstChildElement("put").text();
            QString get=slot.firstChildElement("get").text();
            qDebug() << "PUT:" << put;
            qDebug() << "GET:" << get;

            if ((!put.isEmpty() || !get.isEmpty()) && (FIODevices.contains(AStanza.id().toInt())))
            {
                FUploadUrl[AStanza.id().toInt()]->put = put;
                FUploadUrl[AStanza.id().toInt()]->get = get;
                startUpload(put,AStanza.id().toInt());
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

int HttpUploadService::uploadFile(const Jid &AStreamJid, const Jid &AService, QIODevice *ADevice, const QByteArray &AContentType, const QString &AFileName)
{
    int id=qrand();
    int sizeLimit = 0;

    if (sizeLimit > 0 && !ADevice->size() > sizeLimit)
    {
        FIODevices.insert(id, ADevice);
        requestUploadSlot(AStreamJid, AService, ADevice->size(), AContentType, AFileName, id);
    }

    return id;
}

// Request a slot on the upload service
bool HttpUploadService::requestUploadSlot(const Jid &AStreamJid, const Jid &AService, qint64 ASize, const QByteArray &AContentType, const QString &AFileName, int AId)
{
    Stanza request(STANZA_KIND_IQ);
    request.setType(STANZA_TYPE_GET).setTo(AService.bare()).setId(QString::number(AId));
    request.addElement("request",NS_HTTP_UPLOAD);
    request.setAttribute("filename", AFileName);
    request.setAttribute("size", QString::number(ASize));
    request.setAttribute("content-type", AContentType.isEmpty() || AContentType==NULL ? DEFAULT_CONTENT_TYPE : AContentType);
    if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,SLOT_IQ_TIMEOUT))
    {
        LOG_STRM_INFO(AStreamJid,QString("New upload slot request sent, to=%1").arg(AService.bare()));
        FIqUploadRequests.insert(request.id(),AService.bare());
        return true;
    }
    else
    {
        LOG_STRM_WARNING(AStreamJid,QString("Failed to send upload slot request, to=%1").arg(AService.bare()));
    }

    return false;
}

void HttpUploadService::startUpload(const QString &APut, int AId)
{
    QIODevice *device = FIODevices.take(AId);
    QNetworkRequest request;
    request.setUrl(QUrl(APut));
    request.setHeader(QNetworkRequest::ContentLengthHeader, device->size());
    QNetworkReply *reply=FNetworkAccessManager->put(request, device->readAll());
    connect(reply, SIGNAL(finished()), SLOT(onUploadFinished(AId)));
 //   connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(onProgressChanged(qint64,qint64)))
}

void HttpUploadService::onUploadFinished(int AId)
{
    QNetworkReply *reply=qobject_cast<QNetworkReply *>(sender());
    if (reply->error()==QNetworkReply::NoError)
    {
        FIODevices.remove(AId);
        emit httpUploadFinished(AId,FUploadUrl[AId]->get);
    }
    else
    {
        emit httpUploadError(AId,reply->errorString());
    }
}

Jid HttpUploadService::serviceJid() const
{
    return FServiceJid;
}

int HttpUploadService::sizeLimit() const
{
    return FSizeLimit;
}
