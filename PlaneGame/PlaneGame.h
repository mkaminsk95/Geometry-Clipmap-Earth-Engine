#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PlaneGame.h"

class PlaneGame : public QMainWindow
{
	Q_OBJECT

public:
	PlaneGame(QWidget *parent = Q_NULLPTR);

private:
	Ui::PlaneGameClass ui;
};
