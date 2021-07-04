#include "instantgaming.h"
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/xmpperrors.h>
#include <definitions/stanzahandlerorders.h>
#include <utils/message.h>
#include <definitions/toolbargroups.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/notificationtypes.h>
#include <utils/qt4qt5compat.h>

#include "gameselect.h"
#include <utils/widgetmanager.h>
#include <QDebug>

#define SHC_INSTANTGAMING_INVITE         "/message/invite[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_DECLINE         "/message/decline[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_JOIN         "/message/join[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_TURN         "/message/turn[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_SAVE         "/message/save[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_SAVED         "/message/saved[@xmlns='" NS_INSTANTGAMING "']"
#define SHC_INSTANTGAMING_TERMINATE         "/message/terminate[@xmlns='" NS_INSTANTGAMING "']"

#define ADR_STREAM_JID   Action::DR_StreamJid
#define ADR_CONTACT_JID  Action::DR_Parametr1

#define INSTANTGAMING_INVITE_NEW             "new"
#define INSTANTGAMING_INVITE_RENEW           "renew"
#define INSTANTGAMING_INVITE_ADJOURNED       "adjourned"
#define INSTANTGAMING_INVITE_CONSTRUCTED     "constructed"

#define INSTANTGAMING_REASON_WON             "won"
#define INSTANTGAMING_REASON_LOST            "lost"
#define INSTANTGAMING_REASON_DRAW            "draw"
#define INSTANTGAMING_REASON_RESIGN          "resign"
#define INSTANTGAMING_REASON_ADJOURN         "adjourn"
#define INSTANTGAMING_REASON_CHEATING        "cheating"

InstantGaming::InstantGaming():
	FXmppStreamManager(NULL),
	FStanzaProcessor(NULL),
	FDiscovery(NULL),
	FDataForms(NULL),
	FMessageWidgets(NULL),
	FNotifications(NULL)
{
}

InstantGaming::~InstantGaming()
{
	FStanzaProcessor->removeStanzaHandle(FSHIGames);
}

void InstantGaming::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Instant Gaming");
	APluginInfo->description = tr("Serverless instant gaming in a one-to-one context.");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(DATAFORMS_UUID);
	APluginInfo->dependences.append(SERVICEDISCOVERY_UUID);
}

bool InstantGaming::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpneed(IXmppStream *)),SLOT(onXmppStreamOpened(IXmppStream*)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onXmppStreamClosed(IXmppStream*)));
		}
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IDataForms").value(0,NULL);
	if (plugin)
		FDataForms = qobject_cast<IDataForms *>(plugin->instance());

	QList<IPlugin *> games = APluginManager->pluginInterface("IGame");
	for (QList<IPlugin *>::ConstIterator it=games.constBegin(); it!=games.constEnd(); ++it)
	{
		FGames.insert((*it)->pluginUuid(),qobject_cast<IGame *>((*it)->instance()));
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(),SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget*)),SLOT(onToolBarWidgetCreated(IMessageToolBarWidget*)));
		}
	}

	plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
		{
			connect(FNotifications->instance(),SIGNAL(notificationActivated(int)),SLOT(onNotificationActivated(int)));
			connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)),SLOT(onNotificationRemoved(int)));
		}
	}

	return true;
}

