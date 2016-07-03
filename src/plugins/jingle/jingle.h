#ifndef JINGLE_H
#define JINGLE_H

#include <interfaces/ijingle.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ioptionsmanager.h>
#include "jinglesession.h"

class Jingle: public QObject,
              public IPlugin,
              public IJingle,
              public IStanzaHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IJingle IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingle")
#endif
public:
    Jingle(QObject *parent = 0);
	~Jingle();
    bool sendStanzaOut(Stanza &AStanza);

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return JINGLE_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    //IStanzaHandler
    bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

    //IJingle
    IJingleApplication *appByNS(const QString &AApplicationNS) {return FApplications.value(AApplicationNS);}
    QString sessionCreate(const Jid &AStreamJid, const Jid &AContactJid, const QString &AApplicationNS);
    bool    sessionInitiate(const QString &ASid);
    bool    sessionAccept(const QString &ASid);
    bool    sessionTerminate(const QString &ASid, Reason AReason);
	bool    sendAction(const QString &ASid, IJingle::Action AAction, const QDomElement &AJingleElement);
    bool    sendAction(const QString &ASid, IJingle::Action AAction, const QDomNodeList &AJingleElements);  

    IJingleContent *contentAdd(const QString &ASid, const QString &AName, const QString &AMediaType, const QString &ATransportNameSpace, bool AFromResponder);
    QHash<QString, IJingleContent *> contents(const QString &ASid) const;
    IJingleContent *content(const QString &ASid, const QString &AName) const;
	IJingleContent *content(const QString &ASid, QIODevice *ADevice) const;
	bool    selectTransportCandidate(const QString &ASid, const QString &AContentName, const QString &ACandidateId);
	bool    connectContent(const QString &ASid, const QString &AName);
    bool    setConnected(const QString &ASid);
    bool    fillIncomingTransport(IJingleContent *AContent);
	void    freeIncomingTransport(IJingleContent *AContent);

    SessionStatus sessionStatus(const QString &ASid) const;
    bool    isOutgoing(const QString &ASid) const;
    Jid     contactJid(const QString &ASid) const;
	Jid     streamJid(const QString &ASid) const;
    QString errorMessage(Reason AReason) const;

protected:
    void registerDiscoFeatures();
	bool processSessionInitiate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
    bool processSessionAccept(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
	bool processSessionTerminate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
	bool processSessionInfo(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);

protected slots:
	void onConnectionOpened(IJingleContent *AContent);
	void onConnectionFailed(IJingleContent *AContent);
    void onIncomingTransportFilled(IJingleContent *AContent);
    void onIncomingTransportFillFailed(IJingleContent *AContent);

signals:
	void startSendData(IJingleContent *AContent);
    void connectionFailed(IJingleContent *AContent);
    void contentAdded(IJingleContent *AContent);
    void contentAddFailed(IJingleContent *AContent);
    void incomingTransportFilled(IJingleContent *AContent);
    void incomingTransportFillFailed(IJingleContent *AContent);

private:
    IStanzaProcessor    *FStanzaProcessor;
    IServiceDiscovery   *FServiceDiscovery;
	IOptionsManager		*FOptionsManager;
    QMap<QString, IJingleApplication*>  FApplications;
    QMap<QString, IJingleTransport*>    FTransports;
    QHash<QIODevice *, IJingleContent *> FCandidateTries;
    QList<IJingleContent *> FPendingContents;
    int FSHIRequest;
    int FSHIResult;
    int FSHIError;
};

#endif // JINGLE_H
