# esp8266_milight_hub
This is a replacement for a Milight/LimitlessLED remote/gateway hosted on an ESP8266. Leverages [Henryk Plötz's awesome reverse-engineering work](https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol).

[Milight bulbs](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LPRQ4BK/r) are cheap smart bulbs that are controllable with an undocumented 2.4 GHz protocol. In order to control them, you either need a [remote](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LCSALV6/r?th=1) (~$13), which allows you to control them directly, or a [WiFi gateway](https://www.amazon.com/BTF-LIGHTING-Mi-Light-WiFi-Bridge-Controller/dp/B01H87DYR8/ref=sr_1_7?ie=UTF8&qid=1485715984&sr=8-7&keywords=milight) (~$30), which allows you to control them with a mobile app or a [UDP protocol](http://www.limitlessled.com/dev/).

## What you'll need

1. An ESP8266. I used a NodeMCU.
2. A NRF24L01+ module (~$3 on ebay).

## Why this is useful

1. Both the remote and the WiFi gateway are limited to four groups. This means if you want to control more than four groups of bulbs, you need another remote or another gateway. This project allows you to control 262,144 groups (4*2^16, the limit imposed by the protocol).
2. This project exposes a nice REST API to control your bulbs.
3. You can secure the ESP8266 with a username/password, which is more than you can say for the Milight gateway! (The 2.4 GHz protocol is still totally insecure, so this doesn't accomplish much :).

## Installing

#### Connect the NRF24L01+

This module is an SPI device. [This guide] details how to connect it. I used GPIO 16 for CE and GPIO 15 for CSN. These can be configured later.

#### Setting up the ESP

1. Build from source. I use [PlatformIO](http://platformio.org/), but it's probably not hard to build this in the Arduino IDE.
2. Flash an ESP8266 with the firmware.
3. Connect to the "ESP XXXX" WiFi network to configure network settings. Alternatively you can update `main.cpp` to connect to your network directly.

#### Installing the Web UI

The HTTP endpoints will be fully functional at this point, but the firmware doesn't ship with a web UI (I didn't want to maintain a website in Arduino Strings).

If you want the UI, upload it to the `/web` endpoint. curl command:

```
$ curl -X POST -F 'image=@web/index.html' <ip of ESP>/web
success%
```

You should now be able to navigate to `http://<ip of ESP>`. It should look like this:


## REST endpoints

1. `GET /`. Opens web UI. You'll need to upload it first.
2. `GET /settings`. Gets current settings as JSON.
3. `PUT /settings`. Patches settings (e.g., doesn't overwrite keys that aren't present). Accepts a JSON blob in the body.
4. `GET /gateway_traffic`. Starts an HTTP long poll. Returns any Milight traffic it hears. Useful if you need to know what your Milight gateway/remote ID is.
5. `PUT /gateways/:device_id/:group_id`. Controls or sends commands to `:group_id` from `:device_id`. Accepts a JSON blob.
6. `PUT /gateways/:device_id`. A few commands have support for being sent to all groups. You can send those here.
7. `POST /firmware`. OTA firmware update.
8. `POST /web`. Update web UI.

#### Bulb commands

Route (5) supports these commands:

1. `status`. Toggles on/off. Can be "on", "off", "true", or "false".
2. `hue`. This is the only way to control color with these bulbs. Should be in the range `[0, 359]`.
3. `level`. Controls brightness. Should be in the range `[0, 100]`.
4. `command`. Sends a command to the group. Can be one of:
   * `set_white`. Turns off RGB and enters WW/CW mode.
   * `pair`. Emulates the pairing process. Send this command right as you connect an unpaired bulb and it will pair with the device ID being used.
   * `unpair`. Emulates the unpairing process. Send as you connect a paired bulb to have it disassociate with the device ID being used.
   
Route (6) suports the `command`s `all_on` and `all_off`, which do as you'd expect.


#### Examples

Turn on group 2 for device ID 0xCD86, set hue to 100, and brightness to 50%:

```
$ curl --data-binary '{"status":"on","hue":100,"level":50}' -X PUT http://esp8266/gateways/0xCD86/2
true%
```

Set color to white (disable RGB):

```
$ curl --data-binary '{"command":"set_white"}' -X PUT http://esp8266/gateways/0xCD86/2
true%
```
