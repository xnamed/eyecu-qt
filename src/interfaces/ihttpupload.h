#ifndef IHTTPUPLOAD_H
#define IHTTPUPLOAD_H

#include <utils/jid.h>
#include <utils/xmpperror.h>

#define HTTPUPLOAD_UUID "{c3854a22-432a-d901-1213-57ac94294092}"

class IHttpUploadService
{
public:
    virtual QObject *instance() =0;
    virtual const Jid &streamJid() const =0;
    virtual const Jid &serviceJid() const =0;
    virtual int sizeLimit() const =0;
    virtual int uploadFile(QIODevice *ADevice, const QByteArray &AContentType=QByteArray(), QString AFileName=QString()) =0;
protected:
    virtual void httpUploadServiceError(const QString &AId, const XmppError &AError) =0;
    virtual void httpUploadError(int AId, const QString &AError) =0;
    virtual void httpUploadFinished(int AId, const QUrl &AUrl) =0;
};

class IHttpUpload
{
public:
    virtual QObject *instance() =0;
    virtual QList<IHttpUploadService *> availableServices(const Jid &AStreamJid) const=0;
protected:
    virtual void httpUploadServicesUpdated(const Jid &AStreamJid, const QList<IHttpUploadService *> &AServices) =0;
};

Q_DECLARE_INTERFACE(IHttpUploadService,"RWS.Plugin.IHttpUploadService/1.0")
Q_DECLARE_INTERFACE(IHttpUpload,"RWS.Plugin.IHttpUpload/1.0")

#endif // IHTTPUPLOAD_H
