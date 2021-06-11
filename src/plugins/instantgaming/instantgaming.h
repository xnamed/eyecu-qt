#ifndef INSTANTGAMING_H
#define INSTANTGAMING_H

#include <interfaces/iinstantgaming.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iservicediscovery.h>

class InstantGaming:
		public QObject,
		public IPlugin,
		public IInstantGaming
{

	Q_OBJECT
	Q_INTERFACES(IPlugin IInstantGaming)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IInstantGaming")
#endif

public:
	InstantGaming();
	~InstantGaming();

	//IPlugin
	QObject *instance() {return this;}
	QUuid pluginUuid() const {return INSTANTGAMING_UUID;}
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}

protected:
	void registerDiscoFeatures();
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
private:
	IServiceDiscovery *FDiscovery;
};

#endif // INSTANTGAMING_H
