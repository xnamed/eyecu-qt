#include "hangmangame.h"
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/widgetmanager.h>
#include "gamewindow.h"
#include <QDebug>

HangmanGame::HangmanGame():
	FDiscovery(NULL),
	FInstantGaming(NULL)
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

	plugin = APluginManager->pluginInterface("IInstantGaming").value(0,NULL);
	if (plugin)
		FInstantGaming = qobject_cast<IInstantGaming *>(plugin->instance());

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
	AGameInfo->icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HANGMAN_GAME);
	AGameInfo->var = NS_HANGMAN_GAME;
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

GameWindow *HangmanGame::getGameWindow(const QString &AThread)
{
	GameWindow *window = FGameWindow.value(AThread);
	if (window == NULL)
	{
		window = new GameWindow(this, AThread);
		//connect(window,SIGNAL(destroyed(QObject*)),SLOT());
		FGameWindow.insert(AThread,window);
	}
	return window;
}

bool HangmanGame::isSupported(const Jid &AStreamJid, const Jid &AContactJid)
{
	return !FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
			FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_HANGMAN_GAME);
}

void HangmanGame::startGame(const QString &AThread, const Jid &AStreamJid, const Jid &AContactJid, int AType)
{
	GameWindow *window = getGameWindow(AThread);
	WidgetManager::showActivateRaiseWindow(window);

	IInstantGame instantGame = FInstantGaming->findGame(AThread);
	/* change to the opposite of the role the inviting player had
	 * in the previous match with that match ID */
	if (FRole.contains(AThread))
		FRole[AThread] = FRole.value(AThread)==Guesser ? PuzzleSetter
													   : Guesser;
	else
		FRole[AThread] = instantGame.player==IInstantGaming::Initiator ? PuzzleSetter
																	   : Guesser;

	if (AType == IInstantGaming::New)
	{}
	else if (AType == IInstantGaming::Renew)
	{}
	else if (AType == IInstantGaming::Adjourned)
	{}
	else if (AType == IInstantGaming::Constructed)
	{}

	window->switchRole(FRole.value(AThread));
	window->show();
}

void HangmanGame::turn(const QString &AThread, const QDomElement &AElement, const QString &AMessage)
{
	GameWindow *window = getGameWindow(AThread);
	QDomNode node = AElement.firstChild();
	if (!node.isNull())
	{
		QDomElement childElem = node.toElement();
		if (childElem.tagName() == "ordered" && FRole.value(AThread) == Guesser
				&& !FPuzzle.contains(AThread))
		{
			QDomElement wordElem = childElem.firstChildElement("word");
			bool accept;
			int puzzleLen =0;
			while (!wordElem.isNull())
			{
				int puzzle = wordElem.text().toInt();
				if (puzzle > 0)
				{
					accept = true;
					FPuzzle[AThread].append(puzzle);
					puzzleLen += puzzle;
				}
				wordElem = wordElem.nextSiblingElement("word");
			}
			if (accept)
				FChance[AThread] = puzzleLen > 9 ? puzzleLen - 9
												 : 0;
				window->setPuzzle(FPuzzle.value(AThread),AMessage);
				window->updateGallows(9 - (FChance.value(AThread) +9));
		}
		else if (childElem.tagName() == "guess"  && FRole.value(AThread) == PuzzleSetter)
		{
			QString letter = childElem.text().toLower();
			if (!isLetterAlreadyUsed(AThread,letter) && letter.length() == 1)
			{
				FGuess[AThread].append(letter);

				QString words = FWords[AThread].join("");
				QRegExp rx(QRegExp::escape(letter));
				int pos =0;
				QStringList matched;

				while ((pos = rx.indexIn(words, pos)) != -1)
				{
					pos += rx.matchedLength();
					matched.append(QString::number(pos));

					if (!FGuessed.value(AThread).contains(pos))
						FGuessed[AThread].insert(pos,letter);
				}
				qDebug() << "FChance[AThread]" << FChance[AThread];

				QDomDocument doc;
				QDomElement turnElem = doc.createElementNS(NS_HANGMAN_GAME,"guessed").toElement();
				turnElem.appendChild(doc.createTextNode(QString(matched.join(","))));
				turnElem.setAttribute("letter", letter);

				if (FInstantGaming->sendTurn(AThread,QString(""),turnElem))
				{
					if (matched.isEmpty())
					{
						FChance[AThread] -= 1;
						window->updateGallows(9 - (FChance.value(AThread) +9));
					}
					else
					{
						window->updatePuzzle(FPuzzle.value(AThread),FGuessed.value(AThread));
					}

					window->updateKeys(FGuess.value(AThread),FGuessed.value(AThread).values());
				}
			}
		}
		else if (childElem.tagName() == "guessed"  && FRole.value(AThread) == Guesser)
		{
			QString letter = childElem.attribute("letter").toLower();
			QString guessed = childElem.text().toLower();
			if (!guessed.isEmpty())
			{
				QRegExp rx("(\\d+)");
				int pos = 0;

				while ((pos = rx.indexIn(guessed, pos)) != -1)
				{
					FGuessed[AThread].insert(rx.cap(1).toInt(),letter);
					pos += rx.matchedLength();
				}

				window->updatePuzzle(FPuzzle.value(AThread),FGuessed.value(AThread));
			}
			else
			{
				FChance[AThread] -= 1;
				window->updateGallows(9 - (FChance.value(AThread) +9));
			}

			window->updateKeys(FGuess.value(AThread),FGuessed.value(AThread).values());
		}
	}
}

bool HangmanGame::sendPuzzle(const QString &AThread, const QStringList &AWords, const QString &AMessage)
{
	QDomDocument doc;
	QDomElement turnElem = doc.createElementNS(NS_HANGMAN_GAME,"ordered").toElement();

	int puzzleLen =0;
	foreach (const QString &word, AWords)
	{
		puzzleLen += word.length();
		FPuzzle[AThread].append(word.length());
		turnElem.appendChild(doc.createElement("word"))
				.appendChild(doc.createTextNode(QString::number(word.length())));
	}

	FChance[AThread] = puzzleLen > 9 ? puzzleLen - 9
									 : 0;
	FWords[AThread] = AWords;
	return FInstantGaming->sendTurn(AThread,AMessage,turnElem);
}

bool HangmanGame::sendGuess(const QString &AThread, const QString &ALetter, const QString &AMessage)
{
	QDomDocument doc;
	QDomElement turnElem = doc.createElementNS(NS_HANGMAN_GAME,"guess").toElement();
	turnElem.appendChild(doc.createTextNode(ALetter));

	FGuess[AThread].append(ALetter);
	return FInstantGaming->sendTurn(AThread,AMessage,turnElem);
}

bool HangmanGame::isLetterAlreadyUsed(const QString &AThread, const QString &ALetter)
{
	return FGuess[AThread].contains(ALetter);
}