bool InstantGaming::initObjects()
{
	if (FDiscovery)
		registerDiscoFeatures();

	if (FNotifications)
	{
		INotificationType inviteType; //Invitation
		inviteType.order = NTO_GAME_INVITE_NOTIFY;
		inviteType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GAME_INVITE);
		inviteType.title = tr("When receiving an invitation to play game");
		inviteType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|INotification::PopupWindow|INotification::SoundPlay|INotification::AutoActivate;
		inviteType.kindDefs = inviteType.kindMask & ~ (INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_GAME_INVITE,inviteType);

		INotificationType gameplayType; //Game Play
		gameplayType.order = NTO_GAME_PLAY_NOTIFY;
		gameplayType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GAME_PLAY);
		gameplayType.title = tr("");
		gameplayType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|INotification::PopupWindow|INotification::SoundPlay|INotification::AutoActivate;
		gameplayType.kindDefs = gameplayType.kindMask & ~ (INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_GAME_PLAY,gameplayType);

		INotificationType gamestateType; //Joining, Saving, Termination
		gamestateType.order = NTO_GAME_STATE_NOTIFY;
		gamestateType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_INSTANTGAMING);
		gamestateType.title = tr("");
		gamestateType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|INotification::PopupWindow|INotification::SoundPlay|INotification::AutoActivate;
		gamestateType.kindDefs = gameplayType.kindMask & ~ (INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_GAME_STATE,gamestateType);
	}

	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_INSTANTGAMING;
		shandle.direction = IStanzaHandle::DirectionIn;
		//shandle.streamJid = AXmppStream->streamJid();
		shandle.conditions.append(SHC_INSTANTGAMING_INVITE);
		shandle.conditions.append(SHC_INSTANTGAMING_DECLINE);
		shandle.conditions.append(SHC_INSTANTGAMING_JOIN);
		shandle.conditions.append(SHC_INSTANTGAMING_TURN);
		shandle.conditions.append(SHC_INSTANTGAMING_SAVE);
		shandle.conditions.append(SHC_INSTANTGAMING_SAVED);
		shandle.conditions.append(SHC_INSTANTGAMING_TERMINATE);
		FSHIGames = FStanzaProcessor->insertStanzaHandle(shandle);
	}

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
	return !FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
			FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_INSTANTGAMING);
}

bool InstantGaming::sendInvitation(const QString &AThread, IDataForm &AForm, const QString &AReason, int AType)
{
	qDebug() << "sendInvitation:" << AThread;
	IInstantGame game = FActiveGames.value(AThread);
	if (FStanzaProcessor && FDataForms)
	{
		Stanza invite(STANZA_KIND_MESSAGE);
		invite.setTo(game.contactJid.full());
		invite.addElement("thread").appendChild(invite.createTextNode(AThread));

		QDomElement inviteElem = invite.addElement("invite", NS_INSTANTGAMING).toElement();

		inviteElem.setAttribute("type", inviteTypeToString(AType));

		if (!AReason.isEmpty())
		{
			inviteElem.appendChild(invite.createElement("reason")).appendChild(invite.createTextNode(AReason));
		}

		QDomElement gameElem = inviteElem.appendChild(invite.createElement("game")).toElement();
		gameElem.setAttribute("var", game.var);

		if (AType == IInstantGaming::Adjourned || AType == IInstantGaming::Constructed)
			FDataForms->xmlForm(AForm,gameElem);

		if (FStanzaProcessor->sendStanzaOut(game.streamJid,invite))
			return true;
	}
	return false;
}

bool InstantGaming::declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread)
{
	if (FStanzaProcessor)
	{
		Stanza decline(STANZA_KIND_MESSAGE);
		decline.setTo(AContactJid.full());
		decline.addElement("thread").appendChild(decline.createTextNode(AThread));

		QDomElement declineElem = decline.addElement("decline", NS_INSTANTGAMING).toElement();
		if (!AReason.isEmpty())
		{
			declineElem.appendChild(decline.createElement("reason")).appendChild(decline.createTextNode(AReason));
		}
		if (FStanzaProcessor->sendStanzaOut(AStreamJid,decline))
			return true;
	}
	return false;
}

bool InstantGaming::joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage, const QString &AThread)
{
	qDebug() << "joinGame:" << AThread;
	if (FStanzaProcessor)
	{
		Stanza join(STANZA_KIND_MESSAGE);
		join.setTo(AContactJid.full());
		join.addElement("thread").appendChild(join.createTextNode(AThread));
		join.addElement("join", NS_INSTANTGAMING);
		if (FStanzaProcessor->sendStanzaOut(AStreamJid,join))
			return true;
	}
	return false;
}

