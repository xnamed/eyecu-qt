#include "gameselect.h"
#include "ui_gameselect.h"
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#define TDR_VAR     Qt::UserRole+1

GameSelect::GameSelect(InstantGaming *AInstantGaming, const Jid &AStreamJid, const Jid &AContactJid, const QHash<QUuid, IGame *> &AGames, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GameSelect)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);

	FStreamJid = AStreamJid;
	FContactJid = AContactJid;
	FInstantGaming = AInstantGaming;

	setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_INSTANTGAMING));
	setWindowTitle(tr("Play with - %1").arg(AContactJid.uFull()));

	ui->tedMessage->setText(tr("would you like to play a little game?"));

	for (QHash<QUuid,IGame *>::ConstIterator it=AGames.constBegin(); it != AGames.constEnd(); ++it)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem();

		IGameInfo gameInfo;
		(*it)->gameInfo(&gameInfo);

		item->setIcon(0, gameInfo.icon);
		item->setText(0, gameInfo.name);
		item->setData(0, Qt::UserRole, it.key());
		item->setData(0, TDR_VAR, gameInfo.var);
		item->setText(1, gameInfo.category);
		ui->twGames->addTopLevelItem(item);

		//tabSaved games

		connect(ui->dbbButtons,SIGNAL(accepted()),SLOT(onDialogAccepted()));
		connect(ui->dbbButtons,SIGNAL(rejected()),SLOT(reject()));
	}
}

void GameSelect::onDialogAccepted()
{
	QTreeWidgetItem *item = ui->twGames->currentItem();
	if (item)
	{
		QUuid gameUuid = item->data(0, Qt::UserRole).toString();
		IInstantGamePlay instantGame;
		instantGame.streamJid = FStreamJid;
		instantGame.contactJid = FContactJid;
		instantGame.var = item->data(0, TDR_VAR).toString();
		instantGame.uuid = gameUuid;
		instantGame.player = IInstantGaming::Initiator;

		FInstantGaming->selectGame(instantGame, ui->tedMessage->toPlainText(), IInstantGaming::New);
	}
}

GameSelect::~GameSelect()
{
	delete ui;
}
