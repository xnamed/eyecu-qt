#ifndef IHANGMANGAME_H
#define IHANGMANGAME_H

#include <QObject>

#define HANGMANGAME_UUID  "6BCB485A-4672-4B8F-AF9B-C9A5620C3750"

class IHangmanGame
{
public:
	virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IHangmanGame, "RWS.Plugin.IHangmanGame/1.0")

#endif // IHANGMANGAME_H