bool InstantGaming::rejectGame(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (FStanzaProcessor)
	{
		Stanza error = FStanzaProcessor->makeReplyError(AStanza,XmppStanzaError::EC_NOT_ALLOWED);
		FStanzaProcessor->sendStanzaOut(AStreamJid,error);
		return true;
	}
	return false;
}

bool InstantGaming::sendTurn(const QString &AThread, const QString &AMessage, const QDomElement &AElement)
{
	qDebug() << "sendTurn:" << AThread;
	IInstantGame game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza turn(STANZA_KIND_MESSAGE);
		turn.setTo(game.contactJid.full()).setType(MESSAGE_TYPE_CHAT);
		turn.addElement("thread").appendChild(turn.createTextNode(AThread));

		QDomElement turnElem = turn.addElement("turn", NS_INSTANTGAMING).toElement();
		turnElem.appendChild(AElement);

		if (AMessage.isEmpty())
		{}
		if (FStanzaProcessor->sendStanzaOut(game.streamJid,turn))
			return true;
	}
	return false;
}

bool InstantGaming::sendSave(const QString &AThread)
{
	qDebug() << "sendSave:" << AThread;
	IInstantGame game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza save(STANZA_KIND_MESSAGE);
		save.setTo(game.contactJid.full());
		save.addElement("thread").appendChild(save.createTextNode(AThread));
		save.addElement("save",NS_INSTANTGAMING);
		if (FStanzaProcessor->sendStanzaOut(game.streamJid,save))
			return true;
	}
	return false;
}

bool InstantGaming::sendSaved(const QString &AThread)
{
	qDebug() << "sendSaved:" << AThread;
	IInstantGame game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza saved(STANZA_KIND_MESSAGE);
		saved.setTo(game.contactJid.full());
		saved.addElement("thread").appendChild(saved.createTextNode(AThread));
		saved.addElement("saved",NS_INSTANTGAMING);
		if (FStanzaProcessor->sendStanzaOut(game.streamJid,saved))
			return true;
	}
	return false;
}

bool InstantGaming::terminateGame(const QString &AThread, const QString &AMessage, int AReason)
{
	qDebug() << "terminateGame:" << terminateReasonToString(AReason);
	IInstantGame game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza terminate(STANZA_KIND_MESSAGE);
		terminate.setTo(game.contactJid.full());
		terminate.addElement("thread").appendChild(terminate.createTextNode(AThread));
		terminate.addElement("terminate", NS_INSTANTGAMING);
		terminate.setAttribute("reason", terminateReasonToString(AReason));

		if (FStanzaProcessor->sendStanzaOut(game.streamJid,terminate))
			return true;
	}
	return false;
}

void InstantGaming::onXmppStreamOpened(IXmppStream *AXmppStream)
{}

void InstantGaming::onXmppStreamClosed(IXmppStream *AXmppStream)
{}

