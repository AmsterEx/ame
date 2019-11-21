#ifndef STATISTICSPAGE_H
#define STATISTICSPAGE_H

#include <QWidget>

//#include "walletmodel.h"

namespace Ui {
	class StatisticsPage;
}
class ClientModel;

class StatisticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPage(QWidget *parent = 0);
    ~StatisticsPage();
    
    void setModel(ClientModel *model);
	void setModel2(ClientModel *model2);
	void showOutOfSyncWarning(bool fShow);
    void updatePlot();
	int heightPrevious;
    int connectionPrevious;
    int volumePrevious;
    int stakemaxPrevious;
    QString stakecPrevious;
    QString pawratePrevious;
    double hardnessPrevious2;
	
    
    void updatePrevious(int, int, double, QString, int, double);
	void updateStats();

private slots:

private:
    Ui::StatisticsPage *ui;
    ClientModel *model;
	ClientModel *model2;
	
	QVector<double> vX;
	QVector<double> vY;
	
	QVector<double> vX3;
	QVector<double> vY3;
	
	double currentStrength;
};

#endif 