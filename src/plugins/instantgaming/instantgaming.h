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
	virtual bool sendInvitation(const QString &AThread, IDataForm &AForm, const QString &AMessage, int AType);
	virtual bool sendTurn(const QString &AThread, const QString &AMessage, const QDomElement &AElement);
	virtual bool joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage, const QString &AThread);
	virtual bool rejectGame(const Jid &AStreamJid, const Stanza &AStanza);
	virtual bool sendSave(const QString &AThread);
	virtual bool sendSaved(const QString &AThread);
	virtual bool terminateGame(const QString &AThread, const QString &AMessage, int AReason);
	virtual void selectGame(IInstantGamePlay &AGame, const QString &AMessage, int AType);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

protected:
	void registerDiscoFeatures();
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	void updateToolBarAction(IMessageToolBarWidget *AWidget);
	void showGameSelector(const Jid &AStreamJid, const Jid &AContactJid);
	bool declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread);
protected:
	void startGame(IInstantGamePlay &AGame, int AType);
protected:
	int inviteTypeToCode(const QString &AType) const;
	QString inviteTypeToString(int AType) const;
	int terminateReasonToCode(const QString &AReason) const;
	QString terminateReasonToString(int AReason) const;
signals:
	void invitationReceived(const IInstantGamePlay &AGame);
	void invitationDeclined(const QString &AReason, const QString &AThread);
	void gameJoined(const QString &AMessage, const QString &AThread);
	void gamePlay(const QString &AMessage, QDomElement &ATurnElem, const QString &AThread);
	void gameSaveRequested(const QString &AThread);
	void gameSaved(const QString &AThread);
	void gameTerminated(const QString AReason, const QString &AMessage, const QString &AThread);
protected slots:
	void onXmppStreamOpened(IXmppStream *AXmppStream);
	void onXmppStreamClosed(IXmppStream *AXmppStream);
protected slots:
	void onShowGameSelectorByToolBarAction(bool);
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onToolBarWidgetAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onToolBarWidgetDestroyed(QObject *AObject);
private:
	int FSHIGames;
	QHash<QUuid, IGame*> FGames;
	QMap<IMessageToolBarWidget*, Action*> FToolBarActions;
	QHash<QString, IInstantGamePlay> FActiveGames;
private:
	IXmppStreamManager *FXmppStreamManager;
	IStanzaProcessor  *FStanzaProcessor;
	IServiceDiscovery *FDiscovery;
	IDataForms *FDataForms;
	IMessageWidgets *FMessageWidgets;
};

#endif // INSTANTGAMING_H
