#include "screen_light_manager.h"

Screen_light_manager::Screen_light_manager(QObject *parent) : QObject(parent)
{
#ifdef _DIAGNOSTIC
    this->count=0;
    this->constructor_state="Start constructor Screen_light_manager(QObject *parent)\n\n";
#endif
    this->p_light_sensor = nullptr;
    this->p_mb_light_sensor = nullptr;
    //Максимальная яркость по датчику
    this->max_lux=0;  //max is 3743
    this->swch=true;
    this->mbl_sensor_switch=false;
    this->config_max_lux=0;
    this->initialization_attempts=0;
    this->initialization_attempts_limit=1;
    this->initialization_delay=2000;
    int answer[2] = { 0, 0 };

    //Один процент от максимальной яркости датчика
    this->light_sensor_percent=(double)this->max_lux/100.0;
    //Максимальная яркость подсветки экрана
    this->max_screen_brightness=0;
    this->prev_brightness=0;
    this->read_config(answer[0]);
    this->validate_config(answer[1]);
    if( -1 == answer[0] || -1 == answer[1] )
        this->save_to_config(answer[0]);
}

Screen_light_manager::~Screen_light_manager()
{
    int i=0;
    this->save_to_config(i);
}


void Screen_light_manager::get_brightness()
{
    QObject::disconnect(this->p_light_sensor, &QLightSensor::readingChanged, this, &Screen_light_manager::get_brightness);
    if(!this->swch)
        return;
    this->change_brightness();
    QTimer::singleShot(this->sensor_frequency, this, &Screen_light_manager::sensor_start);
}



void Screen_light_manager::change_brightness()
{
#ifdef _DIAGNOSTIC
    this->change_brightness_state="Start the function \"change_brightness()\"\n\n";
    unsigned int lux=this->p_light_sensor->reading()->lux();
    if( lux > this->max_lux )
        this->max_lux=lux;
    unsigned int percent = (double)lux / (double)this->max_lux * 100.0;
    int brightness = percent * this->light_screen_percent;
    this->change_brightness_state+="Яркость сенсора в lux:" + QString::number(lux) + "\n";
    this->change_brightness_state+="Максимальная яркость сенсора в lux:" + QString::number(this->max_lux) + "\n";
    this->change_brightness_state+="Яркость сенсора в процентах:" + QString::number(percent) + "\n";
    this->change_brightness_state+="Яркость экрана в процентах:" + QString::number(brightness) + "\n";
#else
    int brightness = ((double)this->p_light_sensor->reading()->lux() / (double)this->max_lux * 100.0) * this->light_screen_percent;
#endif
    if((uint)brightness<this->min_screen_brightness)
        brightness=this->min_screen_brightness;
    if(brightness != this->prev_brightness)
    {
        this->prev_brightness=brightness;
        QDBusMessage msg = QDBusMessage::createMethodCall(
                    "org.freedesktop.PowerManagement",
                    "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                    "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                    this->kde_service_function);
        msg.setArguments({brightness});
#ifdef _DIAGNOSTIC
        msg=QDBusConnection::sessionBus().call(msg);
        this->change_brightness_state+="Ошибка сообщения через D-Bus: \n";
        this->change_brightness_state+=msg.errorMessage() + "\n";
        this->count++;
        this->change_brightness_state+="Количество считываний показаний сенсора:" + QString::number(this->count) + "\n";
        //this->change_brightness_state=out_text;
#else
        QDBusConnection::sessionBus().call(msg);
#endif
    }

}


