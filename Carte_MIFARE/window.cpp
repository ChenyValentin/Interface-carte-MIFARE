#include "window.h"
#include "ui_window.h"
#include <QtGui>

#include "MfErrNo.h"
#include "Sw_Device.h"
#include "Sw_ISO14443A-3.h"
#include "Sw_Mf_Classic.h"
#include "Tools.h"
#include "TypeDefs.h"

window::window(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::window)
{
    ui->setupUi(this);
}

window::~window()
{
    delete ui;
}

ReaderName MonLecteur;

void window::on_exitButton_clicked()
{
    int16_t status = MI_OK;
    RF_Power_Control(&MonLecteur, FALSE, 0);
    status = LEDBuzzer(&MonLecteur, LED_OFF);
    status = CloseCOM(&MonLecteur);
    qApp->quit();
}


void window::on_connectButton_clicked()
{
    int16_t status = MI_OK;
    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    status = OpenCOM(&MonLecteur);
    qDebug() << "OpenCOM" << status;
    status = Version(&MonLecteur);
    ui->display->setText(MonLecteur.version);
    ui->display->update();
}

void window::on_saisieButton_clicked()
{
    QString Text = ui->inputArea->toPlainText();
    qDebug() << "Text : " << Text;
}

