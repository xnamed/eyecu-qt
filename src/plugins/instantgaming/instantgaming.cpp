#include "instantgaming.h"
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/xmpperrors.h>
#include <definitions/stanzahandlerorders.h>
#include <utils/message.h>
#include <definitions/toolbargroups.h>

#include "gameselect.h"
#include <utils/widgetmanager.h>

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
	FMessageWidgets(NULL)
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

	plugin = APluginManager->pluginInterface("IStanzaProcesssor").value(0);
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
	return !FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
			FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_INSTANTGAMING);
}

bool InstantGaming::sendInvitation(const QString &AThread, IDataForm &AForm, const QString &AMessage, int AType)
{
	IInstantGamePlay game = FActiveGames.value(AThread);
	if (FStanzaProcessor && FDataForms)
	{
		Stanza invite(STANZA_KIND_MESSAGE);
		invite.setTo(game.contactJid.full());
		invite.createElement("thread").appendChild(invite.createTextNode(game.threadId));

		QDomElement inviteElem = invite.addElement("invite", NS_INSTANTGAMING);
		inviteElem.setAttribute("type", inviteTypeToString(AType));

		if (!AMessage.isEmpty())
		{
			inviteElem.appendChild(invite.createElement("reason")).appendChild(invite.createTextNode(AMessage));
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
		decline.createElement("thread").appendChild(decline.createTextNode(AThread));

		QDomElement declineElem = decline.addElement("decline", NS_INSTANTGAMING);
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
	if (FStanzaProcessor)
	{
		Stanza join(STANZA_KIND_MESSAGE);
		join.setTo(AContactJid.full());
		join.createElement("thread").appendChild(join.createTextNode(AThread));
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
	IInstantGamePlay game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza turn(STANZA_KIND_MESSAGE);
		turn.setTo(game.contactJid.full()).setType(MESSAGE_TYPE_CHAT);
		turn.createElement("thread").appendChild(turn.createTextNode(AThread));

		QDomElement turnElem = turn.addElement("turn", NS_INSTANTGAMING);
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
	IInstantGamePlay game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza save(STANZA_KIND_MESSAGE);
		save.setTo(game.contactJid.full());
		save.createElement("thread").appendChild(save.createTextNode(AThread));
		save.addElement("save",NS_INSTANTGAMING);
		if (FStanzaProcessor->sendStanzaOut(game.streamJid,save))
			return true;
	}
	return false;
}

bool InstantGaming::sendSaved(const QString &AThread)
{
	IInstantGamePlay game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza saved(STANZA_KIND_MESSAGE);
		saved.setTo(game.contactJid.full());
		saved.createElement("thread").appendChild(saved.createTextNode(AThread));
		saved.addElement("saved",NS_INSTANTGAMING);
		if (FStanzaProcessor->sendStanzaOut(game.streamJid,saved))
			return true;
	}
	return false;
}

bool InstantGaming::terminateGame(const QString &AThread, const QString &AMessage, int AReason)
{
	IInstantGamePlay game = FActiveGames.value(AThread);
	if (FStanzaProcessor)
	{
		Stanza terminate(STANZA_KIND_MESSAGE);
		terminate.setTo(game.contactJid.full());
		terminate.createElement("thread").appendChild(terminate.createTextNode(AThread));
		terminate.addElement("terminate", NS_INSTANTGAMING);
		terminate.setAttribute("reason", terminateReasonToString(AReason));

		if (FStanzaProcessor->sendStanzaOut(game.streamJid,terminate))
			return true;
	}
	return false;
}

void InstantGaming::onXmppStreamOpened(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.streamJid = AXmppStream->streamJid();
		shandle.conditions.append(SHC_INSTANTGAMING_INVITE);
		shandle.conditions.append(SHC_INSTANTGAMING_DECLINE);
		shandle.conditions.append(SHC_INSTANTGAMING_JOIN);
		shandle.conditions.append(SHC_INSTANTGAMING_TURN);
		shandle.conditions.append(SHC_INSTANTGAMING_SAVE);
		shandle.conditions.append(SHC_INSTANTGAMING_SAVED);
		shandle.conditions.append(SHC_INSTANTGAMING_TERMINATE);
		FSHIGames = FStanzaProcessor->insertStanzaHandle(shandle);
	}
}

