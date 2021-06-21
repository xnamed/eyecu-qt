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
	virtual void startGame(const QString &AThread, int AType) =0;
};

Q_DECLARE_INTERFACE(IGame, "RWS.Plugin.IGame/1.0")

#endif // IGAME_H
