#ifndef GAMESELECT_H
#define GAMESELECT_H

#include <QDialog>
#include "instantgaming.h"
#include "ui_gameselect.h"

namespace Ui {
class GameSelect;
}

class GameSelect : public QDialog
{
	Q_OBJECT

public:
	GameSelect(InstantGaming *AInstantGaming, const Jid &AStreamJid, const Jid &AContactJid, const QHash<QUuid, IGame *> &AGames, QWidget *parent = 0);
	~GameSelect();
protected slots:
	void onDialogAccepted();

private:
	Ui::GameSelect *ui;
private:
	Jid FStreamJid;
	Jid FContactJid;
private:
	IInstantGaming *FInstantGaming;
};

#endif // GAMESELECT_H