bool InstantGaming::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (AHandleId == FSHIGames && FDataForms)
	{
		AAccept = true;
		Message message(AStanza);
		QString threadId = AStanza.firstElement("thread").text();

		if (threadId.isEmpty())
			return false;

		IInstantGame instantGame = FActiveGames.value(threadId);

		//Invitation / Game Loading
		if (!AStanza.firstElement("invite",NS_INSTANTGAMING).isNull())
		{
			qDebug() << "Game invitation from:" << AStanza.fromJid().bare();
			QDomElement inviteElem = AStanza.firstElement("invite",NS_INSTANTGAMING);
			instantGame.type = inviteTypeToCode(inviteElem.attribute("type"));
			QString reason = inviteElem.firstChildElement("reason").text();

			QDomElement gameElem = inviteElem.firstChildElement("game");
			instantGame.var = gameElem.attribute("var");

			if (FDiscovery && !instantGame.var.isEmpty() &&
					FDiscovery->discoFeatures().contains(instantGame.var) &&
					FGames.contains(findGameUuid(instantGame.var)))
			{
				QDomElement formElem = Stanza::findElement(gameElem,"x",NS_JABBER_DATA);
				//Game Loading
				if (!formElem.isNull())
					IDataForm form = FDataForms->dataForm(formElem);
				if (!FActiveGames.contains(threadId))
				{
					instantGame.streamJid = AStreamJid;
					instantGame.contactJid = AStanza.fromJid();
					instantGame.threadId = threadId;
					instantGame.uuid = findGameUuid(instantGame.var);
					instantGame.player = Opponent;
					instantGame.status = Pending;
					FActiveGames.insert(threadId,instantGame);
				}
				else if (instantGame.status == Active)
				{
					return false;
				}
				notifyGame(threadId,Invitation,reason,instantGame.type);
			}
			else
			{
				//Not supported
			}
		}
		else if (!AStanza.firstElement("decline",NS_INSTANTGAMING).isNull())
		{
			QDomElement declElem = AStanza.firstElement("decline",NS_INSTANTGAMING);
			QString reason = declElem.firstChildElement("reason").text();
			if (instantGame.player == Initiator && instantGame.status == Pending)
			{
				notifyGame(threadId,Declined,reason,0);
				FActiveGames.remove(threadId);
			}
		}
		//Joining a Game
		else if (!AStanza.firstElement("join",NS_INSTANTGAMING).isNull())
		{
			qDebug() << "Game join from:" << AStanza.fromJid().bare();
			//The initiator might refuse the join
			if (AStanza.isError())
			{}
			//<body/> element is OPTIONAL.
			else if (instantGame.player == Initiator && instantGame.status == Pending)
			{
				startGame(instantGame);
				notifyGame(threadId,Joined,message.body(),0);
			}
		}
		/* Game Play
		 * A receipt for each message MAY be requested in accordance
		 * with Message Delivery Receipts (XEP-0184).
		 */
		else if (!AStanza.firstElement("turn",NS_INSTANTGAMING).isNull())
		{
			QDomElement turnElem = AStanza.firstElement("turn",NS_INSTANTGAMING);
			setTurn(threadId,turnElem,message.body());
			notifyGame(threadId,GamePlay,message.body(),0);
		}
		//Game Saving
		else if (!AStanza.firstElement("save",NS_INSTANTGAMING).isNull())
		{
			if (AStanza.isError())
			{
				notifyGame(threadId,Saving,"",0);
			}
			else
			{
				notifyGame(threadId,Saving,"",1);
			}
		}
		else if (!AStanza.firstElement("saved",NS_INSTANTGAMING).isNull())
		{
			notifyGame(threadId,Saved,"",0);
		}
		//Termination
		else if (!AStanza.firstElement("terminate",NS_INSTANTGAMING).isNull())
		{
			if (instantGame.status == Active)
			{
				QDomElement termElem = AStanza.firstElement("terminate",NS_INSTANTGAMING);
				int reason = terminateReasonToCode(termElem.attribute("reason"));
				notifyGame(threadId,Termination,message.body(),reason);
				instantGame.status = Inactive;
			}
		}
		return true;
	}
	return false;
}

void InstantGaming::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
	if (chatWindow)
		connect(AWidget->messageWindow()->address()->instance(),SIGNAL(addressChanged(Jid,Jid)),SLOT(onToolBarWidgetAddressChanged(Jid,Jid)));
	connect(AWidget->instance(),SIGNAL(destroyed(QObject*)),SLOT(onToolBarWidgetDestroyed(QObject*)));

	updateToolBarAction(AWidget);
}

void InstantGaming::onToolBarWidgetAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore); Q_UNUSED(AContactBefore);
	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	if (address)
	{
		foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
			if (widget->messageWindow()->address() == address)
				updateToolBarAction(widget);
	}
}

