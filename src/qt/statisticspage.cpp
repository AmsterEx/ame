#include "statisticspage.h"
#include "ui_statisticspage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "main.h"
#include "bitcoinrpc.h"
#include "util.h"
#include "init.h"

#include <sstream>
#include <string>
double GetPoSKernelPS(const CBlockIndex* pindex);

using namespace json_spirit;



StatisticsPage::StatisticsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticsPage)
{
    ui->setupUi(this);
    ui->labelNetworkStatus->setText(" (" + tr("Network info is turned off during syncing") + ")");
	ui->labelNetworkStatusCharts->setText(" (" + tr("Charts are turned off during syncing") + ")");
	
    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
    if(GetBoolArg("-chart", true))
    {
		QFont label = font();
		
		//weight
        ui->diffplot_weight->addGraph();
		ui->diffplot_weight->setBackground(QBrush(QColor(255,255,255)));
        ui->diffplot_weight->xAxis->setLabel("Height");
        ui->diffplot_weight->yAxis->setLabel("Network Weight");
        ui->diffplot_weight->graph(0)->setPen(QPen(QColor(131,189,191), 3, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		ui->diffplot_weight->graph(0)->setBrush(QBrush(QColor(131,189,191,30)));
        ui->diffplot_weight->xAxis->setTickLabelColor(QColor(48,63,76));
        ui->diffplot_weight->xAxis->setBasePen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->xAxis->setLabelColor(QColor(131,189,191));
        ui->diffplot_weight->xAxis->setTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->xAxis->setSubTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->yAxis->setTickLabelColor(QColor(48,63,76));
        ui->diffplot_weight->yAxis->setBasePen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->yAxis->setLabelColor(QColor(131,189,191));
        ui->diffplot_weight->yAxis->setTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->yAxis->setSubTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_weight->yAxis->grid()->setPen(QPen(QColor(QColor(212,211,211)), 1, Qt::DotLine));		
        ui->diffplot_weight->xAxis->grid()->setPen(QPen(QColor(QColor(212,211,211)), 1, Qt::DotLine));
		ui->diffplot_weight->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
        ui->diffplot_weight->graph(0)->setLineStyle(QCPGraph::lsLine); 
        ui->diffplot_weight->xAxis->setLabelFont(label);
        ui->diffplot_weight->yAxis->setLabelFont(label);
		
		//difficulty pos
		ui->diffplot_pos->addGraph();
		ui->diffplot_pos->setBackground(QBrush(QColor(255,255,255)));
        ui->diffplot_pos->xAxis->setLabel("Height");
        ui->diffplot_pos->yAxis->setLabel("Difficulty PoS");
        ui->diffplot_pos->graph(0)->setPen(QPen(QColor(131,189,191), 3, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		ui->diffplot_pos->graph(0)->setBrush(QBrush(QColor(131,189,191,30)));
        ui->diffplot_pos->xAxis->setTickLabelColor(QColor(48,63,76));
        ui->diffplot_pos->xAxis->setBasePen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->xAxis->setLabelColor(QColor(131,189,191));
        ui->diffplot_pos->xAxis->setTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->xAxis->setSubTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->yAxis->setTickLabelColor(QColor(48,63,76));
        ui->diffplot_pos->yAxis->setBasePen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->yAxis->setLabelColor(QColor(131,189,191));
        ui->diffplot_pos->yAxis->setTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->yAxis->setSubTickPen(QPen(QColor(212,211,211)));
        ui->diffplot_pos->yAxis->grid()->setPen(QPen(QColor(QColor(212,211,211)), 1, Qt::DotLine));		
        ui->diffplot_pos->xAxis->grid()->setPen(QPen(QColor(QColor(212,211,211)), 1, Qt::DotLine));
		ui->diffplot_pos->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
        ui->diffplot_pos->graph(0)->setLineStyle(QCPGraph::lsLine); 
		ui->diffplot_pos->xAxis->setLabelFont(label);
		ui->diffplot_pos->yAxis->setLabelFont(label);
	
    }

}

void StatisticsPage::updatePlot()
{
    static int64_t lastUpdate = 0;
    // Double Check to make sure we don't try to update the plot when it is disabled
    if(!GetBoolArg("-chart", true)) { return; }
    if (GetTime() - lastUpdate < 600) { return; } // This is just so it doesn't redraw rapidly during syncing

    if(fDebug) { printf("Plot: Getting Ready: pindexBest: %p\n", pindexBest); }
    	
		bool fProofOfStake = (nBestHeight > 1);
    if (fProofOfStake)
        ui->diffplot_weight->yAxis->setLabel("Network Weight");
		else
        ui->diffplot_weight->yAxis->setLabel("Network Weight");

    int numLookBack = 500;
    double diffMax = 0;
    const CBlockIndex* pindex = GetLastBlockIndex(pindexBest, fProofOfStake);
    int height = pindex->nHeight;
    int xStart = std::max<int>(height-numLookBack, 0) + 1;
    int xEnd = height;

    // Start at the end and walk backwards
    int i = numLookBack-1;
    int x = xEnd;

    // This should be a noop if the size is already 2000
    vX.resize(numLookBack);
    vY.resize(numLookBack);

    if(fDebug) {
        if(height != pindex->nHeight) {
            printf("Plot: Warning: nBestHeight and pindexBest->nHeight don't match: %d:%d:\n", height, pindex->nHeight);
        }
    }

    if(fDebug) { printf("Plot: Reading blockchain\n"); }

    const CBlockIndex* itr = pindex;
    while(i >= 0 && itr != NULL)
    {
        if(fDebug) { printf("Plot: Processing block: %d - pprev: %p\n", itr->nHeight, itr->pprev); }
        vX[i] = itr->nHeight;
        if (itr->nHeight < xStart) {
        	xStart = itr->nHeight;
        }
        vY[i] = fProofOfStake ? GetPoSKernelPS(itr) : GetDifficulty(itr);
        diffMax = std::max<double>(diffMax, vY[i]);

        itr = GetLastBlockIndex(itr->pprev, fProofOfStake);
        i--;
        x--;
    }

    if(fDebug) { printf("Plot: Drawing plot\n"); }

    ui->diffplot_weight->graph(0)->setData(vX, vY);

    // set axes ranges, so we see all data:
    ui->diffplot_weight->xAxis->setRange((double)xStart, (double)xEnd);
    ui->diffplot_weight->yAxis->setRange(0, diffMax+(diffMax/10));

    ui->diffplot_weight->xAxis->setAutoSubTicks(false);
    ui->diffplot_weight->yAxis->setAutoSubTicks(false);
    ui->diffplot_weight->xAxis->setSubTickCount(0);
    ui->diffplot_weight->yAxis->setSubTickCount(0);

    ui->diffplot_weight->replot();
	
	    diffMax = 0;

    // Start at the end and walk backwards
    i = numLookBack-1;
    x = xEnd;
	// This should be a noop if the size is already 2000
    vX3.resize(numLookBack);
    vY3.resize(numLookBack);

    CBlockIndex* itr3 = pindex;

    while(i >= 0 && itr3 != NULL)
    {
        vX3[i] = itr3->nHeight;
        vY3[i] = fProofOfStake ? GetDifficulty(itr3) : GetPoSKernelPS(itr3);
        diffMax = std::max<double>(diffMax, vY3[i]);

        itr3 = GetLastBlockIndex(itr3->pprev, fProofOfStake);
        i--;
        x--;
    }

    ui->diffplot_pos->graph(0)->setData(vX3, vY3);

    // set axes ranges, so we see all data:
    ui->diffplot_pos->xAxis->setRange((double)xStart, (double)xEnd);
    ui->diffplot_pos->yAxis->setRange(0, diffMax+(diffMax/10));

    ui->diffplot_pos->xAxis->setAutoSubTicks(false);
    ui->diffplot_pos->yAxis->setAutoSubTicks(false);
    ui->diffplot_pos->xAxis->setSubTickCount(0);
    ui->diffplot_pos->yAxis->setSubTickCount(0);

    ui->diffplot_pos->replot();

    

    if(fDebug) { printf("Plot: Done!\n"); }
    	
    lastUpdate = GetTime();
}

int heightPrevious = -1;
int connectionPrevious = -1;
int volumePrevious = -1;
double netPawratePrevious = -1;
double hardnessPrevious = -1;
double hardnessPrevious2 = -1;
int stakeminPrevious = -1;
int stakemaxPrevious = -1;
QString stakecPrevious = "";
QString percPrevious = "";

void StatisticsPage::updateStats()
{
	int unit = model->getOptionsModel()->getDisplayUnit();
    double pHardness2 = GetDifficulty(GetLastBlockIndex(pindexBest, true));
    int nHeight = pindexBest->nHeight;
    uint64_t nNetworkWeight = GetPoSKernelPS();
    double volume = pindexBest->nMoneySupply;
    int peers = this->model2->getNumConnections();
    QString height = QString::number(nHeight);
    QString stakemax = QString::number(nNetworkWeight);
    QString hardness2 = QString::number(pHardness2, 'f', 8);
	QString Qlpawrate = model2->getLastBlockDate().toString();
    QString QPeers = QString::number(peers);
    QString qVolume = BitcoinUnits::formatWithUnit(unit, volume);
	
    if(nHeight > heightPrevious)
    {
        ui->heightBox->setText("" + height + "");
    } else {
    ui->heightBox->setText(height);
    }

    if(0 > stakemaxPrevious)
    {
        ui->maxBox->setText("" + stakemax + "");
    } else {
    ui->maxBox->setText(stakemax);
    }

    if(pHardness2 > hardnessPrevious2)
    {
        ui->diffBox2->setText("" + hardness2 + "");
    } else if(pHardness2 < hardnessPrevious2) {
        ui->diffBox2->setText("" + hardness2 + "");
    } else {
        ui->diffBox2->setText(hardness2);
    }
    	
	if(Qlpawrate != pawratePrevious)
    {
        ui->localBox->setText("" + Qlpawrate + "");
    } else {
    ui->localBox->setText(Qlpawrate);
    }

    if(peers > connectionPrevious)
    {
        ui->connectionBox->setText("" + QPeers + "");             
    } else if(peers < connectionPrevious) {
        ui->connectionBox->setText("" + QPeers + "");        
    } else {
        ui->connectionBox->setText(QPeers);  
    }

    if(volume > volumePrevious)
    {
        ui->volumeBox->setText("" + qVolume + "");
    } else if(volume < volumePrevious) {
        ui->volumeBox->setText("" + qVolume + "");
    } else {
        ui->volumeBox->setText(qVolume);
    }
    updatePrevious(nHeight, nNetworkWeight, pHardness2, Qlpawrate, peers, volume);
}

void StatisticsPage::updatePrevious(int nHeight, int nNetworkWeight, double pHardness2, QString Qlpawrate, int peers, double volume)
{
    heightPrevious = nHeight;	
    stakemaxPrevious = nNetworkWeight;
    hardnessPrevious2 = pHardness2;
	pawratePrevious = Qlpawrate;
    connectionPrevious = peers;
    volumePrevious = volume;
}

void StatisticsPage::setModel(ClientModel *model)
{
    //updateStatistics();
    this->model = model;
}

void StatisticsPage::setModel2(ClientModel *model2)
{
    //updateStatistics();
    this->model2 = model2;
}

void StatisticsPage::showOutOfSyncWarning(bool fShow)
{
	ui->labelNetworkStatus->setVisible(fShow);
	ui->labelNetworkStatusCharts->setVisible(fShow);
}

StatisticsPage::~StatisticsPage()
{
    delete ui;
}
