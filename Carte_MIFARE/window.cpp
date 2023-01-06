#include "window.h"
#include "ui_window.h"
#include <QtGui>

#include "MfErrNo.h"
#include "Sw_Device.h"
#include "Sw_ISO14443A-3.h"
#include "Sw_Mf_Classic.h"
#include "Tools.h"
#include "TypeDefs.h"

uint8_t atq[2];
uint8_t sak[1];
uint8_t uid[12];
uint16_t uid_len = 12;
uint8_t sect_count = 0;
uint8_t data[240] = {0};
uint8_t bloc, sect;
uint8_t offset;

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

int card_read(BYTE sect_count);

uint8_t key_sec2Lec[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
uint8_t key_sec2Ecr[6] = { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5 };

uint8_t key_sec3Lec[6] = { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5 };
uint8_t key_sec3Ecr[6] = { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5 };

ReaderName MonLecteur;

void window::on_exitButton_clicked()
{
    int16_t status = 0;
    RF_Power_Control(&MonLecteur, FALSE, 0);
    status = LEDBuzzer(&MonLecteur, LED_OFF);
    status = CloseCOM(&MonLecteur);
    qApp->quit();
}


void window::on_connectButton_clicked()
{
    int16_t status = 0;

    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    status = OpenCOM(&MonLecteur);
    qDebug() << "OpenCOM" << status;

    status = Version(&MonLecteur);
    ui->display->setText(MonLecteur.version);
    ui->display->update();

    status = LEDBuzzer(&MonLecteur, LED_RED_ON);
    if(status != 0)
    {
        status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
        qDebug() << "LED [FAILED]";
        qApp -> quit();
    }
    RF_Power_Control(&MonLecteur, TRUE, 0); //Alimente le champ EM du lecteur pour pouvoir lire les cartes

    //To start
    status = ISO14443_3_A_Halt(&MonLecteur);
    if (status != 0){
        printf("Failed to halt the tag\n");
        qApp -> quit();
    }
    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_RED_ON);
    DELAYS_MS(1);
    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);
}




void window::on_ReadBtn_clicked()
{
    int16_t status = 0;
    uint8_t data[240] = {0};

    qDebug() << ("Waiting card !\n");
    while(ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len))
    {
        QApplication::processEvents();
    }

    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON + LED_GREEN_ON);
    DELAYS_MS(1);
    status = LEDBuzzer(&MonLecteur, LED_RED_ON);

    if (status != 0){
        status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
        qDebug() << ("No available tag in RF field\n");
    }

    //Last name read
     sect = 2;
     bloc = 2;
     qDebug() << "Reading sector : " << sect;
     memset(data, 0x00, 240);
     status = Mf_Classic_Read_Sector(&MonLecteur, TRUE, sect, data, AuthKeyA, 2);

     QString LastNameText;
     for (offset = 0; offset < 16; offset++){
         if (data[16 * bloc + offset] >= ' '){
             LastNameText += (char)data[16 * bloc + offset];
         }
     }
     qDebug() << "Last name : " << LastNameText;
     ui->LastNameText->setText(LastNameText);
     ui->LastNameText->update();

     //First Name read
     sect = 2;
     bloc = 1;
     memset(data, 0x00, 240);
     status = Mf_Classic_Read_Sector(&MonLecteur, TRUE, sect, data, AuthKeyA, 2);
     QString FirstNameText;
     for (offset = 0; offset < 16; offset++){
         if (data[16 * bloc + offset] >= ' '){
             FirstNameText += (char)data[16 * bloc + offset];
         }
     }
    qDebug() << "First name : " << FirstNameText;
    ui->FirstNameText->setText(FirstNameText);
    ui->FirstNameText->update();

    //Number read
    sect = 3;
    bloc = 2;
    memset(data, 0x00, 240);
    status = Mf_Classic_Read_Sector(&MonLecteur, TRUE, sect, data, AuthKeyA, 3);

    uint32_t UnitsNumber;
    int16_t statut=Mf_Classic_Read_Value(&MonLecteur,TRUE,14,&UnitsNumber,AuthKeyA,3);

    qDebug() << "UnitsNumber : " << UnitsNumber;
    QString UnitsNumber_text=QString::number(UnitsNumber);
    qDebug()<<UnitsNumber_text;
    ui->UnitsNumber->setText(UnitsNumber_text);
    ui->UnitsNumber->update();

    if ((sak[0] & 0x1F) == 0x08){
        // Mifare classic 1k : 16 sectors, 3+1 blocks in each sector
        qDebug() << ("Tag appears to be a Mifare classic 1k\n");
        sect_count = 16;
    } else if ((sak[0] & 0x1F) == 0x18){
        // Mifare classic 4k : 40 sectors, 3+1 blocks in 32-first sectors, 15+1 blocks in the 8-last sectors
        qDebug() << ("Tag appears to be a Mifare classic 4k\n");
        sect_count = 40;
    }


    DELAYS_MS(1000);
    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);
}