void InstantGaming::onToolBarWidgetDestroyed(QObject *AObject)
{
	foreach (IMessageToolBarWidget *widget, FToolBarActions.keys())
		if (qobject_cast<QObject *>(widget->instance()) == AObject)
			FToolBarActions.remove(widget);
}

void InstantGaming::onNotificationActivated(int ANotifyId)
{
	if (FGameNotify.contains(ANotifyId))
	{
		GameState gameState = FGameNotify.take(ANotifyId);
		IInstantGame game = FActiveGames.value(gameState.threadId);
		IGame *plugin = FGames.value(findGameUuid(game.var));
		if (plugin)
		{
			if (gameState.state == Invitation)
			{
				IGameInfo gameInfo;
				plugin->gameInfo(&gameInfo);

				QString contactName = FNotifications->contactName(game.streamJid,game.contactJid);

				QString msg;
				switch (gameState.type)
				{
				case New:
					msg = tr("%1 invites you to play %2 game").arg(contactName,gameInfo.name);
					msg += QString("<br>%1<br>").arg(HTML_ESCAPE(gameState.text));
					msg += tr("Do you want to join the game?");
					break;
				case Renew:
					msg = tr("%1 invites you to recreate the game from the beginning").arg(contactName);
					msg += QString("<br>%1<br>").arg(HTML_ESCAPE(gameState.text));
					msg += tr("Do you want to renew the game?");
					break;
				case Adjourned:
					msg = tr("%1 invites you to resume a saved %2 game").arg(contactName,gameInfo.name);
					msg += QString("<br>%1<br>").arg(HTML_ESCAPE(gameState.text));
					msg += tr("Do you want to resume the game?");
					break;
				case Constructed:
					msg = tr("%1 invites you to resume a constructed %2 game").arg(contactName,gameInfo.name);
					msg += QString("<br>%1<br>").arg(HTML_ESCAPE(gameState.text));
					msg += tr("Do you want to resume the game?");
					break;
				}

				QMessageBox *inviteDialog = new QMessageBox(QMessageBox::Question,tr("Invitation to game"),
															msg,
															QMessageBox::Yes|QMessageBox::No);
				inviteDialog->setAttribute(Qt::WA_DeleteOnClose,true);
				inviteDialog->setEscapeButton(QMessageBox::No);
				inviteDialog->setModal(false);
				connect(inviteDialog,SIGNAL(finished(int)),SLOT(onInviteDialogFinished(int)));

				FInviteDialogs.insert(inviteDialog,gameState.threadId);
				inviteDialog->show();
			}
			else
			{
				// Show game window
			}
		}
	}
}

void InstantGaming::onNotificationRemoved(int ANotifyId)
{
	FGameNotify.remove(ANotifyId);
}

void InstantGaming::onInviteDialogFinished(int AResult)
{
	QMessageBox *inviteDialog = qobject_cast<QMessageBox *>(sender());
	if (inviteDialog)
	{
		IInstantGame game = FActiveGames.value(FInviteDialogs.take(inviteDialog));

		if (AResult == QMessageBox::Yes)
		{
			if (joinGame(game.streamJid,game.contactJid,"",game.threadId))
				startGame(game);
		}
		else
		{
			declineInvitation(game.streamJid,game.contactJid,"",game.threadId);
			FActiveGames.remove(game.threadId);
		}
	}
}

void InstantGaming::updateToolBarAction(IMessageToolBarWidget *AWidget)
{
	Action *gameAction = FToolBarActions.value(AWidget);
	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());

	if (chatWindow != NULL)
	{
		if (gameAction == NULL)
		{
			gameAction = new Action(AWidget->toolBarChanger()->toolBar());
			gameAction->setIcon(RSR_STORAGE_MENUICONS, MNI_INSTANTGAMING);
			gameAction->setText(tr("Play Game"));
			//gameAction->setShortcutId();
			gameAction->setData(ADR_STREAM_JID,chatWindow->streamJid().full());
			gameAction->setData(ADR_CONTACT_JID,chatWindow->contactJid().full());
			connect(gameAction,SIGNAL(triggered(bool)),SLOT(onShowGameSelectorByToolBarAction(bool)));
			AWidget->toolBarChanger()->insertAction(gameAction,TBG_MWTBW_INSTANTGAMING);
			FToolBarActions.insert(AWidget,gameAction);
		}
		gameAction->setEnabled(isSupported(chatWindow->streamJid(),chatWindow->contactJid()));
	}
}

