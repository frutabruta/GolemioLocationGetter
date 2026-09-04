#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , downloaderPublic("")
    , downloaderGtfs("")
    , downloaderDepartures("")
    , qSettings(QApplication::applicationDirPath()+"/settings.ini", QSettings::IniFormat)
{
    ui->setupUi(this);


    connect(&downloaderPublic,&GolemioPublicVehiclePositions::signalReceivedData,this,&MainWindow::slotDataPublicReceived);
    connect(&downloaderPublic,&GolemioPublicVehiclePositions::signalDataParsed,this,&MainWindow::slotPublicParsedDataReceived);

    connect(&downloaderGtfs,&GolemioVehiclePositions::signalReceivedData,this,&MainWindow::slotDataGtfsReceived);
    connect(&downloaderGtfs,&GolemioVehiclePositions::signalDataParsed,this,&MainWindow::slotGtfsParsedDataReceived);

    connect(&downloaderDepartures,&GolemioDepartureBoardsV4::signalReceivedData,this,&MainWindow::slotDataTransferBoardsReceived);

    connect(&secondsTimer,&QTimer::timeout,this,&MainWindow::slotSecondsTimer);
    connect(&downloadTimer,&QTimer::timeout,this,&MainWindow::slotDownloadTimer);

    ui->lineEdit_key->setText(qSettings.value("golemio/apiKey").toString());
    setKeys(ui->lineEdit_key->text().toLatin1());


    sqLiteBase.dbFilePath=QApplication::applicationDirPath()+"/data.sqlite";

}

MainWindow::~MainWindow()
{
    sqLiteBase.dbClose();
    delete ui;
}

void MainWindow::slotSecondsTimer()
{
    ui->label_timerValue->setText(QString::number(downloadTimer.remainingTime()/1000));
}

void MainWindow::slotDownloadTimer()
{
    on_pushButton_startRequestPublic_clicked();
}

void MainWindow::on_pushButton_startRequestGtfs_clicked()
{
    ui->plainTextEdit_error->clear();
    ui->plainTextEdit_resultGtfs->clear();

    downloaderGtfs.startDataDownload(ui->lineEdit_gtfsTripId->text());
}


void MainWindow::on_pushButton_startRequestPublic_clicked()
{
    ui->plainTextEdit_error->clear();
    ui->plainTextEdit_result->clear();
    ui->label_parsedResult->clear();

    setWindowTitle("GLC "+ui->lineEdit_vehicleId->text().replace("service-",""));
    //downloader.startDataDownload(ui->lineEdit_vehicleId->text()+"?scopes=info&scopes=stop_times&scopes=vehicle_descriptor");
    downloaderPublic.startDataDownload(ui->lineEdit_vehicleId->text()+"?scopes=info&scopes=vehicle_descriptor");
}



void MainWindow::on_pushButton_setKey_clicked()
{
    setKeys(ui->lineEdit_key->text().toLatin1());
}

void MainWindow::setKeys(QByteArray key)
{
    downloaderPublic.setKey(key);
    downloaderGtfs.setKey(key);
    downloaderDepartures.setKey(key);

    qSettings.setValue("golemio/apiKey",QString(key));
}


void MainWindow::slotDataPublicReceived(QByteArray data)
{
    ui->plainTextEdit_result->setPlainText(data);

    if(ui->checkBox_relayToTester->isChecked())
    {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
        sendCoordinates(parseCoordinates(jsonDocument),ui->lineEdit_portNumber->text().toUInt());
    }
}

void MainWindow::slotDataGtfsReceived(QByteArray data)
{
    ui->plainTextEdit_resultGtfs->setPlainText(data);

    QJsonDocument dataJson=QJsonDocument::fromJson(data);

    QString stopId=gtfsStopIdToAswId(getNextStopIdFromJson(dataJson));
    qDebug()<<stopId;

    ui->lineEdit_transferBoard->setText(transferBoardsIdFromStopIdAndVehicleString(stopId,ui->lineEdit_vehicleId->text()));

    if(ui->checkBox_transferBoards->isChecked())
    {
        on_pushButton_startRequestTransferBoards_clicked();
    }
}

