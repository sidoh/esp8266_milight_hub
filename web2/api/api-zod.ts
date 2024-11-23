import { makeApi, Zodios, type ZodiosOptions } from "@zodios/core";
import { z } from "zod";

const RemoteType = z.enum([
  "rgbw",
  "cct",
  "rgb_cct",
  "rgb",
  "fut089",
  "fut091",
  "fut020",
]);
const Alias = z
  .object({
    alias: z.string(),
    id: z.number().int().optional(),
    device_id: z.number().int(),
    group_id: z.number().int(),
    device_type: RemoteType,
  })
  .passthrough();
const putAliasesId_Body = z
  .object({
    alias: z.string(),
    device_id: z.number().int(),
    group_id: z.number().int(),
  })
  .partial()
  .passthrough();
const BooleanResponse = z
  .object({
    success: z.boolean(),
    error: z
      .string()
      .describe("If an error occurred, message specifying what went wrong")
      .optional(),
  })
  .passthrough();
const State = z.enum(["ON", "OFF"]);
const ColorMode = z.enum(["brightness", "rgb", "color_temp", "onoff"]);
const NormalizedGroupState = z
  .object({
    alias: z.string(),
    state: State.describe("On/Off state"),
    color: z
      .object({ r: z.number().int(), g: z.number().int(), b: z.number().int() })
      .passthrough(),
    level: z.number().int().gte(0).lte(100),
    kelvin: z.number().int().gte(0).lte(100),
    color_mode:
      ColorMode.describe(`Describes the current color mode of the bulb.  Useful for HomeAssistant.
`),
  })
  .partial()
  .passthrough();
const GatewayListItem = z
  .object({
    state: NormalizedGroupState.describe(
      "Group state with a static set of fields"
    ),
    device: z
      .object({
        id: z.number(),
        device_id: z.number(),
        device_type: RemoteType,
        group_id: z.number(),
        alias: z.string(),
      })
      .passthrough(),
  })
  .passthrough();
const BulbId = z
  .object({
    device_id: z.number().int().gte(0).lte(65536),
    group_id: z.number().int().gte(0).lte(8),
    device_type: RemoteType,
  })
  .passthrough();
const GroupStateCommand = z.enum([
  "unpair",
  "pair",
  "set_white",
  "night_mode",
  "level_up",
  "level_down",
  "temperature_up",
  "temperature_down",
  "next_mode",
  "previous_mode",
  "mode_speed_down",
  "mode_speed_up",
  "toggle",
]);
const TransitionField = z.enum([
  "hue",
  "saturation",
  "brightness",
  "level",
  "kelvin",
  "color_temp",
  "color",
  "status",
]);
const TransitionValue = z.union([z.number(), z.string()]);
const TransitionArgs = z
  .object({
    field:
      TransitionField.describe(`If transitioning 'status': * If transitioning to 'OFF', will fade to 0 brightness and then turn off. * If transitioning to 'ON', will turn on, set brightness to 0, and fade to brightness 100.
`),
    start_value: TransitionValue.describe("Either an int value or a color"),
    end_value: TransitionValue.describe("Either an int value or a color"),
    duration: z
      .number()
      .describe("Duration of transition, measured in seconds"),
    period: z
      .number()
      .int()
      .describe(
        "Length of time between updates in a transition, measured in milliseconds"
      ),
  })
  .partial()
  .passthrough();
const GroupStateCommands = z
  .object({
    command: z.union([
      GroupStateCommand,
      z
        .object({ command: z.literal("transition"), args: TransitionArgs })
        .partial()
        .passthrough(),
    ]),
    commands: z.array(GroupStateCommand),
  })
  .partial()
  .passthrough();
