#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <libebmbus/ebmbus.h>
#include "revpidio.h"
#include "daisychaininterface.h"
#include "lightbutton.h"
#include "uninterruptiblepowersupply.h"
#include "operatingsystemcontrol.h"

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);

private:
    QList<EbmBus*> m_ebmbuslist;

    QList<DaisyChainInterface*> m_dcilist;

    RevPiDIO m_io;

    LightButton* m_lightbutton_operation;
    LightButton* m_lightbutton_error;
    LightButton* m_lightbutton_speed_0;
    LightButton* m_lightbutton_speed_50;
    LightButton* m_lightbutton_speed_100;

    UninterruptiblePowerSupply* m_ups;
    OperatingSystemControl* m_osControl;

    QTimer m_timer;

    int m_speed;

private slots:
    void slot_timer_fired();

    void slot_button_operation_clicked();
    void slot_button_error_clicked();
    void slot_button_speed_0_clicked();
    void slot_button_speed_50_clicked();
    void slot_button_speed_100_clicked();

    void slot_showResponse(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);

    void slot_shutdownNOW();
};

#endif // MAINCONTROLLER_H
