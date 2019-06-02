#ifndef OMEMO_H
#define OMEMO_H

#include <interfaces/iomemo.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagestylemanager.h>

class SignalProtocol;

class Omemo: public QObject,
			 public IPlugin,
			 public IOmemo,
			 public IPEPHandler
//			 public IMessageEditor,
//			 public IMessageWriter
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IOmemo IPEPHandler) // IMessageEditor IMessageWriter)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOmemo")
#endif
public:
	Omemo();
	~Omemo();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return OMEMO_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }

	// IPEPHandler
	virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza) override;

	//IMessageEditor
//	virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection) override;

	//IMessageWriter
//	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang) override;
//	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang) override;
//	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang) override;

protected:
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	void registerDiscoFeatures();
	void updateChatWindowActions(IMessageChatWindow *AChatWindow);

protected slots:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
//	void onInsertLink(bool);

	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onNormalWindowCreated(IMessageNormalWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);

private:
	IPEPManager*		FPepManager;
	IXmppStreamManager*	FXmppStreamManager;
//	IMessageProcessor*	FMessageProcessor;
	IServiceDiscovery*	FDiscovery;
	IMessageWidgets*	FMessageWidgets;
	IMessageStyleManager* FMessageStyleManager;

	IconStorage*		FIconStorage;
	int					FOmemoHandlerIn;
	int					FOmemoHandlerOut;

	SignalProtocol*		FSignalProtocol;

	QMap <Jid, QString> FStreamOmemo;
};

#endif // OMEMO_H