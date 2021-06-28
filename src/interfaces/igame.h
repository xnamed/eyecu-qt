#ifndef IGAME_H
#define IGAME_H

#include <QObject>
#include <utils/jid.h>
#include "iinstantgaming.h"

struct IGameInfo
{
	QString name;
	QString category;
	QIcon icon;
	QString var;
};

class IGame
{
public:
	virtual QObject *instance() =0;
	virtual void gameInfo(IGameInfo *AGameInfo) =0;
	virtual bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) =0;
	virtual void startGame(const QString &AThread, const Jid &AStreamJid, const Jid &AContactJid, int AType) =0;
	virtual void turn(const QString &AThread, const QDomElement &AElement, const QString &AMessage) =0;
	//virtual bool save(const QString &AThread) =0;
	//virtual bool terminate(const QString &AThread, const QString &AMessage, int AReason) =0;
};

Q_DECLARE_INTERFACE(IGame, "RWS.Plugin.IGame/1.0")

#endif // IGAME_H
