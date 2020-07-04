#include "httpupload.h"

#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/discofeaturehandlerorders.h>

#include "utils/logger.h"
#include <QDebug>

#define DIC_STORE                     "store"
#define DIT_FILE                      "file"
#define FORM_FIELD_MAX_SIZE           "max-file-size"

HttpUpload::HttpUpload():
    FDataForms(nullptr),
    FDiscovery(nullptr),
	FHttpUpload(nullptr)
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
	IPlugin *plugin = APluginManager->pluginInterface("IDataForms").value(0, nullptr);
    if (plugin)
        FDataForms = qobject_cast<IDataForms *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
    if (plugin)
    {
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
        if (FDiscovery)
        {
            connect(FDiscovery->instance(),SIGNAL(discoInfoReceived(const IDiscoInfo &)),SLOT(onDiscoInfoReceived(const IDiscoInfo &)));
        }
    }

	return FDataForms!=nullptr;
}

bool HttpUpload::initSettings()
{
    return true;
}

bool HttpUpload::initObjects()
{
    //if (FOptionsManager)
    //    FOptionsManager->insertOptionsDialogHolder(this);
	if (FDiscovery)
	{
		registerDiscoFeatures(true);
		FDiscovery->insertFeatureHandler(NS_HTTP_UPLOAD,this,DFO_DEFAULT);
	}

    return true;
}

void HttpUpload::registerDiscoFeatures(bool ARegister)
{
    if (ARegister)
    {
        IDiscoFeature dfeature;
		dfeature.active = false;
        dfeature.var = NS_HTTP_UPLOAD;
		dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HTTP_UPLOAD);
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
	IDiscoInfo dinfo = FDiscovery ? FDiscovery->discoInfo(AStreamJid, AStreamJid.domain()) : IDiscoInfo();
    if (dinfo.features.contains(NS_HTTP_UPLOAD))
    {
        for (int i=0; !supported && i<dinfo.identity.count(); i++)
        {
            const IDiscoIdentity &ident = dinfo.identity.at(i);
            supported = ident.category==DIC_STORE && ident.type==DIT_FILE;
        }
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
                    bool found = false;
                    QList<IHttpUploadService *> services = availableServices(AInfo.streamJid);

                    for (QList<IHttpUploadService*>::ConstIterator it=services.constBegin(); it!=services.constEnd(); ++it)
                    {
                        if ((*it)->serviceJid() == AInfo.contactJid)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
						IHttpUploadService *service = new HttpUploadService(AInfo.streamJid, AInfo.contactJid, sizeLimit, this);
						services.append(service);
						FServices[AInfo.streamJid].append(service);
                        emit httpUploadServicesUpdated(AInfo.streamJid,services);
                    }
                }

                LOG_STRM_DEBUG(AInfo.streamJid,QString("HTTP Upload in disco info received from=%1").arg(AInfo.contactJid.full()));
                break;
            }
        }
    }
}

QList<IHttpUploadService *> HttpUpload::availableServices(const Jid &AStreamJid) const
{
	qDebug() << "availableServices:" << FServices.value(AStreamJid);
    return FServices.value(AStreamJid);
}

bool HttpUpload::execDiscoFeature(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo)
{
	if (AFeature == NS_HTTP_UPLOAD)
		//
	return false;
}

Action *HttpUpload::createDiscoFeatureAction(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo, QWidget *AParent)
{
	return NULL;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_httpupload, HttpUpload)
#endif
