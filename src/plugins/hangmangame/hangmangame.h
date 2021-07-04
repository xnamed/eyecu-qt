#ifndef HANGMANGAME_H
#define HANGMANGAME_H

#include <interfaces/igame.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iinstantgaming.h>
#include <interfaces/iservicediscovery.h>
class GameWindow;

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

	enum Role {
		Guesser,
		PuzzleSetter
	};

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
	virtual void startGame(const QString &AThread, const Jid &AStreamJid, const Jid &AContactJid, int AType);
	virtual void turn(const QString &AThread, const QDomElement &AElement, const QString &AMessage);

	//HangmanGame
	virtual bool sendPuzzle(const QString &AThread, const QStringList &AWords, const QString &AMessage);
	virtual bool sendGuess(const QString &AThread, const QString &ALetter, const QString &AMessage);
	virtual bool isLetterAlreadyUsed(const QString &AThread, const QString &ALetter);
protected:
	void registerDiscoFeatures();
protected:
	void guess();
	void guessed();
	GameWindow *getGameWindow(const QString &AThread);
private:
	QHash<QString, QStringList> FWords;
	QHash<QString, QList<int> > FPuzzle; //length of each word
	QHash<QString, QStringList> FGuess;  //letters were played
	QHash<QString, QHash<int, QString> > FGuessed; //letters were guessed
	QHash<QString, int> FRole;
	QHash<QString, int> FChance;
private:
	QMap<QString, GameWindow *> FGameWindow;
private:
	IServiceDiscovery *FDiscovery;
	IInstantGaming *FInstantGaming;
};

#endif // HANGMANGAME_H