const GroupState = z
  .object({
    state: State.describe("On/Off state"),
    status: State.describe("On/Off state"),
    hue: z
      .number()
      .int()
      .gte(0)
      .lte(359)
      .describe("Color hue.  Will change bulb to color mode."),
    saturation: z
      .number()
      .int()
      .gte(0)
      .lte(100)
      .describe("Color saturation.  Will normally change bulb to color mode."),
    kelvin: z
      .number()
      .int()
      .gte(0)
      .lte(100)
      .describe("White temperature.  0 is coolest, 100 is warmest."),
    temperature: z
      .number()
      .int()
      .gte(0)
      .lte(100)
      .describe("Alias for `kelvin`."),
    color_temp: z
      .number()
      .int()
      .gte(153)
      .lte(370)
      .describe(
        "White temperature measured in mireds.  Lower values are cooler."
      ),
    mode: z
      .number()
      .int()
      .describe("Party mode ID.  Actual effect depends on the bulb."),
    color: z.union([
      z.string(),
      z
        .object({
          r: z.number().int(),
          g: z.number().int(),
          b: z.number().int(),
        })
        .partial()
        .passthrough(),
    ]),
    level: z
      .number()
      .int()
      .gte(0)
      .lte(100)
      .describe("Brightness on a 0-100 scale."),
    brightness: z
      .number()
      .int()
      .gte(0)
      .lte(255)
      .describe("Brightness on a 0-255 scale."),
    effect: z.enum(["night_mode", "white_mode"]),
    transition: z.number()
      .describe(`Enables a transition from current state to the provided state.
`),
    color_mode:
      ColorMode.describe(`Describes the current color mode of the bulb.  Useful for HomeAssistant.
`),
  })
  .partial()
  .passthrough();
const UpdateBatch = z
  .object({
    gateways: z.array(BulbId),
    update: z.union([GroupStateCommands, GroupState]),
  })
  .partial()
  .passthrough();
const About = z
  .object({
    firmware: z.string().describe("Always set to 'milight-hub'"),
    version: z.string().describe("Semver version string"),
    ip_address: z.string(),
    reset_reason: z.string().describe("Reason the system was last rebooted"),
    variant: z.string().describe("Firmware variant (e.g., d1_mini, nodemcuv2)"),
    free_heap: z
      .number()
      .int()
      .describe("Amount of free heap remaining (measured in bytes)"),
    arduino_version: z
      .string()
      .describe("Version of Arduino SDK firmware was built with"),
    queue_stats: z
      .object({
        length: z
          .number()
          .int()
          .describe("Number of enqueued packets to be sent"),
        dropped_packets: z
          .number()
          .int()
          .describe(
            "Number of packets that have been dropped since last reboot"
          ),
      })
      .partial()
      .passthrough(),
    mqtt: z
      .object({
        configured: z.boolean(),
        connected: z.boolean(),
        status: z.string(),
      })
      .partial()
      .passthrough(),
  })
  .partial()
  .passthrough();
const BooleanResponseWithMessage = z
  .object({ success: z.boolean(), message: z.string() })
  .passthrough();
const postSystem_Body = z
  .object({ command: z.enum(["restart", "clear_wifi_config"]) })
  .passthrough();
