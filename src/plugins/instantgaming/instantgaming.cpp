#include "instantgaming.h"
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>


InstantGaming::InstantGaming():
	FDiscovery(NULL)
{
}

InstantGaming::~InstantGaming()
{}

void InstantGaming::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Instant Gaming");
	APluginInfo->description = tr("Serverless instant gaming in a one-to-one context.");
	APluginInfo->version = "1.0";
	APluginInfo->author = tr("Road Works Software");
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool InstantGaming::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	return true;

}

bool InstantGaming::initObjects()
{
	if (FDiscovery)
		registerDiscoFeatures();

	return true;
}

bool InstantGaming::initSettings()
{
	return true;
}

void InstantGaming::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.var = NS_INSTANTGAMING;
	dfeature.active = true;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_INSTANTGAMING);
	dfeature.name = tr("Instant Gaming");
	dfeature.description = tr("Supports serverless instant gaming");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool InstantGaming::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
			FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_INSTANTGAMING);
}
