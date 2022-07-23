#include "screen_light_manager_dbus_connector.h"

Screen_light_manager_Dbus_connector::Screen_light_manager_Dbus_connector(QObject *parent, const QVariantList &) : KDEDModule(parent)
{
    //////////////////////////////////////////////////////////
    ///Регистрация этого обьекта в D-Bus
    //////////////////////////////////////////////////////////////
    if(QDBusConnection::sessionBus().registerService(_SERVICE_NAME))
    {
#ifdef _DIAGNOSTIC
        this->constructor_state+="D-Bus service is registred\n";
        if(QDBusConnection::sessionBus().registerObject("/org/kde/screen/light/manager", this, QDBusConnection::ExportNonScriptableSlots))
            this->constructor_state+="D-Bus Object is registred\n";
        else
            this->constructor_state+="D-Bus Object is not registred\n";
#else
        QDBusConnection::sessionBus().registerObject("/org/kde/screen/light/manager", this, QDBusConnection::ExportNonScriptableSlots);
#endif
     }
#ifdef _DIAGNOSTIC
     else
        this->constructor_state+="D-Bus service is not registred\n";
#endif
    ////////////////////////////////////////////////////////////////////////
    /// Конец регистрации сервиса
    ////////////////////////////////////////////////////////////////////////
    this->scr_light = new Screen_light_manager(this);
}

Screen_light_manager_Dbus_connector::~Screen_light_manager_Dbus_connector()
{
    QDBusConnection::sessionBus().unregisterObject("/org/kde/screen/light/manager", QDBusConnection::UnregisterNode);
    QDBusConnection::sessionBus().unregisterService(_SERVICE_NAME);
}

void Screen_light_manager_Dbus_connector::set_change_brightness_silent(bool silent)
{
    this->scr_light->set_change_brightness_silent(silent);
}

bool Screen_light_manager_Dbus_connector::get_change_brightness_silent()
{
    return this->scr_light->get_change_brightness_silent();
}

void Screen_light_manager_Dbus_connector::set_brightness_manage_on(bool swch)
{
    this->scr_light->set_brightness_manage_on(swch);
}

int  Screen_light_manager_Dbus_connector::get_brightness_manage_on()
{
    return this->scr_light->get_brightness_manage_on();
}

void Screen_light_manager_Dbus_connector::set_ambl_or_light_sensor(bool mbl_sensor_switch)
{
    this->scr_light->set_ambl_or_light_sensor(mbl_sensor_switch);
}


bool Screen_light_manager_Dbus_connector::get_ambl_or_light_sensor()
{
    return this->scr_light->get_ambl_or_light_sensor();
}

QString Screen_light_manager_Dbus_connector::reread_config()
{
    return this->scr_light->reread_config();
}

#ifdef _DIAGNOSTIC
QString Screen_light_manager_Dbus_connector::get_diagnostic()
{
    return this->scr_light->get_diagnostic();
}
#endif

K_PLUGIN_FACTORY(ScreenLightManagerFactory,
                 registerPlugin<Screen_light_manager_Dbus_connector>();)
#include "screen_light_manager_dbus_connector.moc"

