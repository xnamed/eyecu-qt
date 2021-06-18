#ifndef HANGMANGAME_H
#define HANGMANGAME_H

#include <interfaces/igame.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iinstantgaming.h>
#include <interfaces/iservicediscovery.h>

#define HANGMANGAME_UUID  "6BCB485A-4672-4B8F-AF9B-C9A5620C3750"

class HangmanGame:
		public QObject,
		public IPlugin,
		public IGame
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IGame)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IGame")
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

	//IGame
	void gameInfo(IGameInfo *AGameInfo);
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid);
protected:
	void registerDiscoFeatures();
private:
	IServiceDiscovery *FDiscovery;
};

#endif // HANGMANGAME_H
