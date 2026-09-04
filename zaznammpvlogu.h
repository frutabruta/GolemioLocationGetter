#ifndef ZAZNAMMPVLOGU_H
#define ZAZNAMMPVLOGU_H
#include <QObject>
#include <QMap>

class ZaznamMpvLogu
{
public:
    ZaznamMpvLogu();

    QMap<QString, QString>  obsah;
    QVector<QString> toQVectorQString(QVector<QString> hlavicka);

    QString vypis();
    QString vypisCsvRadek(QVector<QString> hlavicka);
    static QString escape(QString vstup);
    static QString vypisCsvHlavicka(QVector<QString> hlavicka);

private:

};

#endif // ZAZNAMMPVLOGU_H
