#ifndef IHTTPUPLOAD_H
#define IHTTPUPLOAD_H

#include <utils/jid.h>

#define HTTPUPLOAD_UUID "{c3854a22-432a-d901-1213-57ac94294092}"

class IHttpUploadService
{
public:
    virtual QObject *instance() =0;
    virtual Jid serviceJid() const =0;
    virtual int sizeLimit() const =0;
    virtual int uploadFile(const Jid &AStreamJid, const Jid &AService, QIODevice *ADevice, const QByteArray &AContentType=QByteArray(), const QString &AFileName=QString()) =0;
};

class IHttpUpload
{
public:
    virtual QObject *instance() =0;
    virtual QList<IHttpUploadService *> availableServices(const Jid &AStreamJid) const=0;
};

Q_DECLARE_INTERFACE(IHttpUploadService,"RWS.Plugin.IHttpUploadService/1.0")
Q_DECLARE_INTERFACE(IHttpUpload,"RWS.Plugin.IHttpUpload/1.0")

#endif // IHTTPUPLOAD_H
