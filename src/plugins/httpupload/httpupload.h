#ifndef HTTPUPLOAD_H
#define HTTPUPLOAD_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ihttpupload.h>
#include <interfaces/idataforms.h>
#include <httpuploadservice.h>

class HttpUpload: public QObject,
        public IPlugin,
        public IHttpUpload
{
    Q_OBJECT
    Q_INTERFACES(IPlugin IHttpUpload)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IHttpUpload")
#endif
public:
    HttpUpload();
    ~HttpUpload();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return HTTPUPLOAD_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IHttpUploadService
    virtual QList<IHttpUploadService *> availableServices(const Jid &AStreamJid) const;
protected:
    bool isSupported(const Jid &AStreamJid) const;
    void registerDiscoFeatures(bool ARegister);
protected slots:
    void onDiscoInfoReceived(const IDiscoInfo &AInfo);
signals:
    void httpUploadServicesUpdated(const Jid &AStreamJid, const QList<IHttpUploadService *> &AServices);
private:
    IDataForms          *FDataForms;
    IServiceDiscovery   *FDiscovery;
    IHttpUpload			*FHttpUpload;
    IconStorage         *FIconStorage;
private:
    QHash<Jid, QList<IHttpUploadService *> >FServices;
};

#endif // HTTPUPLOAD_H
