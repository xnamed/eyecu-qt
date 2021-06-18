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
	QObject *instance() {return this;}
	QUuid pluginUuid() const {return INSTANTGAMING_UUID;}
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}

	virtual bool sendInvitation(const IInstantGamePlay &AGame);
	virtual bool sendTurn(const IInstantGamePlay &AGame, const QDomElement &AElement);
	virtual bool declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread);
	virtual bool joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread);
	virtual bool rejectGame(const Jid &AStreamJid, const Stanza &AStanza);
	virtual bool sendSave(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread);
	virtual bool sendSaved(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread);
	virtual bool terminateGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread);

	//IStanzaHandler
	bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

protected:
	void registerDiscoFeatures();
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	void updateToolBarAction(IMessageToolBarWidget *AWidget);
	void showGameSelector(const Jid &AStreamJid, const Jid &AContactJid);
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
private:
	IXmppStreamManager *FXmppStreamManager;
	IStanzaProcessor  *FStanzaProcessor;
	IServiceDiscovery *FDiscovery;
	IDataForms *FDataForms;
	IMessageWidgets *FMessageWidgets;
};

#endif // INSTANTGAMING_H
