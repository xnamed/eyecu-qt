#ifndef HTTPUPLOAD_H
#define HTTPUPLOAD_H

#include <interfaces/ipluginmanager.h>
//#include <interfaces/ioptionsmanager.h>
//#include <interfaces/imainwindow.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ihttpupload.h>

#include <interfaces/istanzaprocessor.h>
#include <interfaces/idataforms.h>

struct ServiceItem {
    Jid service;
    int sizeLimit;
};

struct UploadItem {
    Jid streamJid;
    Jid contactJid;
    QString mimeType;
    QString name;
    qint64 size;
    QString url;
};

class HttpUpload: public QObject,
        public IPlugin,
        public IHttpUpload,
        public IStanzaRequestOwner
{
    Q_OBJECT
    Q_INTERFACES(IPlugin IHttpUpload IStanzaRequestOwner)
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
    //IStanzaRequestOwner
    virtual void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza);
protected:
    bool isSupported(const Jid &AStreamJid) const;
    void registerDiscoFeatures(bool ARegister);
    void requestUploadSlot(const Jid &AStreamJid, const QString &AService);
protected slots:
    void onDiscoInfoReceived(const IDiscoInfo &AInfo);
signals:
    void httpUploadError(const QString &AId, const XmppError &AError);
private:
    IDataForms          *FDataForms;
    IServiceDiscovery   *FDiscovery;
    IStanzaProcessor *FStanzaProcessor;
    IHttpUpload			*FHttpUpload;
    IconStorage         *FIconStorage;
private:
    QMap<QString, QString> FIqUploadRequests;
    QMap<Jid, ServiceItem> FServices;
};

#endif // HTTPUPLOAD_H