const LedMode = z.enum([
  "Off",
  "Slow toggle",
  "Fast toggle",
  "Slow blip",
  "Fast blip",
  "Flicker",
  "On",
]);
const RF24Channel = z.enum(["LOW", "MID", "HIGH"]);
const GroupStateField = z.enum([
  "state",
  "status",
  "brightness",
  "level",
  "hue",
  "saturation",
  "color",
  "mode",
  "kelvin",
  "color_temp",
  "bulb_mode",
  "computed_color",
  "effect",
  "device_id",
  "group_id",
  "device_type",
  "oh_color",
  "hex_color",
  "color_mode",
]);
const Settings = z
  .object({
    admin_username: z
      .string()
      .describe(
        "If specified along with a password, HTTP basic auth will be enabled to access the web interface and the REST API."
      )
      .default(""),
    admin_password: z
      .string()
      .describe(
        "If specified along with a username, HTTP basic auth will be enabled to access the web interface and the REST API."
      )
      .default(""),
    ce_pin: z
      .number()
      .int()
      .describe("CE pin to use for SPI radio (nRF24, LT8900)")
      .default(4),
    csn_pin: z.number().int().describe("CSN pin to use with nRF24").default(15),
    reset_pin: z
      .number()
      .int()
      .describe("Reset pin to use with LT8900")
      .default(0),
    led_pin: z
      .number()
      .int()
      .describe(
        "Pin to control for status LED.  Set to a negative value to invert on/off status."
      )
      .default(-2),
    packet_repeats: z
      .number()
      .int()
      .describe(
        "Number of times to resend the same 2.4 GHz milight packet when a command is sent."
      )
      .default(50),
    http_repeat_factor: z
      .number()
      .int()
      .describe(
        "Packet repeats resulting from REST commands will be multiplied by this number."
      )
      .default(1),
    auto_restart_period: z
      .number()
      .int()
      .describe(
        "Automatically restart the device after the number of specified minutes.  Use 0 to disable."
      )
      .default(0),
    mqtt_server: z
      .union([z.string(), z.string()])
      .describe(
        "MQTT server to connect to. Can contain port number in the form 'mqtt-hostname:1883'. Leave empty to disable MQTT."
      )
      .nullable(),
    mqtt_username: z
      .string()
      .describe(
        "If specified, use this username to authenticate with the MQTT server."
      ),
    mqtt_password: z
      .string()
      .describe(
        "If specified, use this password to authenticate with the MQTT server."
      ),
    mqtt_topic_pattern: z
      .string()
      .describe(
        "Topic pattern to listen on for commands.  More detail on the format in README."
      ),
    mqtt_update_topic_pattern: z
      .string()
      .describe(
        "Topic pattern individual intercepted commands will be sent to.  More detail on the format in README."
      ),
    mqtt_state_topic_pattern: z
      .string()
      .describe(
        "Topic pattern device state will be sent to.  More detail on the format in README."
      ),
    mqtt_client_status_topic: z
      .string()
      .describe("Topic client status will be sent to."),
    mqtt_retain: z
      .boolean()
      .describe(
        "If true, messages sent to state and client status topics will be published with the retain flag."
      )
      .default(true),
    simple_mqtt_client_status: z
      .boolean()
      .describe(
        "If true, will use a simple enum flag (`connected` or `disconnected`) to indicate status.  If false, will send a rich JSON message including IP address, version, etc."
      )
      .default(true),
    radio_interface_type: z
      .enum(["nRF24", "LT8900"])
      .describe(
        "Type of radio interface to use. NRF24 is better supported and more common. Only use LT8900 if you're sure you mean to!"
      )
      .default("nRF24"),
    discovery_port: z
      .number()
      .int()
      .describe(
        "UDP port used for milight's discovery protocol. Set to 0 to disable."
      )
      .default(48899),
    listen_repeats: z
      .number()
      .int()
      .describe(
        "Controls how many cycles are spent listening for packets.  Set to 0 to disable passive listening."
      )
      .default(3),
    ignored_listen_protocols: z
      .array(z.enum(["RGBW", "CCT", "FUT089", "RGB", "FUT020"]))
      .describe(
        "Improve listen reliability by ignoring specific protocol types. Leave empty if you are unsure."
      )
      .default([]),
    state_flush_interval: z
      .number()
      .int()
      .describe(
        "Controls how many miliseconds must pass between states being flushed to persistent storage.  Set to 0 to disable throttling."
      )
      .default(10000),
    mqtt_state_rate_limit: z
      .number()
      .int()
      .describe(
        "Controls how many miliseconds must pass between MQTT state updates.  Set to 0 to disable throttling."
      )
      .default(500),
    mqtt_debounce_delay: z
      .number()
      .int()
      .describe(
        "Controls how much time has to pass after the last status update was queued."
      )
      .default(500),
    packet_repeat_throttle_threshold: z
      .number()
      .int()
      .describe(
        "Controls how packet repeats are throttled.  Packets sent with less time (measured in milliseconds) between them than this value (in milliseconds) will cause packet repeats to be throttled down.  More than this value will unthrottle up."
      )
      .default(200),
    packet_repeat_throttle_sensitivity: z
      .number()
      .int()
      .gte(0)
      .lte(1000)
      .describe(
        "Controls how packet repeats are throttled. Higher values cause packets to be throttled up and down faster.  Set to 0 to disable throttling."
      )
      .default(0),
    packet_repeat_minimum: z
      .number()
      .int()
      .describe(
        "Controls how far throttling can decrease the number of repeated packets"
      )
      .default(3),
    enable_automatic_mode_switching: z
      .boolean()
      .describe(
        "When making updates to hue or white temperature in a different bulb mode, switch back to the original bulb mode after applying the setting change."
      )
      .default(false),
    led_mode_wifi_config: LedMode,
    led_mode_wifi_failed: LedMode,
    led_mode_operating: LedMode,
    led_mode_packet: LedMode,
    led_mode_packet_count: z
      .number()
      .int()
      .describe("Number of times the LED will flash when packets are changing")
      .default(3),
    hostname: z
      .string()
      .regex(/[a-zA-Z0-9-]+/)
      .describe("Hostname that will be advertized on a DHCP request")
      .default("milight-hub"),
    rf24_power_level: z
      .enum(["MIN", "LOW", "HIGH", "MAX"])
      .describe(
        "Power level used when packets are sent.  See nRF24 documentation for further detail."
      )
      .default("MAX"),
    rf24_listen_channel: RF24Channel,
    wifi_static_ip: z
      .string()
      .describe("If specified, the static IP address to use"),
    wifi_static_ip_gateway: z
      .string()
      .describe(
        "If specified along with static IP, the gateway address to use"
      ),
    wifi_static_ip_netmask: z
      .string()
      .describe("If specified along with static IP, the netmask to use"),
    packet_repeats_per_loop: z
      .number()
      .int()
      .describe(
        "Packets are sent asynchronously.  This number controls the number of repeats sent during each iteration.  Increase this number to improve packet throughput.  Decrease to improve system multi-tasking."
      )
      .default(10),
    home_assistant_discovery_prefix: z
      .string()
      .describe(
        "If specified along with MQTT settings, will enable HomeAssistant MQTT discovery using the specified discovery prefix.  HomeAssistant's default is `homeassistant/`."
      )
      .default("homeassistant/"),
    wifi_mode: z
      .enum(["b", "g", "n"])
      .describe(
        "Forces WiFi into the spcified mode.  Try using B or G mode if you are having stability issues. Changing this may cause the device to momentarily lose connection to the network."
      )
      .default("n"),
    rf24_channels: z
      .array(RF24Channel)
      .describe(
        "Defines which channels we send on.  Each remote type has three channels.  We can send on any subset of these."
      ),
    gateway_configs: z
      .array(z.array(z.number().int()))
      .describe(
        "List of UDP servers, stored as 3-long arrays.  Elements are 1) remote ID to bind to, 2) UDP port to listen on, 3) protocol version (5 or 6)"
      ),
    group_state_fields: z.array(GroupStateField),
    group_id_aliases: z.object({}).partial().passthrough()
      .describe(`DEPRECATED (use /aliases routes instead)

Keys are aliases, values are 3-long arrays with same schema as items in 'device_ids'.
`),
    default_transition_period: z.number().int()
      .describe(`Default number of milliseconds between transition packets.  Set this value lower for more granular transitions, or higher if
you are having performance issues during transitions.
`),
  })
  .partial()
  .passthrough();
