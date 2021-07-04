#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
class HangmanGame;

namespace Ui {
class GameWindow;
}

class GameWindow : public QWidget
{
	Q_OBJECT

public:
	GameWindow(HangmanGame *AGame, const QString &AThread, QWidget *parent = 0);
	~GameWindow();
	void setPuzzle(const QList<int> &APuzzle, const QString &AHint);
	void updatePuzzle(const QList<int> &APuzzle, const QHash<int, QString> &AGuessed);
	void updateGallows(int ADiagram);
	void updateKeys(const QStringList &APlayed, const QList<QString> &AGuessed);
	void switchRole(int ARole);
protected slots:
	void onTextChanged(const QString &AText);
	void onPlayButtonClicked(bool);

private:
	Ui::GameWindow *ui;
private:
	HangmanGame *FGame;
	QString FThread;
	int FRole;
};

#endif // GAMEWINDOW_H
