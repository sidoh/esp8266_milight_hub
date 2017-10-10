# esp8266_milight_hub
This is a replacement for a Milight/LimitlessLED remote/gateway hosted on an ESP8266. Leverages [Henryk Plötz's awesome reverse-engineering work](https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol).

[Milight bulbs](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LPRQ4BK/r) are cheap smart bulbs that are controllable with an undocumented 2.4 GHz protocol. In order to control them, you either need a [remote](https://www.amazon.com/Mi-light-Dimmable-RGBWW-Spotlight-Smart/dp/B01LCSALV6/r?th=1) ($13), which allows you to control them directly, or a [WiFi gateway](https://www.amazon.com/BTF-LIGHTING-Mi-Light-WiFi-Bridge-Controller/dp/B01H87DYR8/ref=sr_1_7?ie=UTF8&qid=1485715984&sr=8-7&keywords=milight) ($30), which allows you to control them with a mobile app or a [UDP protocol](http://www.limitlessled.com/dev/).

[This guide](http://blog.christophermullins.com/2017/02/11/milight-wifi-gateway-emulator-on-an-esp8266/) on my blog details setting one of these up.

## Why this is useful

1. Both the remote and the WiFi gateway are limited to four groups. This means if you want to control more than four groups of bulbs, you need another remote or another gateway. This project allows you to control 262,144 groups (4*2^16, the limit imposed by the protocol).
2. This project exposes a nice REST API to control your bulbs.
3. You can secure the ESP8266 with a username/password, which is more than you can say for the Milight gateway! (The 2.4 GHz protocol is still totally insecure, so this doesn't accomplish much :).
4. Official hubs connect to remote servers to enable WAN access, and this behavior is not disableable.
5. This project is capable of passively listening for Milight packets sent from other devices (like remotes). It can publish data from intercepted packets to MQTT. This could, for example, allow the use of Milight remotes while keeping your home automation platform's state in sync. See the MQTT section for more detail.

## Supported bulbs

Support has been added for the following [bulb types](http://futlight.com/productlist.aspx?typeid=101):

1. RGBW bulbs: FUT014, FUT016, FUT103
1. Dual-White (CCT) bulbs: FUT019
1. RGB LED strips: FUT025
1. RGB + Dual White (RGB+CCT) bulbs: FUT015

Other bulb types might work, but have not been tested. It is also relatively easy to add support for new bulb types.

## What you'll need

1. An ESP8266. I used a NodeMCU.
2. A NRF24L01+ module (~$3 on ebay). Alternatively, you can use a LT8900.
3. Some way to connect the two (7 female/female dupont cables is probably easiest).

## Installing

#### Connect the NRF24L01+ / LT8900

This project is compatible with both NRF24L01 and LT8900 radios. LT8900 is the same model used in the official MiLight devices. NRF24s are a very common 2.4 GHz radio device, but require software emulation of the LT8900's packet structure. As such, the LT8900 is more performant.

Both modules are SPI devices and should be connected to the standard SPI pins on the ESP8266.

##### NRF24L01+

[This guide](https://www.mysensors.org/build/esp8266_gateway) details how to connect an NRF24 to an ESP8266. I used GPIO 16 for CE and GPIO 15 for CSN. These can be configured later.

##### LT8900

Connect SPI pins (CS, SCK, MOSI, MISO) to appropriate SPI pins on the ESP8266. With default settings, connect RST to GPIO 0, and PKT to GPIO 16.

#### Setting up the ESP

The goal here is to flash your ESP with the firmware. It's really easy to do this with [PlatformIO](http://platformio.org/):

```
export ESP_BOARD=nodemcuv2
platformio run -e $ESP_BOARD --target upload
```

Of course make sure to substitute `nodemcuv2` with the board that you're using.

You can find pre-compiled firmware images on the [releases](https://github.com/sidoh/esp8266_milight_hub/releases).

#### Configure WiFi

This project uses [WiFiManager](https://github.com/tzapu/WiFiManager) to avoid the need to hardcode AP credentials in the firmware.

When the ESP powers on, you should be able to see a network named "ESPXXXXX", with XXXXX being an identifier for your ESP. Connect to this AP with the password "milightHub" and a window should pop up prompting you to enter WiFi credentials.


#### Get IP Address

Both mDNS and SSDP are supported.

* OS X - you should be able to navigate to http://milight-hub.local.
* Windows - you should see a device called "ESP8266 MiLight Gateway" show up in your network explorer.
* Linux users can install [avahi](http://www.avahi.org/) (`sudo apt-get install avahi-daemon` on Ubuntu), and should then be able to navigate to http://milight-hub.local.

#### Use it!

The HTTP endpoints (shown below) will be fully functional at this point. You should also be able to navigate to `http://<ip_of_esp>`, or `http://milight-hub.local` if your client supports mDNS. The UI should look like this:

![Web UI](http://imgur.com/XNNigvL.png)

## REST endpoints

1. `GET /`. Opens web UI. You'll need to upload it first.
1. `GET /about`. Return information about current firmware version.
1. `POST /system`. Post commands in the form `{"comamnd": <command>}`. Currently supports the commands: `restart`.
1. `POST /firmware`. OTA firmware update.
1. `GET /settings`. Gets current settings as JSON.
1. `PUT /settings`. Patches settings (e.g., doesn't overwrite keys that aren't present). Accepts a JSON blob in the body.
1. `GET /radio_configs`. Get a list of supported radio configs (aka `device_type`s).
1. `GET /gateway_traffic(/:device_type)?`. Starts an HTTP long poll. Returns any Milight traffic it hears. Useful if you need to know what your Milight gateway/remote ID is. Since protocols for RGBW/CCT are different, specify one of `rgbw`, `cct`, or `rgb_cct` as `:device_type.  The path `/gateway_traffic` without a `:device_type` will sniff for all protocols simultaneously.
1. `PUT /gateways/:device_id/:device_type/:group_id`. Controls or sends commands to `:group_id` from `:device_id`. Accepts a JSON blob. The schema is documented below in the _Bulb commands_ section.
1. `POST /raw_commands/:device_type`. Sends a raw RF packet with radio configs associated with `:device_type`. Example body:
    ```
    {"packet": "01 02 03 04 05 06 07 08 09", "num_repeats": 10}
    ```

#### Bulb commands

Route (5) supports these commands. Note that each bulb type has support for a different subset of these commands:

1. `status`. Toggles on/off. Can be "on", "off", "true", or "false".
1. `hue`. Sets color. Should be in the range `[0, 359]`.
1. `saturation`. Controls saturation.
1. `level`. Controls brightness. Should be in the range `[0, 100]`.
1. `temperature`. Controls white temperature. Should be in the range `[0, 100]`.
1. `mode`. Sets "disco mode" setting to the specified value. Note that not all bulbs that have modes support this command. Some will only allow you to cycle through next/previous modes using commands.
1. `command`. Sends a command to the group. Can be one of:
   * `set_white`. Turns off RGB and enters WW/CW mode.
   * `pair`. Emulates the pairing process. Send this command right as you connect an unpaired bulb and it will pair with the device ID being used.
   * `unpair`. Emulates the unpairing process. Send as you connect a paired bulb to have it disassociate with the device ID being used.
   * `next_mode`. Cycles to the next "disco mode".
   * `previous_mode`. Cycles to the previous disco mode.
   * `mode_speed_up`.
   * `mode_speed_down`.
   * `level_down`. Turns down the brightness. Not all dimmable bulbs support this command.
   * `level_up`. Turns down the brightness. Not all dimmable bulbs support this command.
   * `temperature_down`. Turns down the white temperature. Not all bulbs with adjustable white temperature support this command.
   * `temperature_up`. Turns up the white temperature. Not all bulbs with adjustable white temperature support this command.
   * `night_mode`. Enable "night mode", which is minimum brightness and bulbs only responding to on/off commands.
1. `commands`. An array containing any number of the above commands (including repeats).

The following redundant commands are supported for the sake of compatibility with HomeAssistant's [`mqtt_json`](https://home-assistant.io/components/light.mqtt_json/) light platform:

1. `color`. Hash containing RGB color. All keys for r, g, and b should be present. For example, `{"r":255,"g":200,"b":255}`.
1. `color_temp`. Controls white temperature. Value is in [mireds](https://en.wikipedia.org/wiki/Mired). Milight bulbs are in the range 153-370 mireds (2700K-6500K).
1. `brightness`. Same as `level` with a range of `[0,255]`.
1. `state`. Same as `status`.

If you'd like to control bulbs in all groups paired with a particular device ID, set `:group_id` to 0.

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

## MQTT

To configure your ESP to integrate with MQTT, fill out the following settings:

1. `mqtt_server`- IP or hostname should work. Specify a port with standard syntax (e.g., "mymqttbroker.com:1884").
1. `mqtt_topic_pattern` - you can control arbitrary configurations of device ID, device type, and group ID with this. A good default choice is something like `milight/:device_id/:device_type/:group_id`. More detail is provided below.
1. (optionally) `mqtt_username`
1. (optionally) `mqtt_password`

#### More detail on `mqtt_topic_pattern`

`mqtt_topic_pattern` leverages single-level wildcards (documented [here](https://mosquitto.org/man/mqtt-7.html)). For example, specifying `milight/:device_id/:device_type/:group_id` will cause the ESP to subscribe to the topic `milight/+/+/+`. It will then interpret the second, third, and fourth tokens in topics it receives messages on as `:device_id`, `:device_type`, and `:group_id`, respectively.

Messages should be JSON objects using exactly the same schema that the REST gateway uses for the `/gateways/:device_id/:device_type/:group_id` endpoint. Documented above in the _Bulb commands_ section.

##### Example:

If `mqtt_topic_pattern` is set to `milight/:device_id/:device_type/:group_id`, you could send the following message to it (the below example uses a ruby MQTT client):

```ruby
irb(main):001:0> require 'mqtt'
irb(main):002:0> client = MQTT::Client.new('10.133.8.11',1883)
irb(main):003:0> client.connect
irb(main):004:0> client.publish('milight/0x118D/rgb_cct/1', '{"status":"ON","color":{"r":255,"g":200,"b":255},"brightness":100}')
```

This will instruct the ESP to send messages to RGB+CCT bulbs with device ID `0x118D` in group 1 to turn on, set color to RGB(255,200,255), and brightness to 100.

#### Updates

To enable passive listening, make sure that `listen_repeats` is set to something larger than 0 (the default value of 3 is a good choice).

To publish data from intercepted packets to an MQTT topic, configure MQTT server settings, and set the `mqtt_update_topic_pattern` to something of your choice. As with `mqtt_topic_pattern`, the tokens `:device_id`, `:device_type`, and `:group_id` will be substituted with the values from the relevant packet.

The published message is a JSON blob containing the following keys:

* `device_id`
* `device_type` (rgb_cct, rgbw, etc.)
* `group_id`
* Any number of: `status`, `level`, `hue`, `saturation`, `kelvin`

As an example, if `mqtt_update_topic_pattern` is set to `milight/updates/:device_id/:device_type/:group_id`, and the group 1 on button of a Milight remote is pressed, the following update will be dispatched:

```ruby
irb(main):005:0> client.subscribe('milight/updates/+/+/+')
=> 27
irb(main):006:0> puts client.get.inspect
["lights/updates/0x1C8E/rgb_cct/1", "{\"device_id\":7310,\"group_id\":1,\"device_type\":\"rgb_cct\",\"status\":\"on\"}"]
```

## UDP Gateways

You can add an arbitrary number of UDP gateways through the REST API or through the web UI. Each gateway server listens on a port and responds to the standard set of commands supported by the Milight protocol. This should allow you to use one of these with standard Milight integrations (SmartThings, Home Assistant, OpenHAB, etc.).

You can select between versions 5 and 6 of the UDP protocol (documented [here](http://www.limitlessled.com/dev/)). Version 6 has support for the newer RGB+CCT bulbs and also includes response packets, which can theoretically improve reliability. Version 5 has much smaller packets and is probably lower latency.

## Acknowledgements

* @WoodsterDK added support for LT8900 radios.
