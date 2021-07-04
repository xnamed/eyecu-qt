#include "gamewindow.h"
#include "ui_gamewindow.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <QDebug>
#include "hangmangame.h"

GameWindow::GameWindow(HangmanGame *AGame, const QString &AThread, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::GameWindow)
{
	ui->setupUi(this);
	FGame = AGame;
	FThread = AThread;

	setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HANGMAN_GAME));
	setWindowTitle(tr("Hangman game"));
	ui->pbtPlay->setText(tr("Play"));
	ui->lblExtraChanceLabel->setText(tr("Extra chance:"));

	connect(ui->lneGuess,SIGNAL(textChanged(QString)),SLOT(onTextChanged(QString)));
	connect(ui->pbtPlay,SIGNAL(clicked(bool)),SLOT(onPlayButtonClicked(bool)));
}

GameWindow::~GameWindow()
{
	delete ui;
}

void GameWindow::setPuzzle(const QList<int> &APuzzle, const QString &AHint)
{
	QString hint = AHint.isEmpty() ? tr("Guess the following word:")
								   : AHint;
	ui->lblHint->setText(hint);
	ui->gbPuzzle->setEnabled(true);

	QStringList puzzle;
	for (QList<int>::const_iterator it=APuzzle.constBegin(); it!=APuzzle.constEnd(); it++)
	{
		puzzle.append(QString(" _").repeated(*it));
	}
	ui->lblPuzzle->setText(QString("<pre align='center'>%1</pre>")
						   .arg(puzzle.join("  ")));
}

void GameWindow::updatePuzzle(const QList<int> &APuzzle, const QHash<int, QString> &AGuessed)
{
	int len = 0;
	QStringList puzzle;
	for (QList<int>::const_iterator it=APuzzle.constBegin(); it!=APuzzle.constEnd(); it++)
	{
		QString text = QString(" _").repeated(*it);

		for (int i=1; i < (*it+1); i++)
		{
			if (AGuessed.contains(i+len))
			{
				/* " _ _ _ _"
				 * "01234567" */
				text[(i*2)-1] = *(AGuessed.value(i+len).toUpper().unicode());
			}
		}
		len += *it;
		puzzle.append(text);
	}
	ui->lblPuzzle->setText(QString("<p align='center'><span><pre>%1</pre></span></p>")
						   .arg(puzzle.join("  ")));
}

void GameWindow::updateGallows(int ADiagram)
{
	if (ADiagram < 0)
	{
		ui->lblExtraChance->setText(QString::number(abs(ADiagram)));
	}
	else
	{
		ui->lblExtraChance->setText("0");
	}
}

void GameWindow::updateKeys(const QStringList &APlayed, const QList<QString> &AGuessed)
{
	QStringList found;
	foreach (const QString &str, APlayed)
	{
		if (!AGuessed.contains(str))
				found.append(str);
	}
	ui->lblWrong->setText(QString("<p align='center'>%1</p>").arg(found.join(", ")));
}

void GameWindow::switchRole(int ARole)
{
	FRole = ARole;
	if (ARole == HangmanGame::PuzzleSetter)
	{
		ui->lneGuess->setMaxLength(32);
		ui->lblMessageLabel->setText(tr("Hint"));
		ui->pbtPlay->setEnabled(false);
	}
	else if (ARole == HangmanGame::Guesser)
	{
		ui->lneGuess->setMaxLength(1);
		ui->lblMessageLabel->setText(tr("Message"));
		ui->gbPuzzle->setEnabled(false);
		ui->lblHint->setStyleSheet("font-size:14px; color:#202122;");
		ui->lblHint->setText(QString("<p align='center'>%1</p>")
							 .arg(tr("Waiting for the opponent to set puzzle ...")));
	}
	ui->pbtPlay->setEnabled(false);
	ui->lblWrong->setStyleSheet("font-size:14px; color:#202122;text-decoration:line-through;");
	ui->lblWrong->setText("");

	ui->lblPuzzle->setStyleSheet("font-size:14px; font-weight:600; color:#202122;");
}

void GameWindow::onTextChanged(const QString &AText)
{
	QString text = AText;
	int len = text.remove(QRegExp("(\\ |\\,|\\.|\\:|\\t)")).length();
	if (len == 1 && FRole == HangmanGame::Guesser
			&& !FGame->isLetterAlreadyUsed(FThread,AText))
	{
		ui->pbtPlay->setEnabled(true);
	}
	else if (len > 2 && FRole == HangmanGame::PuzzleSetter)
	{
		ui->pbtPlay->setEnabled(true);
		int chance = len > 9 ? len - 9 : 0;
		ui->lblExtraChance->setText(QString::number(chance));
	}
	else
	{
		ui->pbtPlay->setEnabled(false);
	}

	if (FRole == HangmanGame::PuzzleSetter)
	{
		QStringList puzzle;
		foreach (const QString &word, AText.split(" ")) {
			puzzle.append(QString(" _").repeated(word.length()));
		}
		ui->lblPuzzle->setText(QString("<p align='center'><span><pre>%1</pre></span></p>")
							   .arg(puzzle.join("  ")));
	}
}

void GameWindow::onPlayButtonClicked(bool)
{
	if (FRole == HangmanGame::PuzzleSetter)
	{
		QStringList words = ui->lneGuess->text().split(" ");
		if (FGame->sendPuzzle(FThread,words,ui->pteHint->toPlainText()))
			ui->gbPuzzle->setEnabled(false);
	}
	else if (FRole == HangmanGame::Guesser)
	{
		if (FGame->sendGuess(FThread,ui->lneGuess->text(),ui->pteHint->toPlainText()))
		{
			ui->lneGuess->setText("");
			ui->pteHint->setText("");
		}
	}
}