void InstantGaming::onShowGameSelectorByToolBarAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();
		showGameSelector(streamJid,contactJid);
	}
}

void InstantGaming::showGameSelector(const Jid &AStreamJid, const Jid &AContactJid)
{
	GameSelect *select = new GameSelect(this,AStreamJid,AContactJid,FGames);
	WidgetManager::showActivateRaiseWindow(select);
}

void InstantGaming::selectGame(IInstantGame &AGame, const QString &AMessage, int AType)
{
	qDebug() << "Selected game type:" << inviteTypeToString(AGame.type);
	IDataForm form;
	if (AType == New)
	{
		int id;
		for (id=qrand(); FActiveGames.keys().contains(QString::number(id)); id++);
		AGame.threadId = QString::number(id);
	}
	else if (AType == Renew)
	{
		/* An invitation of type "renew" SHOULD contain the same match ID
		 * It MUST only be send when the previous match with the same match ID has been terminated.*/
	}
	else
	{
		/* Game Loading
		 * An XMPP entity that wishes to resume a saved game has to send an invitation
		 * of "type" 'adjourned' to the same opponent it began playing with.
		 *
		 * It MAY also resume the game with another opponent,
		 * but then it MUST use the "type" 'constructed' and a new match ID.*/
	}

	FActiveGames.insert(AGame.threadId,AGame);
	if (sendInvitation(AGame.threadId,form,AMessage,AType))
	{
		qDebug() << "Invitation sent:" << AGame.threadId;
	}
}

IInstantGame InstantGaming::findGame(const QString &AThread) const
{
	return FActiveGames.value(AThread);
}

// Start when the opponent accepts the invitation
void InstantGaming::startGame(IInstantGame &AGame)
{
	IGame *plugin = FGames.value(AGame.uuid);
	if (plugin)
	{
		AGame.status = Active;
		plugin->startGame(AGame.threadId, AGame.streamJid, AGame.contactJid, AGame.type);
		qDebug() << "Game started:" << AGame.threadId;
	}
}

void InstantGaming::setTurn(const QString &AThread, const QDomElement &ATurnElem, const QString &AMessage)
{
	if (FActiveGames.contains(AThread))
	{
		IGame *plugin = FGames.value(FActiveGames.value(AThread).uuid);
		if (plugin)
		{
			plugin->turn(AThread,ATurnElem,AMessage);
		}
	}
}

