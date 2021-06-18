#include "hangmangame.h"
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>

HangmanGame::HangmanGame():
	FDiscovery(NULL)
{
}

HangmanGame::~HangmanGame()
{}

void HangmanGame::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Hangman Game");
	APluginInfo->description = tr("Guess letters one at a time to solve the word puzzle.");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(INSTANTGAMING_UUID);
}

bool HangmanGame::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	return true;
}

bool HangmanGame::initObjects()
{
	if (FDiscovery)
		registerDiscoFeatures();

	return true;
}

bool HangmanGame::initSettings()
{
	return true;
}

void HangmanGame::gameInfo(IGameInfo *AGameInfo)
{
	AGameInfo->name = tr("Hangman");
	AGameInfo->category = tr("guessing");
}

void HangmanGame::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.var = NS_HANGMAN_GAME;
	dfeature.active = true;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HANGMAN_GAME);
	dfeature.name = tr("Hangman Game");
	dfeature.description = tr("Supports Hangman Game");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool HangmanGame::isSupported(const Jid &AStreamJid, const Jid &AContactJid)
{
	return !FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
			FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_HANGMAN_GAME);
}
