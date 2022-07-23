# kded_brightness_master

KDED module for handling automatic screen brightness, with visual feedback when brightness change happens. Some assembly might be required.

# Installation

Run `./install_kded_brightness.sh` and install missing dependencies as needed. 

You'll most likely need `qt5-qtbase-devel`, `cmake-utils`, `extra-cmake-modules`, `iio-sensor-proxy`, `qt5-qtsensors` and `kf5-kded-devel`. Depending on your distribution, these packages might have different names. CMake will tell you which packages it is missing.

# Usage
On first start, the service will create a configuration file "User Home Folder/.config/kded_screen_brightness.config" with default settings, edit it if you want to change the settings.
"MAX_SENSOR_BRIGHTNESS" stores the maximum lightness value for the sensor on your device, the fact is that not everywhere you can programmatically find out this value, so the service constantly remembers the maximum value and if it is greater than that recorded in the configuration file, it overwrites it. The quickest way to set this value is simply to take your device out into the sun and after stopping the service or shutting down KDE or the entire device, the service will save the new value to a file.
“MIN_SCREEN_BRIGHTNES” Sets the minimum brightness of the screen when auto brightness changes, on some devices at brightness 0 nothing is visible on the screen.

Also, the service has an interface for communicating with it via Dbus, this can be done through the qdbus or qdbus-qt5 utility, possibly qdbus-qt6 if you have qt6 installed in my distribution, it is included in the libqt5-qdbus package. Type in the command line "qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.function argument".

qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 1 включает авто изменения яркости.

qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 0 отключает авто изменения яркости. 

qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 1 enables auto brightness changes.

qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 0 disables auto brightness changes.

Functions:

“set_change_brightness_silent argument” Argument 1 disables visual feedback, argument 0 enables it.

"get_change_brightness_silent" Returns 0 if visual feedback is enabled and 1 if it is disabled.

"set_brightness_manage_on argument" Argument 1 enables auto brightness management, argument 0 disables it.

"get_brightness_manage_on" Returns 1 if auto brightness control is enabled and 0 if it is disabled.

"set_ambl_or_light_sensor argument" Argument 1 to use QambientLightSensor it only reports five light states and saves resources, argument 0 to use QlightSensor it reports a digital light value and allows you to smoothly change the screen brightness but consumes more device resources.

"get_ambl_or_light_sensor" Returns 1 if QambientLightSensor is used and 0 if QlightSensor is used.

"reread_config" Causes the service to reread the configuration file and apply the new settings without restarting the service.

This interface is convenient to use together with the plasma widget "Configurable button".
To enable, set
qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 1; sleep2; exit 0
To turn off
qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.set_brightness_manage_on 0; sleep2; exit 0
To check the state when the widget starts
service=$(qdbus org.kde.screen.light.manager /org/kde/screen/light/manager org.kde.screen.light.manager.get_brightness_manage_on); if [[ $service -eq 1 ]] ; then exit 0; else exit 1; fi
Also in the folder there are icons that you can replace the standard icons for the on and off button, they are made by me completely and you can use them as you wish.
