# esp8266_milight_hub
This is a replacement for a Milight/LimitlessLED remote/gateway hosted on an ESP8266. Leverages [Henryk Plötz's awesome reverse-engineering work](https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol).

[Milight bulbs](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LPRQ4BK/r) are cheap smart bulbs that are controllable with an undocumented 2.4 GHz protocol. In order to control them, you either need a [remote](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LCSALV6/r?th=1) ($13), which allows you to control them directly, or a [WiFi gateway](https://www.amazon.com/BTF-LIGHTING-Mi-Light-WiFi-Bridge-Controller/dp/B01H87DYR8/ref=sr_1_7?ie=UTF8&qid=1485715984&sr=8-7&keywords=milight) ($30), which allows you to control them with a mobile app or a [UDP protocol](http://www.limitlessled.com/dev/).

[This guide](http://blog.christophermullins.com/2017/02/11/milight-wifi-gateway-emulator-on-an-esp8266/) on my blog details setting one of these up.

## Why this is useful

1. Both the remote and the WiFi gateway are limited to four groups. This means if you want to control more than four groups of bulbs, you need another remote or another gateway. This project allows you to control 262,144 groups (4*2^16, the limit imposed by the protocol).
2. This project exposes a nice REST API to control your bulbs.
3. You can secure the ESP8266 with a username/password, which is more than you can say for the Milight gateway! (The 2.4 GHz protocol is still totally insecure, so this doesn't accomplish much :).
4. Official hubs connect to remote servers to enable WAN access, and this behavior is not disableable.

## What you'll need

1. An ESP8266. I used a NodeMCU.
2. A NRF24L01+ module (~$3 on ebay).
3. Some way to connect the two (7 female/female dupont cables is probably easiest).

## Installing

#### Connect the NRF24L01+

This module is an SPI device. [This guide](https://www.mysensors.org/build/esp8266_gateway) details how to connect it. I used GPIO 16 for CE and GPIO 15 for CSN. These can be configured later.

#### Setting up the ESP

You'll need to flash the firmware and a SPIFFS image. It's really easy to do this with [PlatformIO](http://platformio.org/):

```
export ESP_BOARD=nodemcuv2
platformio run -u $ESP_BOARD --target upload
platformio run -u $ESP_BOARD --target uploadfs
```

Of course make sure to substitute `nodemcuv2` with the board that you're using.

#### Configure WiFi

This project uses [WiFiManager](https://github.com/tzapu/WiFiManager) to avoid the need to hardcode AP credentials in the firmware.

When the ESP powers on, you should be able to see a network named "ESPXXXXX", with XXXXX being an identifier for your ESP. Connect to this AP and a window should pop up prompting you to enter WiFi credentials.

#### Use it!

The HTTP endpoints (shown below) will be fully functional at this point. You should also be able to navigate to `http://<ip_of_esp>`. The UI should look like this:

![Web UI](http://imgur.com/XNNigvL.png)

## REST endpoints

1. `GET /`. Opens web UI. You'll need to upload it first.
1. `GET /settings`. Gets current settings as JSON.
1. `PUT /settings`. Patches settings (e.g., doesn't overwrite keys that aren't present). Accepts a JSON blob in the body.
1. `GET /radio_configs`. Get a list of supported radio configs (aka `device_type`s).
1. `GET /gateway_traffic/:device_type`. Starts an HTTP long poll. Returns any Milight traffic it hears. Useful if you need to know what your Milight gateway/remote ID is. Since protocols for RGBW/CCT are different, specify one of `rgbw`, `cct`, or `rgb_cct` as `:device_type. Accepts a JSON blob.
1. `PUT /gateways/:device_id/:device_type/:group_id`. Controls or sends commands to `:group_id` from `:device_id`. 
1. `PUT /gateways/:device_id/:device_type`. A few commands have support for being sent to all groups. You can send those here.
1. `POST /firmware`. OTA firmware update.
1. `POST /web`. Update web UI.

#### Bulb commands

Route (5) supports these commands:

1. `status`. Toggles on/off. Can be "on", "off", "true", or "false".
2. `hue`. (RGBW only) This is the only way to control color with these bulbs. Should be in the range `[0, 359]`.
3. `level`. (RGBW only) Controls brightness. Should be in the range `[0, 100]`.
4. `temperature`. (CCT only) Controls white temperature. Should be in the range `[0, 100]`.
5. `saturation`. (new RGB+CCT only) Controls saturation.
6. `command`. Sends a command to the group. Can be one of:
   * `set_white`. (RGBW only) Turns off RGB and enters WW/CW mode.
   * `pair`. Emulates the pairing process. Send this command right as you connect an unpaired bulb and it will pair with the device ID being used.
   * `unpair`. Emulates the unpairing process. Send as you connect a paired bulb to have it disassociate with the device ID being used.
   
Route (6) suports the `command`s `all_on` and `all_off`, which do as you'd expect.


#### Examples

Turn on group 2 for device ID 0xCD86, set hue to 100, and brightness to 50%:

```
$ curl --data-binary '{"status":"on","hue":100,"level":50}' -X PUT http://esp8266/gateways/0xCD86/rgbw/2
true%
```

Set color to white (disable RGB):

```
$ curl --data-binary '{"command":"set_white"}' -X PUT http://esp8266/gateways/0xCD86/rgbw/2
true%
```

## UDP Gateways

You can add an arbitrary number of UDP gateways through the REST API or through the web UI. Each gateway server listens on a port and responds to the standard set of commands supported by the Milight protocol. This should allow you to use one of these with standard Milight integrations (SmartThings, Home Assistant, OpenHAB, etc.).

You can select between versions 5 and 6 of the UDP protocol (documented [here](http://www.limitlessled.com/dev/)). Version 6 has support for the newer RGB+CCT bulbs and also includes response packets, which can theoretically improve reliability. Version 5 has much smaller packets and is probably lower latency.
