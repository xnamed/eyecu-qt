#include "hangmangame.h"


HangmanGame::HangmanGame()
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
}

bool HangmanGame::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	return true;
}

bool HangmanGame::initObjects()
{
	return true;
}

bool HangmanGame::initSettings()
{
	return true;
}