void window::displayUnits(uint32_t UnitsNumber){
    int16_t status = MI_OK;

    qDebug() << "UnitsNumber : " << UnitsNumber;
    QString UnitsNumber_text=QString::number(UnitsNumber);
    ui->UnitsNumber->setText(UnitsNumber_text);
    ui->UnitsNumber->update();
}

void window::on_WriteBtn_clicked()
{
    int16_t status = 0;
    uint8_t data[240] = {0};

    int offset;

    QString lastname_update = ui->LastNameText->toPlainText();
    QString firstname_update = ui->FirstNameText->toPlainText();

    //Surname write
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if (status == 0){
        status = LEDBuzzer(&MonLecteur, LED_RED_ON);
        QByteArray array_surname = lastname_update.toUtf8();
        for (offset = 0; offset < 16; offset++){
            data[offset] = array_surname[offset];
        }
        status = Mf_Classic_Write_Block(&MonLecteur, TRUE, 10, data, AuthKeyB, 2);
    }
    else{
        qDebug() << "Writing error";
    }

    // Name write
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if (status == 0){
        QByteArray array_name = firstname_update.toUtf8();
        for (offset = 0; offset < 16; offset++){
            data[offset] = array_name[offset];
        }
        status = Mf_Classic_Write_Block(&MonLecteur, TRUE, 9, data, AuthKeyB, 2);
    }
    else{
        qDebug() << "Writing error";
        status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    }

    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);

}


void window::on_SellBtn_clicked()
{
    int16_t status = 0;
    int increment=ui->SellUnits->value();

    uint32_t count;
    int16_t statut=Mf_Classic_Read_Value(&MonLecteur,TRUE,14,&count,AuthKeyA,3);
    status = LEDBuzzer(&MonLecteur, LED_RED_ON);
    statut=Mf_Classic_Increment_Value(&MonLecteur,TRUE,14,increment,14,AuthKeyB,3);
    ui->SellUnits->setValue(0);
    this->on_ReadBtn_clicked();
    statut=Mf_Classic_Write_Value(&MonLecteur,TRUE,13,count+increment,AuthKeyB,3);
    uint32_t backup;
    statut=Mf_Classic_Read_Value(&MonLecteur,TRUE,13,&backup,AuthKeyA,3);
    qDebug() << "Backup count : " << backup;

    DELAYS_MS(1000);
    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);

}


void window::on_LoadBtn_clicked()
{
    int16_t status = 0;
    int decrement=ui->LoadUnits->value();
    uint32_t count;
    int16_t statut=Mf_Classic_Read_Value(&MonLecteur,TRUE,14,&count,AuthKeyA,3);
    status = LEDBuzzer(&MonLecteur, LED_RED_ON);
    statut=Mf_Classic_Decrement_Value(&MonLecteur,TRUE,14,decrement,14,AuthKeyA,3);
    ui->LoadUnits->setValue(0);
    this->on_ReadBtn_clicked();
    statut=Mf_Classic_Write_Value(&MonLecteur,TRUE,13,count-decrement,AuthKeyB,3);
    uint32_t backup;
    statut=Mf_Classic_Read_Value(&MonLecteur,TRUE,13,&backup,AuthKeyA,3);
    qDebug() << "Backup count : " << backup;
}
