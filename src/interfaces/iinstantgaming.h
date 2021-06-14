#ifndef IINSTANTGAMING_H
#define IINSTANTGAMING_H

#include <QObject>
#include <utils/jid.h>
#include <utils/stanza.h>
#include <interfaces/idataforms.h>

#define INSTANTGAMING_UUID "{963ADDA3-CC98-4EA3-A2DA-E493382CD4FD}"

struct IInstantGamePlay
{
	Jid streamJid;
	Jid contactJid;
	QString threadId;
	QString message;
	QString reason;
	QString type;
	QString game;
	IDataForm form;
};

class IInstantGaming {
public:
	virtual QObject *instance() =0;
	virtual bool sendInvitation(const IInstantGamePlay &AGame) =0;
	virtual bool declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason=QString(), const QString &AThread=QString()) =0;
	virtual bool joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread=QString()) =0;
	virtual bool sendTurn(const IInstantGamePlay &AGame, const QDomElement &AElement) =0;
	virtual bool sendSave(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread=QString()) =0;
	virtual bool sendSaved(const Jid &AStreamJid, const Jid &AContactJid, const QString &AThread=QString()) =0;
	virtual bool rejectGame(const Jid &AStreamJid, const Stanza &AStanz) =0;
	virtual bool terminateGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason, const QString &AThread=QString()) =0;
};

Q_DECLARE_INTERFACE(IInstantGaming, "RWS.Plugin.IInstantGaming/1.0")

#endif // IINSTANTGAMING_H
