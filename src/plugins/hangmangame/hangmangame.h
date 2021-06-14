#ifndef HANGMANGAME_H
#define HANGMANGAME_H

#include <interfaces/ihangmangame.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iinstantgaming.h>
#include <interfaces/iservicediscovery.h>

class HangmanGame:
		public QObject,
		public IPlugin,
		public IHangmanGame
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IHangmanGame)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IHangmanGame")
#endif

public:
	HangmanGame();
	~HangmanGame();

	//IPlugin
	QObject *instance() {return this;}
	QUuid pluginUuid() const {return HANGMANGAME_UUID;}
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin() {return this;}

protected:
	void registerDiscoFeatures();
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid);

private:
	IServiceDiscovery *FDiscovery;
};

#endif // HANGMANGAME_H