void Screen_light_manager::ambl_change_brightness()
{
    QAmbientLightReading::LightLevel light_level = this->p_mb_light_sensor->reading()->lightLevel();
    int tmp=0;
#ifdef _DIAGNOSTIC
    this->ambl_change_brightness_state="Start the function \"ambl_change_brightness()\"\n\n";;
#endif
    switch(light_level)
    {
        case QAmbientLightReading::Dark:
            tmp=this->ambl_brightness_persents[0] * this->light_screen_percent;
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is Dark, screen brightness percent is:" + QString::number(tmp) + "\n";
#endif
            break;
        case QAmbientLightReading::Twilight:
            tmp=this->ambl_brightness_persents[1] * this->light_screen_percent;
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is Twilight, screen brightness percent is:" + QString::number(tmp) + "\n";
#endif
            break;
        case QAmbientLightReading::Light:
            tmp=this->ambl_brightness_persents[2] * this->light_screen_percent;
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is Light, screen brightness percent is:" + QString::number(tmp) + "\n";
#endif
            break;
        case QAmbientLightReading::Bright:
            tmp=this->ambl_brightness_persents[3] * this->light_screen_percent;
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is Bright, screen brightness percent is:" + QString::number(tmp) + "\n";
#endif
            break;
        case QAmbientLightReading::Sunny:
            tmp=this->ambl_brightness_persents[4] * this->light_screen_percent;
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is Sunny, screen brightness percent is:" + QString::number(tmp) + "\n";
#endif
            break;
        default:
#ifdef _DIAGNOSTIC
            this->ambl_change_brightness_state+="QAmbientLightSensor brightness is undefined, droping this sensor readinr\n";
#endif
            return;
    }
    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.PowerManagement",
                "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                this->kde_service_function);
    msg.setArguments({ tmp });
#ifdef _DIAGNOSTIC
    msg=QDBusConnection::sessionBus().call(msg);
    this->ambl_change_brightness_state+="Ошибка сообщения через D-Bus: \n";
    this->ambl_change_brightness_state+=msg.errorMessage() + "\n";
    this->count++;
    this->ambl_change_brightness_state+="Количество считываний показаний сенсора:" + QString::number(this->count) + "\n";
#else
    QDBusConnection::sessionBus().call(msg);
#endif
}

void Screen_light_manager::sensor_start()
{
    this->change_brightness();
    QObject::connect(this->p_light_sensor, &QLightSensor::readingChanged, this, &Screen_light_manager::get_brightness, Qt::UniqueConnection);
}

void Screen_light_manager::set_change_brightness_silent(const bool& silent)
{
    if(silent)
        this->kde_service_function="setBrightnessSilent";
    else
        this->kde_service_function="setBrightness";
}


bool Screen_light_manager::get_change_brightness_silent()
{
    if(this->kde_service_function == "setBrightnessSilent")
        return true;
    else
        return false;
}