void InstantGaming::notifyGame(const QString &AThread, int AGameState, const QString &AMessage, int AType)
{
	qDebug() << "notifyGame:" << AThread;
	IInstantGame instantGame = FActiveGames.value(AThread);

	GameState gameState;
	gameState.threadId = AThread;
	gameState.state = AGameState;
	gameState.type = AType;
	gameState.text = AMessage;

	QString contactName = instantGame.contactJid.uFull();

	IMessageChatWindow *chatWindow = FMessageWidgets!=NULL ? FMessageWidgets->findChatWindow(instantGame.streamJid,instantGame.contactJid) : NULL;
	if (chatWindow)
	{
		if (FNotifications)
			contactName = FNotifications->contactName(instantGame.streamJid,instantGame.contactJid);
	}

	IGame *plugin = FGames.value(instantGame.uuid);
	if (FNotifications && plugin)
	{
		qDebug() << "found game plugin:" << findGameUuid(instantGame.var);
		IGameInfo gameInfo;
		plugin->gameInfo(&gameInfo);

		QString message, tooltip;
		INotification notify;

		if (AGameState != Invitation && AGameState != GamePlay)
		{
			notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_GAME_STATE);
			notify.typeId = NNT_GAME_STATE;
		}
		notify.data.insert(NDR_ICON,gameInfo.icon);
		notify.data.insert(NDR_POPUP_TITLE,FNotifications->contactName(instantGame.streamJid,instantGame.contactJid));
		notify.data.insert(NDR_STREAM_JID,instantGame.streamJid.full());
		notify.data.insert(NDR_CONTACT_JID,instantGame.contactJid.full());
		notify.data.insert(NDR_POPUP_IMAGE,FNotifications->contactAvatar(instantGame.contactJid));

		switch (AGameState)
		{
		case Invitation:
			notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_GAME_INVITE);
			notify.typeId = NNT_GAME_INVITE;
			notify.data.insert(NDR_POPUP_CAPTION, tr("Invitation received"));
			switch (AType)
			{
			case New:
				tooltip = tr("%1 invites you to play %2 game").arg(contactName,gameInfo.name);
				break;
			case Renew:
				tooltip = tr("%1 invites you to recreate the game from the beginning").arg(contactName);
				break;
			case Adjourned:
				tooltip = tr("%1 invites you to resume a saved game").arg(contactName);
				break;
			case Constructed:
				tooltip = tr("%1 invites you to resume a constructed game").arg(contactName);
				break;
			}
			break;
		case Declined:
			notify.data.insert(NDR_POPUP_CAPTION,tr("Invitation declined"));
			tooltip = AMessage.isEmpty() ? tr("")
										 : AMessage;
			break;
		case Joined:
			notify.data.insert(NDR_POPUP_CAPTION,tr("Game joining"));
			tooltip = tr("%1 has joined the game").arg(contactName);
			break;
		case GamePlay:
			notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_GAME_PLAY);
			notify.typeId = NNT_GAME_PLAY;
			notify.data.insert(NDR_POPUP_CAPTION,tr("Game play"));
			tooltip = AMessage.isEmpty() ? tr("")
										 : AMessage;
			break;
		case Saving:
			notify.data.insert(NDR_POPUP_CAPTION,tr("Game saving request"));
			tooltip = tr("%1 has issued a mutual game saving process").arg(contactName);
			break;
		case Saved:
			 //independently or mutually?
			if (FSaveRequests.contains(AThread))
			{
				message = AType==1 ? tr("Game saved")
								   : tr("Game saving error");
				tooltip = AType==1 ? tr("%1 has confirmed a successful saving process")
								   : tr("%1 failed to save a mutual game");
			}
			else
			{
				message = tr("Game saving");
				tooltip = tr("%1 has saved the game independently");
			}
			notify.data.insert(NDR_POPUP_CAPTION,message);
			notify.data.insert(NDR_POPUP_TEXT,tooltip.arg(contactName));
			break;
		case Termination:
			notify.data.insert(NDR_POPUP_CAPTION,tr("Game terminated"));
			switch (AType)
			{
			case Won:
				tooltip = tr("%1 has lost the game").arg(contactName);
			case Lost:
				tooltip = tr("%1 has won the game").arg(contactName);
			case Draw:
				tooltip = tr("The game is over and nobody has won").arg(contactName);
			case Resign:
				tooltip = tr("%1 has capitulated and quit the game").arg(contactName);
			case Adjourn:
				tooltip = tr("%1 has adjourned the game. The game is not over and might be resumed at a later time").arg(contactName);
			case Cheating:
				tooltip = tr("You cheat or have submitted an illegal game move").arg(contactName);
				break;
			}
			break;
		default:
			notify.kinds = 0;
		}

		if (notify.kinds > 0)
		{
			notify.data.insert(NDR_POPUP_TEXT,tooltip);
			FGameNotify.insert(FNotifications->appendNotification(notify),gameState);
		}

		if (!tooltip.isEmpty())
		{
			if (chatWindow)
				showStatusEvent(chatWindow->viewWidget(),tooltip);
		}
		emit gameStateChanged(AThread,AGameState,AMessage,AType);
	}
}

