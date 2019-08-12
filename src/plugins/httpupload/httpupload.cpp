#include "httpupload.h"

/*#include "definitions/optionvalues.h"
#include "definitions/optionwidgetorders.h"
#include "definitions/optionnodes.h"
#include "definitions/soundfiles.h"*/
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#include "utils/logger.h"
#include <QDebug>

#define SLOT_IQ_TIMEOUT         30000

#define DIC_STORE                     "store"
#define DIT_FILE                      "file"
#define FORM_FIELD_MAX_SIZE           "max-file-size"

HttpUpload::HttpUpload():
    FDataForms(nullptr),
    FDiscovery(nullptr),
    //FOptionsManager(nullptr),
    FStanzaProcessor(nullptr),
    FHttpUpload(nullptr),
    FIconStorage(nullptr)
{}

HttpUpload::~HttpUpload()
{}

void HttpUpload::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("HTTP File Upload");
    APluginInfo->description = tr("Allows to upload a file to a specific path on an HTTP server and at the same time receive a URL from which that file can later be downloaded again.");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(SERVICEDISCOVERY_UUID);
}

bool HttpUpload::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);
    IPlugin *plugin = APluginManager->pluginInterface("IDataForms").value(0,nullptr);
    if (plugin)
        FDataForms = qobject_cast<IDataForms *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,nullptr);
    if (plugin)
        FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
    {
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
        if (FDiscovery)
        {
            connect(FDiscovery->instance(),SIGNAL(discoInfoReceived(const IDiscoInfo &)),SLOT(onDiscoInfoReceived(const IDiscoInfo &)));
        }
    }

    //plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
    //if (plugin)
    //    FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    return FDataForms!=NULL && FStanzaProcessor!=NULL;
}

bool HttpUpload::initSettings()
{
    return true;
}

bool HttpUpload::initObjects()
{
    //if (FOptionsManager)
    //    FOptionsManager->insertOptionsDialogHolder(this);

    return true;
}

void HttpUpload::registerDiscoFeatures(bool ARegister)
{
    if (ARegister)
    {
        IDiscoFeature dfeature;
        dfeature.active = true;
        dfeature.var = NS_HTTP_UPLOAD;
        dfeature.icon = FIconStorage->getIcon(MNI_HTTP_UPLOAD);
        dfeature.name = tr("HTTP File Upload");
        dfeature.description = tr("Supports HTTP File Upload");
        FDiscovery->insertDiscoFeature(dfeature);
    }
    else
        FDiscovery->removeDiscoFeature(NS_HTTP_UPLOAD);
}

bool HttpUpload::isSupported(const Jid &AStreamJid) const
{
    bool supported = false;
    IDiscoInfo dinfo = FDiscovery!=NULL ? FDiscovery->discoInfo(AStreamJid, AStreamJid.domain()) : IDiscoInfo();
    for (int i=0; !supported && dinfo.features.contains(NS_HTTP_UPLOAD) && i<dinfo.identity.count(); i++)
    {
        const IDiscoIdentity &ident = dinfo.identity.at(i);
        supported = ident.category==DIC_STORE && ident.type==DIT_FILE;
    }
    return supported;
}

void HttpUpload::onDiscoInfoReceived(const IDiscoInfo &AInfo)
{
    if (FDataForms && AInfo.node.isEmpty())
    {
        foreach(const IDataForm &form, AInfo.extensions)
        {
            if (FDataForms->fieldValue("FORM_TYPE",form.fields).toString() == NS_HTTP_UPLOAD)
            {
                qDebug() << "HTTP Upload Service" << AInfo.contactJid.full();
                int sizeLimit = FDataForms->fieldValue(FORM_FIELD_MAX_SIZE,form.fields).toInt();
                qDebug() << "Size limit:" << sizeLimit;
                if (sizeLimit > 0)
                {
                   ServiceItem &item = FServices[AInfo.streamJid];
                   item.service = AInfo.contactJid;
                   item.sizeLimit = sizeLimit;
                }

                LOG_STRM_DEBUG(AInfo.streamJid,QString("HTTP Upload in disco info received from=%1").arg(AInfo.contactJid.full()));
                break;
            }
        }
    }
}

void HttpUpload::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
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

            if (!put.isEmpty() || !get.isEmpty())
            {}
            else
            {}
        }
        else
        {
            XmppStanzaError err(AStanza);
            LOG_STRM_WARNING(AStreamJid,QString("Failed requesting slot, id=%2: %3").arg(AStanza.id(),err.condition()));
            emit httpUploadError(AStanza.id(),err);
        }
    }
}

// Request a slot on the upload service
void HttpUpload::requestUploadSlot(const Jid &AStreamJid, const QString &AService)
{
    Stanza request(STANZA_KIND_IQ);
    request.setType(STANZA_TYPE_GET).setTo(AService).setUniqueId();
    request.addElement("request",NS_HTTP_UPLOAD);
    //request.setAttribute("filename", fileName);
    //request.setAttribute("size", ASize);
    //request.setAttribute("content-type", mimeType);
    if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,SLOT_IQ_TIMEOUT))
    {
        LOG_STRM_INFO(AStreamJid,QString("New upload slot request sent, to=%1").arg(AService));
        FIqUploadRequests.insert(request.id(),AService);
    }
    else
    {
        LOG_STRM_WARNING(AStreamJid,QString("Failed to send upload slot request, to=%1").arg(AService));
    }
}
