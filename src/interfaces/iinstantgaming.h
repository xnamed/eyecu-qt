#ifndef IINSTANTGAMING_H
#define IINSTANTGAMING_H

#include <QObject>
#include <utils/jid.h>
#include <utils/stanza.h>
#include <interfaces/idataforms.h>
#include <QUuid>

#define INSTANTGAMING_UUID "{963ADDA3-CC98-4EA3-A2DA-E493382CD4FD}"

struct IInstantGame
{
	Jid streamJid;
	Jid contactJid;
	QString threadId;
	QString var;
	QUuid uuid;
	int player;
	int type;
	int status;
};

class IInstantGaming {
public:
	enum Player {
		Initiator,
		Opponent
		//Spectator
	};
	enum Invite {
		New,
		Renew,
		Adjourned,
		Constructed
	};
	enum Terminate {
		Won,
		Lost,
		Draw,
		Resign,
		Adjourn,
		Cheating
	};
	enum Type {
		Invitation,
		Declined,
		Joined,
		GamePlay,
		Saving,
		Saved,
		Termination
	};
	enum Status {
		Pending,
		Active,
		Inactive
	};

	virtual QObject *instance() =0;
	virtual bool sendInvitation(const QString &AThread, IDataForm &AForm, const QString &AMessage, int AType) =0;
	//virtual bool declineInvitation(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReason=QString(), const QString &AThread=QString()) =0;
	virtual bool joinGame(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage, const QString &AThread) =0;
	virtual bool sendTurn(const QString &AThread, const QString &AMessage, const QDomElement &AElement) =0;
	virtual bool sendSave(const QString &AThread) =0;
	virtual bool sendSaved(const QString &AThread) =0;
	virtual bool rejectGame(const Jid &AStreamJid, const Stanza &AStanz) =0;
	virtual bool terminateGame(const QString &AThread, const QString &AMessage, int AReason) =0;
	virtual void selectGame(IInstantGame &AGame, const QString &AMessage, int AType) =0;
	virtual IInstantGame findGame(const QString &AThread) const =0;
};

Q_DECLARE_INTERFACE(IInstantGaming, "RWS.Plugin.IInstantGaming/1.0")

#endif // IINSTANTGAMING_H
