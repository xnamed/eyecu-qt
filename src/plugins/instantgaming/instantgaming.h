#ifndef INSTANTGAMING_H
#define INSTANTGAMING_H

#include <interfaces/iinstantgaming.h>
#include <interfaces/igame.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/idataforms.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/inotifications.h>
#include <QMessageBox>

struct GameState {
	QString threadId;
	int state;
	int type;
	QString text;
};

class InstantGaming:
		public QObject,
		public IPlugin,
		public IInstantGaming,
		public IStanzaHandler
{

	Q_OBJECT
	Q_INTERFACES(IPlugin IInstantGaming IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IInstantGaming")
#endif

public:
	InstantGaming();
	~InstantGaming();

	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return INSTANTGAMING_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}
	//IInstantGaming
	virtual bool sendInvitation(const QString &AThread, IDataForm &AForm, const QString &AReason, int AType);
	virtual bool sendTurn(const QString &AThread, const QString &AMessage, const QDomElement &AElement);
	virtual bool joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage, const QString &AThread);
	virtual bool rejectGame(const Jid &AStreamJid, const Stanza &AStanza);
	virtual bool sendSave(const QString &AThread);
	virtual bool sendSaved(const QString &AThread);
	virtual bool terminateGame(const QString &AThread, const QString &AMessage, int AReason);
	virtual void selectGame(IInstantGame &AGame, const QString &AMessage, int AType);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

protected:
	void registerDiscoFeatures();
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	void updateToolBarAction(IMessageToolBarWidget *AWidget);
	void showGameSelector(const Jid &AStreamJid, const Jid &AContactJid);
	bool declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread);
protected:
	void startGame(IInstantGame &AGame);
	void setTurn(const QString &AThread, const QDomElement &ATurnElem, const QString &AMessage);
	void notifyGame(const QString &AThread, int AGameState, const QString &AMessage, int AType);
	void showStatusEvent(IMessageViewWidget *AView, const QString &AHtml) const;
protected:
	int inviteTypeToCode(const QString &AType) const;
	QString inviteTypeToString(int AType) const;
	int terminateReasonToCode(const QString &AReason) const;
	QString terminateReasonToString(int AReason) const;
	QUuid findGameUuid(const QString &AVar) const;
signals:
	void gameStateChanged(const QString &AThread, int AGameState, const QString &AMessage, int AType);
protected slots:
	void onXmppStreamOpened(IXmppStream *AXmppStream);
	void onXmppStreamClosed(IXmppStream *AXmppStream);
protected slots:
	void onShowGameSelectorByToolBarAction(bool);
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onToolBarWidgetAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onToolBarWidgetDestroyed(QObject *AObject);
protected slots:
	void onNotificationActivated(int ANotifyId);
	void onNotificationRemoved(int ANotifyId);
	void onInviteDialogFinished(int AResult);
private:
	int FSHIGames;
	QHash<QUuid, IGame*> FGames;
	QMap<IMessageToolBarWidget*, Action*> FToolBarActions;
	QHash<QString, IInstantGame> FActiveGames;
	QMap<int, GameState> FGameNotify;
	QStringList FSaveRequests;
	QMap<QMessageBox *, QString> FInviteDialogs;
private:
	IXmppStreamManager *FXmppStreamManager;
	IStanzaProcessor  *FStanzaProcessor;
	IServiceDiscovery *FDiscovery;
	IDataForms *FDataForms;
	IMessageWidgets *FMessageWidgets;
	INotifications *FNotifications;
};

#endif // INSTANTGAMING_H
