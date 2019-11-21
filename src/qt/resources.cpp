#include "resources.h"
#include "ui_resources.h"

#include "bitcoinunits.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QScrollArea>
#include <QScroller>

ReSources::ReSources(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReSources)
{
    ui->setupUi(this);

    
}

ReSources::~ReSources()
{
    delete ui;
}