void InstantGaming::onXmppStreamClosed(IXmppStream *AXmppStream)
{}

bool InstantGaming::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (AHandleId == FSHIGames && FDataForms)
	{
		AAccept = true;
		Message message(AStanza);
		QString threadId = AStanza.firstElement("thread").text();
		//Invitation / Game Loading
		if (!AStanza.firstElement("invite",NS_INSTANTGAMING).isNull())
		{
			IInstantGamePlay instantGame;
			instantGame.streamJid = AStreamJid;
			instantGame.contactJid = AStanza.fromJid();
			instantGame.threadId = threadId;

			QDomElement inviteElem = AStanza.firstElement("invite",NS_INSTANTGAMING);
			int inviteType = inviteTypeToCode(inviteElem.attribute("type"));
			QString reason = inviteElem.firstChildElement("reason").text();

			QDomElement gameElem = inviteElem.firstChildElement("game");
			instantGame.var = gameElem.attribute("var");

			if (FDiscovery && !instantGame.var.isEmpty() && FDiscovery->hasFeatureHandler(instantGame.var))
			{
				QDomElement formElem = Stanza::findElement(gameElem,"x",NS_JABBER_DATA);

				if (!formElem.isNull())
					IDataForm form = FDataForms->dataForm(formElem);
				emit invitationReceived(instantGame);
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
			emit invitationDeclined(reason,threadId);
		}
		//Joining a Game
		else if (!AStanza.firstElement("join",NS_INSTANTGAMING).isNull())
		{
			//The initiator might refuse the join
			if (AStanza.isError())
			{}
			else
			{
				//<body/> element is OPTIONAL.
				emit gameJoined(message.body(),threadId);
			}
		}
		//Game Play
		else if (!AStanza.firstElement("turn",NS_INSTANTGAMING).isNull())
		{
			QDomElement turnElem = AStanza.firstElement("turn",NS_INSTANTGAMING);
			emit gamePlay(message.body(),turnElem,threadId);
		}
		//Game Saving
		else if (!AStanza.firstElement("save",NS_INSTANTGAMING).isNull())
		{
			if (AStanza.isError())
			{}
			else
			{
				emit gameSaveRequested(threadId);
			}
		}
		else if (!AStanza.firstElement("saved",NS_INSTANTGAMING).isNull())
		{
			emit gameSaved(threadId);
		}
		//Termination
		else if (!AStanza.firstElement("terminate",NS_INSTANTGAMING).isNull())
		{
			QDomElement termElem = AStanza.firstElement("terminate",NS_INSTANTGAMING);
			QString reason = termElem.attribute("reason");
			emit gameTerminated(reason,message.body(),threadId);
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

void InstantGaming::selectGame(IInstantGamePlay &AGame, const QString &AMessage, int AType)
{
	QString threadId;
	IDataForm form;
	if (AType == New)
	{
		int id;
		for (id=qrand(); FActiveGames.keys().contains(QString::number(id)); id++);
		threadId = QString::number(id);
	}
	else if (AType != New)
	{
		threadId = AGame.threadId;
		if (AType != Renew)
		{
			//load game
		}
	}
	FActiveGames.insert(threadId,AGame);
	sendInvitation(threadId,form,AMessage,AType);
}

void InstantGaming::startGame(IInstantGamePlay &AGame, int AType)
{
	if (AGame.player == Initiator)
	{
		for (QHash<QUuid, IGame *>::ConstIterator it=FGames.constBegin(); it!=FGames.constEnd(); ++it)
		{
			IGameInfo gameInfo;
			(*it)->gameInfo(&gameInfo);
			if (gameInfo.var == AGame.var)
			{
				AGame.uuid = it.key();
				FActiveGames.insert(AGame.threadId,AGame);
			}
		}
	}
	IGame *game = FGames.value(AGame.uuid);
	if (game)
	{
		game->startGame(AGame.threadId, AType);
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