void MainWindow::slotDataTransferBoardsReceived(QByteArray data)
{
    ui->plainTextEdit_resultTransferBoards->setPlainText(data);

    saveDataToDB();
}

void MainWindow::slotPublicParsedDataReceived(VehiclePositionResultPublic data)
{
    ui->label_parsedResult->setText(data.dumpToQString());

    if(!data.gtfsTripId.isEmpty())
    {
        ui->lineEdit_gtfsTripId->setText(data.gtfsTripId);
        if(ui->checkBox_directRequest->isChecked())
        {
            on_pushButton_startRequestGtfs_clicked();
        }
    }
}

void MainWindow::slotGtfsParsedDataReceived(VehiclePositionResult data)
{
    // ui->label_parsedResult->setText(data.dumpToQString());
}

void MainWindow::slotErrorReceived(QString errorText)
{
    ui->plainTextEdit_error->setPlainText(errorText);
}

QPointF MainWindow::parseCoordinates(QJsonDocument &inputJson)
{
    QPointF coordinates(inputJson["geometry"]["coordinates"][0].toDouble(),inputJson["geometry"]["coordinates"][1].toDouble());
    return coordinates;
}

void MainWindow::sendCoordinates(QPointF coordinates, qint16 port)
{
    QString data=QString("<GNSSLocationService.Data>"
"    <latitude>"
"        <Degree>"
"                           <Value>%1</Value>"
"        </Degree>"
"        <Direction>"
"            <Value>N</Value>"
"        </Direction>"
"    </latitude>"
"    <longitude>"
"        <Degree>"
"            <Value>%2</Value>"
"        </Degree>"
"        <Direction>"
"            <Value>E</Value>"
"        </Direction>"
"    </longitude>"
"    <GNSSType>MixedGNSSTypes</GNSSType>"
                           "</GNSSLocationService.Data>").arg(QString::number(coordinates.y()),QString::number(coordinates.x()));

    client.odesliRaw("127.0.0.1", data,port);
}



QString MainWindow::getNextStopIdFromJson(QJsonDocument inputJson)
{
    return inputJson["properties"]["last_position"]["next_stop"]["id"].toString();
}
    /*
    gtfsTripId=mVstupniJson["gtfs_trip_id"].toString();
    routeType=mVstupniJson["route_type"].toString();
    routeShortName=mVstupniJson["route_short_name"].toString();
    originRouteName=mVstupniJson["origin_route_name"].toString();
    runNumber=mVstupniJson["run_number"].toInt();
    tripHeadsign=mVstupniJson["trip_headsign"].toString();
    //result.=mVstupniJson[""].toString();
    coordinates=geometryToQPointF(mVstupniJson["geometry"]);
    */


QString MainWindow::gtfsStopIdToAswId(QString input)
{
    QString output;

    input.replace("U","");
    input.replace("P","");

    output=input.split("Z").value(0)+"_"+input.split("Z").value(1);


    return output;
}

QString MainWindow::transferBoardsIdFromStopIdAndVehicleString(QString stopId,QString vehicleString)
{
    QString output;

    QStringList vehicleSplit=vehicleString.split("-");

    if(vehicleSplit.count()==3)
    {
        output="?aswId="+stopId+"&vehicleRegistrationNumber="+vehicleSplit.value(2)+"&routeType="+vehicleSplit.value(1);
    }
    else
    {
        qDebug()<<"invalid vehicle id";
    }

    return output;
}




void MainWindow::on_pushButton_startRequestTransferBoards_clicked()
{
    ui->plainTextEdit_resultTransferBoards->clear();
    downloaderDepartures.startDataDownload(ui->lineEdit_transferBoard->text());
}



