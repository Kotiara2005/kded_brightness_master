#ifndef SCREEN_LIGHT_MANAGER_DBUS_CONNECTOR_H
#define SCREEN_LIGHT_MANAGER_DBUS_CONNECTOR_H

#include <KDEDModule>
#include <KPluginFactory>
#include "screen_light_manager.h"


#define _SERVICE_NAME "org.kde.screen.light.manager"

class Screen_light_manager_Dbus_connector : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", _SERVICE_NAME)
public:
    explicit Screen_light_manager_Dbus_connector(QObject *parent, const QVariantList &);
    ~Screen_light_manager_Dbus_connector();
public slots:
    void set_change_brightness_silent(bool silent);
    bool get_change_brightness_silent();
    void set_brightness_manage_on(bool swch);
    int  get_brightness_manage_on();
    void set_ambl_or_light_sensor(bool mbl_sensor_switch);
    bool get_ambl_or_light_sensor();
    QString reread_config();
#ifdef _DIAGNOSTIC
    QString get_diagnostic();
#endif
private:
    Screen_light_manager* scr_light;
#ifdef _DIAGNOSTIC
    QSring constructor_state;
#endif
};

#endif // SCREEN_LIGHT_MANAGER_DBUS_CONNECTOR_H