const ReadPacket = z
  .object({ packet_info: z.string() })
  .partial()
  .passthrough();
const device_id = z
  .union([z.number(), z.string()])
  .describe("2-byte device ID.  Can be decimal or hexadecimal.");
const putGatewaysDeviceIdRemoteTypeGroupId_Body =
  GroupState.and(GroupStateCommands);
const postRaw_commandsRemoteType_Body = z
  .object({
    packet: z
      .string()
      .regex(/([A-Fa-f0-9]{2}[ ])+/)
      .describe("Raw packet to send"),
    num_repeats: z
      .number()
      .int()
      .gte(1)
      .describe("Number of repeated packets to send"),
  })
  .partial()
  .passthrough();
const TransitionData = TransitionArgs.and(
  z
    .object({
      id: z.number().int(),
      last_sent: z
        .number()
        .int()
        .describe("Timestamp since last update was sent."),
      bulb: BulbId,
      type: z.enum(["field", "color"])
        .describe(`Specifies whether this is a simple field transition, or a color transition.
`),
      current_value: TransitionValue,
      end_value: TransitionValue,
    })
    .partial()
    .passthrough()
);
const postTransitions_Body = TransitionData.and(BulbId);
const PacketMessage = z
  .object({
    t: z.literal("packet").describe("Type of message").optional(),
    d: z
      .object({
        di: z.number().int().describe("Device ID"),
        gi: z.number().int().describe("Group ID"),
        rt: RemoteType.describe(
          "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
        ),
      })
      .passthrough()
      .describe("The bulb that the packet is for"),
    p: z.array(z.number().int()).describe("Raw packet data"),
    s: NormalizedGroupState.describe("Group state with a static set of fields"),
    u: z
      .object({})
      .partial()
      .passthrough()
      .describe("The command represented by the packet"),
  })
  .passthrough();