int MainWindow::saveDataToDB()
{
    qDebug()<<Q_FUNC_INFO;

    int counter=0;

   sqLiteBase.dbFilePath=QApplication::applicationDirPath()+"/"+ui->lineEdit_vehicleId->text()+".sqlite";

  //  sqLiteZaklad.cestaKomplet=QApplication::applicationDirPath()+"/data2.sqlite";
    sqLiteBase.initialize();

    QStringList hlavicka;
    hlavicka<<"timestamp";
    hlavicka<<"vehicleId";
    hlavicka<<"gtfsTrip";
    hlavicka<<"transferUrl";
    hlavicka<<"publicRequest";
    hlavicka<<"gtfsRequest";
    hlavicka<<"transferBoardRequest";

    sqLiteBase.tableCreate("data",hlavicka);


    vlozRadekDatNew("data",
                    ui->lineEdit_vehicleId->text(),
                    ui->lineEdit_gtfsTripId->text(),
                    ui->lineEdit_transferBoard->text(),
                    ui->plainTextEdit_result->toPlainText(),
                    ui->plainTextEdit_resultGtfs->toPlainText(),
                    ui->plainTextEdit_resultTransferBoards->toPlainText());


    sqLiteBase.dbClose();

    /*
    QVector<QString> hlavicka;

    hlavicka<<"timestamp";
    hlavicka<<"publicRequest";
    hlavicka<<"gtfsRequest";
    hlavicka<<"transferBoardRequest";




    if(!sqLiteZaklad.zahajTransakci())
    {
        qDebug()<<"transakci se nepovedlo zahajit";
    }
    //   qDebug()<<"soubor ma "<<counter<<" radku";
    sqLiteZaklad.zrusSqlTabulku("vozidlo",hlavicka);
    sqLiteZaklad.zalozSqlTabulku("vozidlo",hlavicka);


    counter++;




    ZaznamMpvLogu zaznam;

    zaznam.obsah.insert("timestamp",QDateTime::currentDateTime().toString());
    zaznam.obsah.insert("publicRequest",ui->plainTextEdit_result->toPlainText());
    zaznam.obsah.insert("gtfsRequest",ui->plainTextEdit_resultGtfs->toPlainText());
    zaznam.obsah.insert("transferBoardRequest",ui->plainTextEdit_resultTransferBoards->toPlainText());


    sqLiteZaklad.vlozRadekDat("vozidlo",hlavicka,zaznam.toQVectorQString(hlavicka));


    //  qApp->processEvents();


    sqLiteZaklad.ukonciTransakci();
    sqLiteZaklad.zavriDB();


    qDebug()<<"konec soubornaRadky";

*/
    return counter;
}



bool MainWindow::vlozRadekDatNew(QString tableName, QString vehicleId, QString gtfsTrip, QString transferUrl, QString publicRequest, QString gtfsRequest, QString transferBoardRequest )
{

        QSqlQuery query;
        query.prepare("INSERT INTO "+tableName+" "
                    "( timestamp, vehicleId, gtfsTrip, transferUrl, publicRequest, gtfsRequest, transferBoardRequest ) "
                    " VALUES ( :timestamp, :vehicleId, :gtfsTrip, :transferUrl, :publicRequest, :gtfsRequest, :transferBoardRequest)  ");

        query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate) );
        query.bindValue(":vehicleId", vehicleId );
        query.bindValue(":gtfsTrip", gtfsTrip );
        query.bindValue(":transferUrl", transferUrl );
        query.bindValue(":publicRequest", publicRequest);
        query.bindValue(":gtfsRequest", gtfsRequest );
        query.bindValue(":transferBoardRequest",transferBoardRequest );

        sqLiteBase.executeQuery(query);


        return true;
}





void MainWindow::on_checkBox_timer_stateChanged(int arg1)
{
    if(arg1)
    {
        qDebug()<<"timer started";
        secondsTimer.setInterval(1000);
        secondsTimer.start();
        downloadTimer.setInterval(ui->spinBox_timer->value()*1000);
        downloadTimer.start();
        on_pushButton_startRequestPublic_clicked();

    }
    else
    {
        qDebug()<<"timer stopped";
        downloadTimer.stop();
        secondsTimer.stop();
    }
}

