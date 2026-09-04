#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GolemioClient/golemiopublicvehiclepositions.h"
#include "GolemioClient/golemiovehiclepositions.h"
#include "GolemioClient/golemiodepartureboardsv4.h"

#include "XmlRopidImportStream/sqlitebase.h"

#include "UdpSender/udpsender.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_startRequestGtfs_clicked();
    void on_pushButton_startRequestPublic_clicked();
    void on_pushButton_setKey_clicked();

    void slotDataPublicReceived(QByteArray data);
    void slotErrorReceived(QString errorText);
    void slotPublicParsedDataReceived(VehiclePositionResultPublic data);
    void slotGtfsParsedDataReceived(VehiclePositionResult data);
    void slotDataGtfsReceived(QByteArray data);
    void slotDataTransferBoardsReceived(QByteArray data);
    void on_pushButton_startRequestTransferBoards_clicked();


    void slotSecondsTimer();
    void slotDownloadTimer();
    void on_checkBox_timer_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    GolemioPublicVehiclePositions downloaderPublic;
    GolemioVehiclePositions downloaderGtfs;
    GolemioDepartureBoardsV4 downloaderDepartures;

    UdpSender client;

    SqLiteBase sqLiteBase;

    QTimer downloadTimer;
    QTimer secondsTimer;

    QSettings qSettings;

    QString getNextStopIdFromJson(QJsonDocument inputJson);
    QString gtfsStopIdToAswId(QString input);
    QString transferBoardsIdFromStopIdAndVehicleString(QString stopId, QString vehicleString);
    int saveDataToDB();
    void setKeys(QByteArray key);
    bool vlozRadekDatNew(QString tableName, QString vehicleId, QString gtfsTrip, QString transferUrl, QString publicRequest, QString gtfsRequest, QString transferBoardRequest);
    void sendCoordinates(QPointF coordinates, qint16 port);
    QPointF parseCoordinates(QJsonDocument &inputJson);
};
#endif // MAINWINDOW_H
