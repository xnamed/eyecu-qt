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
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return HANGMANGAME_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() {return this;}

	//IGame
	virtual void gameInfo(IGameInfo *AGameInfo);
	virtual bool isSupported(const Jid &AStreamJid, const Jid &AContactJid);
	virtual void startGame(const QString &AThread, int AType);
protected:
	void registerDiscoFeatures();
private:
	IServiceDiscovery *FDiscovery;
};

#endif // HANGMANGAME_H