void Screen_light_manager::set_brightness_manage_on(const bool& swch)
{
    if((this->swch=swch))
    {
        this->sensors_switch();
    }
    else
    {
        if( this->p_light_sensor != nullptr )
        {
            QObject::disconnect(this->p_light_sensor, &QLightSensor::readingChanged, this, &Screen_light_manager::get_brightness);
            this->p_light_sensor->stop();
        }
        if( this->p_mb_light_sensor != nullptr )
        {
            QObject::disconnect(this->p_mb_light_sensor, &QAmbientLightSensor::readingChanged, this, &Screen_light_manager::ambl_change_brightness);
            this->p_mb_light_sensor->stop();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// D-Bus slot function for switching between sensors
///
void Screen_light_manager::set_ambl_or_light_sensor(const bool& mbl_sensor_switch)
{
    this->mbl_sensor_switch = mbl_sensor_switch;
    this->sensors_switch();
}

///////////////////////////////////////////////////////////////////
/// Function for switching between sensors
///
void Screen_light_manager::sensors_switch()
{
    if( this->mbl_sensor_switch )
    {
#ifdef _DIAGNOSTIC
        this->sensors_state="Used QAmbientLightSensor\n";
#endif
        if( nullptr != this->p_light_sensor)
        {
#ifdef _DIAGNOSTIC
            this->sensors_state+="the existing QLightSensor is deleted\n";
#endif
            QObject::disconnect(this->p_light_sensor, &QLightSensor::readingChanged, this, &Screen_light_manager::get_brightness);
            this->p_light_sensor->stop();
            delete this->p_light_sensor;
            this->p_light_sensor=nullptr;
        }
        if( nullptr == this->p_mb_light_sensor)
        {
            this->p_mb_light_sensor = new QAmbientLightSensor(this);
        }
        this->p_mb_light_sensor->start();
        QObject::connect(this->p_mb_light_sensor, &QAmbientLightSensor::readingChanged, this, &Screen_light_manager::ambl_change_brightness, Qt::UniqueConnection);
#ifdef _DIAGNOSTIC
        this->sensors_state+="QAmbientLightSensor created and connected\n";
        this->sensors_state+="pointer to QLightSensor" + QString::number((ulong)this->p_light_sensor) + "\n";
        this->sensors_state+="pointer to QAmbientLightSensor" + QString::number((ulong)this->p_mb_light_sensor) + "\n";
#endif
    }
    else
    {
#ifdef _DIAGNOSTIC
        this->sensors_state="Used QLightSensor\n";
#endif
        if( nullptr != this->p_mb_light_sensor )
        {
#ifdef _DIAGNOSTIC
            this->sensors_state+="the existing QAmbientLightSensor is deleted\n";
#endif
            QObject::disconnect(this->p_mb_light_sensor, &QAmbientLightSensor::readingChanged, this, &Screen_light_manager::ambl_change_brightness);
            this->p_mb_light_sensor->stop();
            delete this->p_mb_light_sensor;
            this->p_mb_light_sensor = nullptr;
        }
        if( nullptr == this->p_light_sensor )
        {
            this->p_light_sensor = new QLightSensor(this);
        }
        this->p_light_sensor->start();
        QObject::connect(this->p_light_sensor, &QLightSensor::readingChanged, this, &Screen_light_manager::get_brightness, Qt::UniqueConnection);
#ifdef _DIAGNOSTIC
        this->sensors_state+="QLightSensor created and connected\n";
        this->sensors_state+="pointer to QAmbientLightSensor" + QString::number((ulong)this->p_mb_light_sensor) + "\n";
        this->sensors_state+="pointer to QLightSensor" + QString::number((ulong)this->p_light_sensor) + "\n";
#endif
    }
}

const bool& Screen_light_manager::get_ambl_or_light_sensor()
{
    return this->mbl_sensor_switch;
}


const bool& Screen_light_manager::get_brightness_manage_on()
{
    return this->swch;
}

#ifdef _DIAGNOSTIC
QString Screen_light_manager::get_diagnostic()
{
    QString out_text="Диагностика сенсора\n\n" + this->sensors_state + "\n\n";
    out_text+=this->constructor_state + "\n\n";
    out_text+=this->read_config_state + "\n\n";
    out_text+=this->validate_state + "\n\n";
    out_text+=this->save_too_config_state + "\n\n";
    out_text+=this->change_brightness_state + "\n\n";
    out_text+=this->ambl_change_brightness_state + "\n\n";
    return out_text;
}
#endif

QString Screen_light_manager::reread_config()
{
    int answer[2] = { 0 , 0 };
    QString str="Out from read config\n\n" + this->read_config(answer[0]);
    str+="Out from validate config\n\n" + this->validate_config(answer[0]);
    if( answer[0] == -1 || answer[1] == -1 )
        this->save_to_config(answer[0]);
    this->sensors_switch();
    return str;
}

QString Screen_light_manager::read_config(int& answer)
{
    QString read_config_state="Start the function \"read_config(int& answer)\"\n\n";
    this->config_max_lux=0;
    QFile config_file(QDir::homePath()+"/.config/kded_screen_brightness.config");
    if(!config_file.open(QIODevice::ReadOnly))
    {
        read_config_state="Failed to open configuration file for reading\n";
        //continue;
        if(!config_file.exists())
        {
            read_config_state+="configuration file does not exist\n";
            answer=-1;
#ifdef _DIAGNOSTIC
            this->read_config_state=read_config_state;
#endif
            return read_config_state;
        }
        answer=1;
#ifdef _DIAGNOSTIC
        this->read_config_state=read_config_state;
#endif
        return read_config_state;
    }
    answer=0;
    QTextStream read(&config_file);
    int line=0;
    while(!read.atEnd())
    {
            line++;
            QString str=read.readLine();
            if(!str.contains(':'))
                continue;
            int coment=str.indexOf('#');
            if(coment>=0)
            {
                if(coment==0)
                    continue;
                //coment--;
                int rm_lenght=(str.length() - coment)+1;
                //rm_lenght=rm_lenght-coment;
                read_config_state+="Removing Coments from string:\n";
                read_config_state+= str + "\nFron ";
                read_config_state+= QString::number(coment) + " too ";
                read_config_state+= QString::number(coment+rm_lenght) + " position\n";
                str.remove(coment,rm_lenght);
            }
            if(!str.contains(':'))
                continue;
            QStringList values, tmp_values = str.split( ':', Qt::SkipEmptyParts );
            if(tmp_values.size()<2)
            {
                read_config_state+="Error in configuration file\n";
                read_config_state+="Error in line " + QString::number(line) + " missing name or value or bouth\n";
                answer=2;
                continue;
            }
            values<<tmp_values.at(0).trimmed()<<tmp_values.at(1).trimmed();
            bool check=false;
            if(values[0]=="MAX_SENSOR_BRIGHTNESS")
            {
                unsigned int tmp=values[1].toUInt(&check);
                if(check && tmp > this->config_max_lux)
                {
                    this->config_max_lux=tmp;
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    read_config_state+="Error in configuration file \n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
            if(values[0]=="MIN_SCREEN_BRIGHTNES")
            {
                unsigned int tmp=values[1].toUInt(&check);
                if(check && tmp<=100)
                {
                    this->min_screen_brightness=tmp;
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
            if(values[0]=="SENSOR_REFRESH_FREQUENCY")
            {
                double tmp=values[1].replace(',','.').toDouble(&check);
                if(check && tmp>=0.1 && tmp<=5)
                {
                    this->sensor_frequency=1000.0/tmp;
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
            if(values[0]=="CHANGE_SCREEN_BRIGHTNESS_QUIET")
            {
                bool tmp=values[1].toInt(&check);
                if(!check)
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                    continue;
                }
                if(tmp)
                {
                    this->kde_service_function="setBrightnessSilent";
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    this->kde_service_function="setBrightness";
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                continue;
            }
            if(values[0]=="SCREEN_BRIGHTNESS_MANAGER_ON")
            {
                bool tmp=values[1].toInt(&check);
                if(!check)
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                    continue;
                }
                this->swch=tmp;
                read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                continue;
            }
            //Ambient
            if(values[0]=="AMBIENT_OR_LIGHT_SENSOR_SWITCH")
            {
                bool tmp=values[1].toInt(&check);
                if(!check)
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                    continue;
                }
                this->mbl_sensor_switch=tmp;
                read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                continue;
            }
            if(values[0]=="INITIALIZATION_DELAY")
            {
                double tmp=values[1].replace(',','.').toDouble(&check);
                if(check)
                {
                    this->initialization_delay=1000.0/tmp;
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
            if(values[0]=="INITIALIZATION_ATTEMPTS")
            {
                unsigned int tmp=values[1].toUInt(&check);
                if(check && tmp<=20)
                {
                    this->initialization_attempts_limit=tmp;
                    read_config_state+=values[0] + "=\"" + values[1] + "\" = " + QString::number(tmp) + "\n";
                }
                else
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
            if(values[0]=="AMBIENT_BRIGHTNES_PERCENTS")
            {
                QStringList tmp=values[1].split( ',', Qt::SkipEmptyParts );
                if(tmp.size()>0)
                {
                    for(int tmp_index=0; tmp_index < tmp.size() && tmp_index < _AMBL_BRIGHTNESS_COUNT; tmp_index++)
                    {
                        this->ambl_brightness_persents[tmp_index]=tmp.at(tmp_index).toInt(&check);
                        if(!check)
                        {
                            this->ambl_brightness_persents[tmp_index]=-1;
                            read_config_state+="Error in configuration file\n";
                            read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                            answer=2;
                        }
                        else
                        {
                            read_config_state+=values[0] + "value:" + QString::number(tmp_index+1) + " =\"";
                            read_config_state+=tmp.at(tmp_index) + "\" = " + QString::number(this->ambl_brightness_persents[tmp_index]) + "\n";
                        }
                    }
                }
                else
                {
                    read_config_state+="Error in configuration file\n";
                    read_config_state+="Error in line " + QString::number(line) + " missing or wrong value\n";
                    answer=2;
                }
                continue;
            }
    }

    //}
#ifdef _DIAGNOSTIC
    this->read_config_state=read_config_state;
#endif
    return read_config_state;
}

void Screen_light_manager::get_validate_config()
{
    int tmp=0;
    this->validate_config(tmp);
}


QString Screen_light_manager::validate_config(int& answer)
{
    QString validate_state="Start the function \"validate_config(int& answer)\"\n\n";;
    answer=0;
    if( this->config_max_lux > this->max_lux )
    {
        this->max_lux = this->config_max_lux;
        validate_state+="Readet from config MAX_SENSOR_BRIGHTNESS is greater than \n";
        validate_state+="the value in the program, seting program value too MAX_SENSOR_BRIGHTNESS\n";
    }
    if( this->config_max_lux < 1500 && this->max_lux < 1500 )
    {
        this->max_lux=1500;
        validate_state+="Readet from config MAX_SENSOR_BRIGHTNESS and the value available \n";
        validate_state+="in the program is less than the default maximum, the value will be set by default 1500\n";
    }
    if( this->min_screen_brightness > 100 )
    {
        this->min_screen_brightness=5;
        validate_state+="MIN_SCREEN_BRIGHTNES does not fit in the range from 0 to 100, \n";
        validate_state+="the value will be set by default 5\n";
    }
    if(this->sensor_frequency < 200 || this->sensor_frequency > 10000)
    {
        this->sensor_frequency=1000;
        validate_state+="SENSOR_REFRESH_FREQUENCY does not fit in the range from 0.1 to 5, \n";
        validate_state+="the value will be set by default 1\n";
    }
    if( this->kde_service_function != "setBrightnessSilent" && this->kde_service_function != "setBrightness" )
    {
#ifdef _DIAGNOSTIC
        this->kde_service_function = "setBrightness";
        validate_state+="CHANGE_SCREEN_BRIGHTNESS_QUIET is not set or is set to an invalid \n";
        validate_state+="value, the value will be set by default 0\n";
#else
        this->kde_service_function = "setBrightnessSilent";
        validate_state+="CHANGE_SCREEN_BRIGHTNESS_QUIET is not set or is set to an invalid \n";
        validate_state+="value, the value will be set by default 1\n";
#endif
    }
    for( int i = 0 ; i < _AMBL_BRIGHTNESS_COUNT ; i++ )
    {
        if( this->ambl_brightness_persents[i] < 0 || this->ambl_brightness_persents[i] > 100 )
            this->ambl_brightness_persents[i] = ( i + 1) * 20;
    }
    if(this->config_max_lux < this->max_lux)
    {
        validate_state+="The value available in the program turned out to be greater than \n";
        validate_state+="that read from MAX_SENSOR_BRIGHTNESS, the larger value is overwritten in the file.\n";
        answer=-1;
    }
    if( 100 > this->initialization_delay || 10000 < this->initialization_delay)
    {
        this->initialization_delay=5000;
        validate_state+="INITIALIZATION_DELAY is outside the allowed range of 0.1 to 10 seconds\n";
        validate_state+="Set INITIALIZATION_DELAY too default 5";
    }
    if( 0 == this->initialization_attempts_limit)
        this->initialization_attempts_limit=5;
    if( 0 == this->max_screen_brightness && this->initialization_attempts < this->initialization_attempts_limit )
    {
        this->initialization_attempts++;
        validate_state+="number initialization attempt:" + QString::number(this->initialization_attempts) + "\n";
        ///////////////////////////////////////////////////////////////////////////////////
        ///Получение максимальной яркости подсветки экрана от службы kde
        ///Управляющей яркостью экрана
        QDBusMessage msg = QDBusMessage::createMethodCall(
                    "org.freedesktop.PowerManagement",
                    "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                    "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                    "brightnessMax");
        msg=QDBusConnection::sessionBus().call(msg);
        if(!msg.errorMessage().isEmpty())
        {
            validate_state+="this->max_screen_brightness=" + QString::number(this->max_screen_brightness) +"\n";
            validate_state+="D-Bus: \n";
            validate_state+=msg.errorMessage() + "\n";
#ifdef _DIAGNOSTIC
            this->validate_state+=validate_state;
#endif
            QTimer::singleShot(this->initialization_delay, this, &Screen_light_manager::get_validate_config);
            return validate_state;
        }
        //Максимальная яркость подсветки экрана
        this->max_screen_brightness=msg.arguments()[0].toUInt();
        ///////////////////////////////////////////////////////////////////////////////
        ///Получение текущей яркости экрана от службы kde управляющей яркостью экрана
        msg = QDBusMessage::createMethodCall(
                        "org.freedesktop.PowerManagement",
                        "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                        "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                        "brightness");
        msg=QDBusConnection::sessionBus().call(msg);
        if(!msg.errorMessage().isEmpty())
        {
            validate_state+="this->prev_brightness=" + QString::number(this->prev_brightness) +"\n";
            validate_state+="D-Bus: \n";
            validate_state+=msg.errorMessage() + "\n";
#ifdef _DIAGNOSTIC
            this->validate_state+=validate_state;
#endif
            QTimer::singleShot(this->initialization_delay, this, &Screen_light_manager::get_validate_config);
            return validate_state;
        }
        //Предидущее значение яркости экрана для сравнения с поступающими показаниями сенсора
        this->prev_brightness=msg.arguments()[0].toDouble();
        //Один процент от максимальной яркости экрана
        this->light_screen_percent=(double)this->max_screen_brightness/100.0;
        if(this->swch)
        {
            this->sensors_switch();
            if(this->mbl_sensor_switch)
                this->ambl_change_brightness();
            else
                this->change_brightness();
        }
    }
#ifdef _DIAGNOSTIC
    this->validate_state+=validate_state;
#endif
    return validate_state;
}

void Screen_light_manager::save_to_config(int& answer)
{
#ifdef _DIAGNOSTIC
    this->save_too_config_state="Start the function \"save_to_config()\"\n\n";
#endif
    QFile config_file(QDir::homePath() + "/.config/kded_screen_brightness.config");
    bool config_file_exist=config_file.exists();
    answer=0;
    if(!config_file.open(QIODevice::ReadWrite))
    {
#ifdef _DIAGNOSTIC
        this->save_too_config_state+="error while opening file \"" +QDir::homePath() + "/.config/kded_screen_brightness.config\" too ReadWrite\n";
#endif
        answer=1;
    }
    QTextStream read(&config_file);
    unsigned int max_lux= this->max_lux > this->config_max_lux ? this->max_lux : this->config_max_lux;
    if(!config_file_exist)
    {
#ifdef _DIAGNOSTIC
        this->save_too_config_state+="The config file has just been created and will be writen with default values\n\n";
#endif
        read<<"#1 This configuration file has been generated and populated automatically with default values.\n";

        read<<"#2 Maximum sensor readings, will be corrected automatically\n";
        read<<"MAX_SENSOR_BRIGHTNESS:"<<max_lux<<" #3\n\n";

        read<<"#4 Minimal percent of screen brightness, default is 5\n";
        read<<"MIN_SCREEN_BRIGHTNES:"<<this->min_screen_brightness<<" #5\n\n";

        read<<"#6 Frequency of reading LightSensor in Hz min 0.1 HZ = 10sec/read max 5 HZ = 0.2sec/read\n";
        read<<"#7 Default is 1, important only when AMBIENT_OR_LIGHT_SENSOR_SWITCH:0\n";
        read<<"SENSOR_REFRESH_FREQUENCY:1 #8\n\n";

        read<<"#9 0 or any integer, if 0, then when the brightness changes, a plate will appear on the screen showing the current\n";
        read<<"CHANGE_SCREEN_BRIGHTNESS_QUIET:0 #10\n\n";

        read<<"#11 0 or any integer. Automatic brightness control at startup if not 0\n";
        read<<"SCREEN_BRIGHTNESS_MANAGER_ON:1 #12\n\n";

        read<<"#13 AMBIENT_OR_LIGHT_SENSOR_SWITCH sets the sensor type.\n";
        read<<"#14 If 0 is used QLightSensor, updates the readings more often and allows smoothly adjust the brightness of the screen.\n";
        read<<"#16 If not 0, QAmbientLightSensor is used,\n";
        read<<"#17 it shows only five levels of illumination because of this,\n";
        read<<"#18 it transmits readings less often, which allows using less system resources,\n";
        read<<"#19 but does not allow smooth adjustment of the screen brightness.\n";
        read<<"AMBIENT_OR_LIGHT_SENSOR_SWITCH:0 #20\n\n";

        read<<"#21 AMBIENT_BRIGHTNES_PERCENTS accepts five unsigned integers separated by commas.\n";
        read<<"#22 They set the brightness of the screen in percent,\n";
        read<<"#23 for the five levels of illumination that the QAmbientLightSensor shows,\n";
        read<<"#24 the first value is the lowest illumination and further upwards,\n";
        read<<"#25 the last fifth value is the maximum illumination.\n";
        read<<"AMBIENT_BRIGHTNES_PERCENTS:10,40,60,80,100 #26\n";
        read<<"#27 This service can start before the kde screen brightness control service,\n";
        read<<"#28 this parameter sets the possible waiting time for the kde service to start,\n";
        read<<"#29 necessary for it to be properly initialized and continue to work.\n";
        read<<"#30 Accepts a floating point value from 0.1 to 10 seconds to wait for a stert kde service.\n";
        read<<"INITIALIZATION_DELAY:5 #31\n";
        read<<"#32 The number of attempts to initialize this service before giving up,\n";
        read<<"#33 takes a positive integer between 1 and 20.\n";
        read<<"INITIALIZATION_ATTEMPTS:5 #34\n";
        config_file.close();
        this->config_max_lux = this->max_lux = max_lux;
        //save_too_config_state+="The configuration file was probably created by the program or was empty, the write was successful.\n";
#ifdef _DIAGNOSTIC
        //this->save_too_config_state=save_too_config_state;
#endif
        answer=-1;
    }
    QString config_content;
    qint64 start_pos=0;
    bool entery_flag[5] = { false, true, true, true, true };
    while(!read.atEnd())
    {
        QString coments(""), str_tmp, str;
        if(!entery_flag[0])
            start_pos=read.pos();
        str_tmp = str = read.readLine();
        int coment=str_tmp.indexOf('#');
        if(coment>=0)
        {
            if(coment==0)
            {
                if(entery_flag[0])
                    config_content+=str+"\n";
                continue;
            }
            coment--;
            int rm_lenght=str_tmp.length() - coment;
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="Removing comments from string:\n";
            this->save_too_config_state+= str_tmp + "\nFron ";
            this->save_too_config_state+= QString::number(coment) + "too ";
            this->save_too_config_state+= QString::number(rm_lenght) + " position\n";
#endif
            coments="   "+str_tmp.mid(coment);
            str_tmp.remove(coment,rm_lenght);
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="Save the comments \"" + coments + "\"\n";
#endif
        }
        if(!str.contains(':'))
        {
            if(entery_flag[0])
                config_content+=str+"\n";
            continue;
        }
        str_tmp=str_tmp.split(':')[0].trimmed();
        if(str_tmp == "MAX_SENSOR_BRIGHTNESS")
        {
            entery_flag[0]=true;
            entery_flag[1]=false;
            str="MAX_SENSOR_BRIGHTNESS:" + QString::number(max_lux) + coments;
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="MAX_SENSOR_BRIGHTNESS has been found, a new string has been created\n";
            this->save_too_config_state+= str + "\n";
#endif
            config_content += str + "\n";
            continue;

        }
        if( str_tmp == "SCREEN_BRIGHTNESS_MANAGER_ON" )
        {
            entery_flag[0]=true;
            entery_flag[2]=false;
            int tmp = this->swch ? 1 : 0;
            str="SCREEN_BRIGHTNESS_MANAGER_ON:" + QString::number(tmp) + coments;
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="SCREEN_BRIGHTNESS_MANAGER_ON has been found, a new string has been created\n";
            this->save_too_config_state+= str + "\n";
#endif
            config_content += str + "\n";
            continue;
        }
        if( str_tmp == "CHANGE_SCREEN_BRIGHTNESS_QUIET" )
        {
            entery_flag[0]=true;
            entery_flag[3]=false;
            int tmp = "setBrightnessSilent" == this->kde_service_function ? 1 : 0 ;
            str="CHANGE_SCREEN_BRIGHTNESS_QUIET:" + QString::number(tmp) + coments;
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="CHANGE_SCREEN_BRIGHTNESS_QUIET has been found, a new string has been created\n";
            this->save_too_config_state += str + "\n";
#endif
            config_content += str + "\n";
            continue;
        }
        if( str_tmp == "AMBIENT_OR_LIGHT_SENSOR_SWITCH" )
        {
            entery_flag[0]=true;
            entery_flag[4]=false;
            int tmp = this->mbl_sensor_switch ? 1 : 0 ;
            str= "AMBIENT_OR_LIGHT_SENSOR_SWITCH:" + QString::number(tmp) + coments;
#ifdef _DIAGNOSTIC
            this->save_too_config_state+="AMBIENT_OR_LIGHT_SENSOR_SWITCH has been found, a new string has been created\n";
            this->save_too_config_state+= str + "\n";
#endif
            config_content += str + "\n";
            continue;
        }
        if(entery_flag[0])
            config_content += str + "\n";
    }
    if(entery_flag[1])
    {
        config_content+="\n# Maximum sensor readings, will be corrected automatically\n";
        config_content+="MAX_SENSOR_BRIGHTNESS:" + QString::number(max_lux) + "\n";
    }
    if(entery_flag[2])
    {
        config_content+="\n# 0 or any integer. Automatic brightness control at startup if not 0\n";
        config_content+="SCREEN_BRIGHTNESS_MANAGER_ON:" + QString::number(this->swch ? 1 : 0) + " \n";
    }
    if(entery_flag[3])
    {
        config_content+="\n# 0 or any integer, if 0, then when the brightness changes, a plate will appear on the screen showing the current\n";
        config_content+="CHANGE_SCREEN_BRIGHTNESS_QUIET:" + QString::number("setBrightnessSilent" == this->kde_service_function ? 1 : 0) + "\n";
    }
    if(entery_flag[4])
    {
        config_content+="\n# AMBIENT_OR_LIGHT_SENSOR_SWITCH sets the sensor type.\n";
        config_content+="# If 0 is used QLightSensor, updates the readings more often and allows smoothly adjust the brightness of the screen.\n";
        config_content+="# If not 0, QAmbientLightSensor is used,\n";
        config_content+="# it shows only five levels of illumination because of this,\n";
        config_content+="# it transmits readings less often, which allows using less system resources,\n";
        config_content+="# but does not allow smooth adjustment of the screen brightness.\n";
        config_content+="AMBIENT_OR_LIGHT_SENSOR_SWITCH:" + QString::number(this->mbl_sensor_switch ? 1 : 0) + "\n";
    }
    read.seek(start_pos);
#ifdef _DIAGNOSTIC
    this->save_too_config_state+="Write position is:" + QString::number(start_pos) + "\n";
#endif
    read<<config_content;
    config_file.close();
    this->config_max_lux = this->max_lux = max_lux;
}