const WebSocketMessage = PacketMessage;
const DeviceId = z.array(z.unknown());

export const schemas = {
  RemoteType,
  Alias,
  putAliasesId_Body,
  BooleanResponse,
  State,
  ColorMode,
  NormalizedGroupState,
  GatewayListItem,
  BulbId,
  GroupStateCommand,
  TransitionField,
  TransitionValue,
  TransitionArgs,
  GroupStateCommands,
  GroupState,
  UpdateBatch,
  About,
  BooleanResponseWithMessage,
  postSystem_Body,
  LedMode,
  RF24Channel,
  GroupStateField,
  Settings,
  ReadPacket,
  device_id,
  putGatewaysDeviceIdRemoteTypeGroupId_Body,
  postRaw_commandsRemoteType_Body,
  TransitionData,
  postTransitions_Body,
  PacketMessage,
  WebSocketMessage,
  DeviceId,
};

const endpoints = makeApi([
  {
    method: "get",
    path: "/about",
    alias: "getAbout",
    requestFormat: "json",
    response: About,
  },
  {
    method: "post",
    path: "/aliases",
    alias: "postAliases",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: Alias,
      },
    ],
    response: z.object({ id: z.number().int() }).partial().passthrough(),
  },
  {
    method: "get",
    path: "/aliases",
    alias: "getAliases",
    requestFormat: "json",
    response: z
      .object({
        aliases: z.array(Alias),
        page: z.number().int(),
        count: z.number().int(),
        num_pages: z.number().int(),
      })
      .partial()
      .passthrough(),
  },
  {
    method: "get",
    path: "/aliases.bin",
    alias: "getAliases_bin",
    requestFormat: "json",
    response: z.void(),
  },
  {
    method: "post",
    path: "/aliases.bin",
    alias: "postAliases_bin",
    requestFormat: "form-data",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: z
          .object({ file: z.instanceof(File) })
          .partial()
          .passthrough(),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "put",
    path: "/aliases/:id",
    alias: "putAliasesId",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: putAliasesId_Body,
      },
      {
        name: "id",
        type: "Path",
        schema: z.number().int(),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "delete",
    path: "/aliases/:id",
    alias: "deleteAliasesId",
    requestFormat: "json",
    parameters: [
      {
        name: "id",
        type: "Path",
        schema: z.number().int(),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "post",
    path: "/backup",
    alias: "postBackup",
    requestFormat: "form-data",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: z
          .object({ file: z.instanceof(File) })
          .partial()
          .passthrough(),
      },
    ],
    response: BooleanResponseWithMessage,
    errors: [
      {
        status: 400,
        description: `error`,
        schema: BooleanResponseWithMessage,
      },
    ],
  },
  {
    method: "get",
    path: "/backup",
    alias: "getBackup",
    requestFormat: "json",
    response: z.void(),
  },
  {
    method: "post",
    path: "/firmware",
    alias: "postFirmware",
    requestFormat: "form-data",
    parameters: [
      {
        name: "body",
        description: `Firmware file`,
        type: "Body",
        schema: z
          .object({ fileName: z.instanceof(File) })
          .partial()
          .passthrough(),
      },
    ],
    response: z.void(),
    errors: [
      {
        status: 500,
        description: `server error`,
        schema: z.void(),
      },
    ],
  },
  {
    method: "get",
    path: "/gateway_traffic",
    alias: "getGateway_traffic",
    description: `Read a packet from any remote type.  Does not return a response until a packet is read.`,
    requestFormat: "json",
    response: z.object({ packet_info: z.string() }).partial().passthrough(),
  },
  {
    method: "get",
    path: "/gateway_traffic/:remoteType",
    alias: "getGateway_trafficRemoteType",
    description: `Read a packet from the given remote type.  Does not return a response until a packet is read. If &#x60;remote-type&#x60; is unspecified, will read from all remote types simultaneously.`,
    requestFormat: "json",
    parameters: [
      {
        name: "remoteType",
        type: "Path",
        schema: z
          .enum(["rgbw", "cct", "rgb_cct", "rgb", "fut089", "fut091", "fut020"])
          .describe(
            "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
          ),
      },
    ],
    response: z.object({ packet_info: z.string() }).partial().passthrough(),
  },
  {
    method: "get",
    path: "/gateways",
    alias: "getGateways",
    requestFormat: "json",
    response: z.array(GatewayListItem),
  },
  {
    method: "put",
    path: "/gateways",
    alias: "putGateways",
    description: `Update a batch of gateways with the provided parameters.`,
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: z.array(UpdateBatch),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "get",
    path: "/gateways/:deviceAlias",
    alias: "getGatewaysDeviceAlias",
    requestFormat: "json",
    parameters: [
      {
        name: "deviceAlias",
        type: "Path",
        schema: z.string().describe("Device alias saved in settings"),
      },
    ],
    response: GroupState,
    errors: [
      {
        status: 404,
        description: `provided device alias does not exist`,
        schema: z.void(),
      },
    ],
  },
  {
    method: "put",
    path: "/gateways/:deviceAlias",
    alias: "putGatewaysDeviceAlias",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: putGatewaysDeviceIdRemoteTypeGroupId_Body,
      },
      {
        name: "deviceAlias",
        type: "Path",
        schema: z.string().describe("Device alias saved in settings"),
      },
      {
        name: "blockOnQueue",
        type: "Query",
        schema: z
          .boolean()
          .describe(
            "If true, response will block on update packets being sent before returning"
          )
          .optional(),
      },
      {
        name: "fmt",
        type: "Query",
        schema: z
          .literal("normalized")
          .describe(
            "If set to `normalized`, the response will be in normalized format."
          )
          .optional(),
      },
    ],
    response: z.union([BooleanResponse, GroupState, NormalizedGroupState]),
    errors: [
      {
        status: 400,
        description: `error with request`,
        schema: BooleanResponse,
      },
    ],
  },
  {
    method: "delete",
    path: "/gateways/:deviceAlias",
    alias: "deleteGatewaysDeviceAlias",
    description: `Usets all known values for state fields for the corresponding device.  If MQTT is configured, the retained state message corresponding to this device will also be deleted.`,
    requestFormat: "json",
    parameters: [
      {
        name: "deviceAlias",
        type: "Path",
        schema: z.string().describe("Device alias saved in settings"),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "get",
    path: "/gateways/:deviceId/:remoteType/:groupId",
    alias: "getGatewaysDeviceIdRemoteTypeGroupId",
    description: `If &#x60;blockOnQueue&#x60; is provided, a response will not be returned until any unprocessed packets in the command queue are finished sending.`,
    requestFormat: "json",
    parameters: [
      {
        name: "deviceId",
        type: "Path",
        schema: device_id,
      },
      {
        name: "remoteType",
        type: "Path",
        schema: z
          .enum(["rgbw", "cct", "rgb_cct", "rgb", "fut089", "fut091", "fut020"])
          .describe(
            "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
          ),
      },
      {
        name: "groupId",
        type: "Path",
        schema: z
          .number()
          .int()
          .gte(0)
          .lte(8)
          .describe(
            "Group ID.  Should be 0-8, depending on remote type.  Group 0 is a 'wildcard' group.  All bulbs paired with the same device ID will respond to commands sent to Group 0."
          ),
      },
      {
        name: "blockOnQueue",
        type: "Query",
        schema: z
          .boolean()
          .describe(
            "If true, response will block on update packets being sent before returning"
          )
          .optional(),
      },
    ],
    response: GroupState,
  },
  {
    method: "put",
    path: "/gateways/:deviceId/:remoteType/:groupId",
    alias: "putGatewaysDeviceIdRemoteTypeGroupId",
    description: `Update state of the bulbs with the provided parameters.  Existing parameters will be unchanged.
if &#x60;blockOnQueue&#x60; is set to true, the response will not return until packets corresponding to the commands sent are processed, and the updated &#x60;GroupState&#x60; will be returned.  If &#x60;blockOnQueue&#x60; is false or not provided, a simple response indicating success will be returned.
if &#x60;fmt&#x60; is set to &#x60;normalized&#x60;, the response will be in normalized format.`,
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: putGatewaysDeviceIdRemoteTypeGroupId_Body,
      },
      {
        name: "deviceId",
        type: "Path",
        schema: device_id,
      },
      {
        name: "remoteType",
        type: "Path",
        schema: z
          .enum(["rgbw", "cct", "rgb_cct", "rgb", "fut089", "fut091", "fut020"])
          .describe(
            "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
          ),
      },
      {
        name: "groupId",
        type: "Path",
        schema: z
          .number()
          .int()
          .gte(0)
          .lte(8)
          .describe(
            "Group ID.  Should be 0-8, depending on remote type.  Group 0 is a 'wildcard' group.  All bulbs paired with the same device ID will respond to commands sent to Group 0."
          ),
      },
      {
        name: "blockOnQueue",
        type: "Query",
        schema: z
          .boolean()
          .describe(
            "If true, response will block on update packets being sent before returning"
          )
          .optional(),
      },
      {
        name: "fmt",
        type: "Query",
        schema: z
          .literal("normalized")
          .describe(
            "If set to `normalized`, the response will be in normalized format."
          )
          .optional(),
      },
    ],
    response: z.union([BooleanResponse, GroupState, NormalizedGroupState]),
    errors: [
      {
        status: 400,
        description: `error with request`,
        schema: BooleanResponse,
      },
    ],
  },
  {
    method: "delete",
    path: "/gateways/:deviceId/:remoteType/:groupId",
    alias: "deleteGatewaysDeviceIdRemoteTypeGroupId",
    description: `Usets all known values for state fields for the corresponding device.  If MQTT is configured, the retained state message corresponding to this device will also be deleted.`,
    requestFormat: "json",
    parameters: [
      {
        name: "deviceId",
        type: "Path",
        schema: device_id,
      },
      {
        name: "remoteType",
        type: "Path",
        schema: z
          .enum(["rgbw", "cct", "rgb_cct", "rgb", "fut089", "fut091", "fut020"])
          .describe(
            "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
          ),
      },
      {
        name: "groupId",
        type: "Path",
        schema: z
          .number()
          .int()
          .gte(0)
          .lte(8)
          .describe(
            "Group ID.  Should be 0-8, depending on remote type.  Group 0 is a 'wildcard' group.  All bulbs paired with the same device ID will respond to commands sent to Group 0."
          ),
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "post",
    path: "/raw_commands/:remoteType",
    alias: "postRaw_commandsRemoteType",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: postRaw_commandsRemoteType_Body,
      },
      {
        name: "remoteType",
        type: "Path",
        schema: z
          .enum(["rgbw", "cct", "rgb_cct", "rgb", "fut089", "fut091", "fut020"])
          .describe(
            "Type of remote to read a packet from.  If unspecified, will read packets from all remote types."
          ),
      },
    ],
    response: z.void(),
  },
  {
    method: "get",
    path: "/remote_configs",
    alias: "getRemote_configs",
    requestFormat: "json",
    response: z.void(),
  },
  {
    method: "get",
    path: "/settings",
    alias: "getSettings",
    requestFormat: "json",
    response: Settings,
  },
  {
    method: "put",
    path: "/settings",
    alias: "putSettings",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: Settings,
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "post",
    path: "/settings",
    alias: "postSettings",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: Settings,
      },
    ],
    response: BooleanResponse,
  },
  {
    method: "post",
    path: "/system",
    alias: "postSystem",
    description: `Send commands to the system.  Supported commands:
1. &#x60;restart&#x60;. Restart the ESP8266.
1. &#x60;clear_wifi_config&#x60;. Clears on-board wifi information. ESP8266 will reboot and enter wifi config mode.
`,
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: postSystem_Body,
      },
    ],
    response: BooleanResponse,
    errors: [
      {
        status: 400,
        description: `error`,
        schema: BooleanResponse,
      },
    ],
  },
  {
    method: "get",
    path: "/transitions",
    alias: "getTransitions",
    requestFormat: "json",
    response: z.array(TransitionData),
  },
  {
    method: "post",
    path: "/transitions",
    alias: "postTransitions",
    requestFormat: "json",
    parameters: [
      {
        name: "body",
        type: "Body",
        schema: postTransitions_Body,
      },
    ],
    response: BooleanResponse,
    errors: [
      {
        status: 400,
        description: `error`,
        schema: BooleanResponse,
      },
    ],
  },
  {
    method: "get",
    path: "/transitions/:id",
    alias: "getTransitionsId",
    requestFormat: "json",
    parameters: [
      {
        name: "id",
        type: "Path",
        schema: z
          .number()
          .int()
          .describe(
            "ID of transition.  This will be an auto-incrementing number reset after a restart."
          ),
      },
    ],
    response: TransitionData,
    errors: [
      {
        status: 404,
        description: `Provided transition ID not found`,
        schema: z.void(),
      },
    ],
  },
  {
    method: "delete",
    path: "/transitions/:id",
    alias: "deleteTransitionsId",
    requestFormat: "json",
    parameters: [
      {
        name: "id",
        type: "Path",
        schema: z
          .number()
          .int()
          .describe(
            "ID of transition.  This will be an auto-incrementing number reset after a restart."
          ),
      },
    ],
    response: BooleanResponse,
    errors: [
      {
        status: 404,
        description: `Provided transition ID not found`,
        schema: BooleanResponse,
      },
    ],
  },
]);

export const api = new Zodios(endpoints);

export function createApiClient(baseUrl: string, options?: ZodiosOptions) {
  return new Zodios(baseUrl, endpoints, options);
}