void InstantGaming::showStatusEvent(IMessageViewWidget *AView, const QString &AHtml) const
{
	qDebug() << "showStatusEvent";
	if (AView!=NULL && !AHtml.isEmpty())
	{
		IMessageStyleContentOptions options;
		options.kind = IMessageStyleContentOptions::KindStatus;
		options.type = IMessageStyleContentOptions::TypeEvent;
		options.direction = IMessageStyleContentOptions::DirectionIn;
		options.time = QDateTime::currentDateTime();
		AView->appendHtml(AHtml,options);
	}
}

int InstantGaming::inviteTypeToCode(const QString &AType) const
{
	int type;
	if (AType == INSTANTGAMING_INVITE_NEW)
		type = IInstantGaming::New;
	else if (AType == INSTANTGAMING_INVITE_RENEW)
		type = IInstantGaming::Renew;
	else if (AType == INSTANTGAMING_INVITE_ADJOURNED)
		type = IInstantGaming::Adjourned;
	else if (AType == INSTANTGAMING_INVITE_CONSTRUCTED)
		type = IInstantGaming::Constructed;
	return type;
}

QString InstantGaming::inviteTypeToString(int AType) const
{
	QString type;
	switch (AType) {
	case IInstantGaming::New:
		type = INSTANTGAMING_INVITE_NEW;
		break;
	case IInstantGaming::Renew:
		type = INSTANTGAMING_INVITE_RENEW;
		break;
	case IInstantGaming::Adjourned:
		type = INSTANTGAMING_INVITE_ADJOURNED;
		break;
	case IInstantGaming::Constructed:
		type = INSTANTGAMING_INVITE_CONSTRUCTED;
		break;
	default:
		break;
	}
	return type;
}

int InstantGaming::terminateReasonToCode(const QString &AReason) const
{
	int reason;
	if (AReason == INSTANTGAMING_REASON_WON)
		reason = IInstantGaming::Won;
	else if (AReason == INSTANTGAMING_REASON_LOST)
		reason = IInstantGaming::Lost;
	else if (AReason == INSTANTGAMING_REASON_DRAW)
		reason = IInstantGaming::Draw;
	else if (AReason == INSTANTGAMING_REASON_RESIGN)
		reason = IInstantGaming::Resign;
	else if (AReason == INSTANTGAMING_REASON_ADJOURN)
		reason = IInstantGaming::Adjourn;
	else if (AReason == INSTANTGAMING_REASON_CHEATING)
		reason = IInstantGaming::Cheating;
	return reason;
}

QString InstantGaming::terminateReasonToString(int AReason) const
{
	QString reason;
	switch (AReason){
	case IInstantGaming::Won:
		reason = INSTANTGAMING_REASON_WON;
		break;
	case IInstantGaming::Lost:
		reason = INSTANTGAMING_REASON_LOST;
		break;
	case IInstantGaming::Draw:
		reason = INSTANTGAMING_REASON_DRAW;
		break;
	case IInstantGaming::Resign:
		reason = INSTANTGAMING_REASON_RESIGN;
		break;
	case IInstantGaming::Adjourn:
		reason = INSTANTGAMING_REASON_ADJOURN;
		break;
	case IInstantGaming::Cheating:
		reason = INSTANTGAMING_REASON_CHEATING;
		break;
	default:
		break;
	}
	return reason;
}

QUuid InstantGaming::findGameUuid(const QString &AVar) const
{
	QUuid gameUuid;
	for (QHash<QUuid, IGame *>::ConstIterator it=FGames.constBegin(); it!=FGames.constEnd(); ++it)
	{
		IGameInfo gameInfo;
		(*it)->gameInfo(&gameInfo);
		if (gameInfo.var == AVar)
		{
			gameUuid = it.key();
		}
	}
	return gameUuid;
}
