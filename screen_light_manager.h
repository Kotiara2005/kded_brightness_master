#ifndef SCREEN_LIGHT_MANAGER_H
#define SCREEN_LIGHT_MANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QTimer>
#include <QAmbientLightSensor>
#include <QLightSensor>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTextStream>


//#define _DIAGNOSTIC
#define _AMBL_BRIGHTNESS_COUNT 5

class Screen_light_manager : public QObject
{
    Q_OBJECT
public:
    explicit Screen_light_manager(QObject *parent = nullptr);
    ~Screen_light_manager();

    void        set_change_brightness_silent(const bool& silent);
    bool        get_change_brightness_silent();
    void        set_brightness_manage_on(const bool& swch);
    const bool& get_brightness_manage_on();
    void        set_ambl_or_light_sensor(const bool& mbl_sensor_switch);
    const bool& get_ambl_or_light_sensor();
    QString     reread_config();
#ifdef _DIAGNOSTIC
    QString get_diagnostic();
#endif
public slots:
    void get_validate_config();
    void get_brightness();
private:
    void sensor_start();
    void change_brightness();
    void ambl_change_brightness();
    QString read_config(int& answer);
    QString validate_config(int& answer);
    void save_to_config(int& answer);
    void sensors_switch();
private:
    QLightSensor* p_light_sensor;
    QAmbientLightSensor* p_mb_light_sensor;
    unsigned int count;
    unsigned int max_lux;
    unsigned int config_max_lux;
    unsigned int max_screen_brightness;
    unsigned int min_screen_brightness;
    unsigned int sensor_frequency;
    unsigned int initialization_delay;
    unsigned int initialization_attempts;
    unsigned int initialization_attempts_limit;
    double light_sensor_percent;
    double light_screen_percent;
    double prev_brightness;
    bool swch;
    bool mbl_sensor_switch;
    int ambl_brightness_persents[_AMBL_BRIGHTNESS_COUNT];
    QString kde_service_function;
#ifdef _DIAGNOSTIC
    QString sensors_state;
    //QString dbus_state;
    QString read_config_state;
    QString validate_state;
    QString save_too_config_state;
    QString ambl_change_brightness_state;
    QString change_brightness_state;
    QString constructor_state;
#endif
};

#endif // SCREEN_LIGHT_MANAGER_H
