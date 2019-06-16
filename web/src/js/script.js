$(function() {
  $(document).on('change', ':file', function() {
    var input = $(this),
        label = input.val().replace(/\\/g, '/').replace(/.*\//, '');
    input.trigger('fileselect', [label]);
  });

  $(document).ready( function() {
    $(':file').on('fileselect', function(event, label) {

        var input = $(this).parents('.input-group').find(':text'),
            log = label;

        if( input.length ) {
            input.val(log);
        }
    });
  });
});

var UNIT_PARAMS = {
  minMireds: 153,
  maxMireds: 370,
  maxBrightness: 255
};

var UI_TABS = [ {
    tag: "tab-wifi",
    friendly: "Wifi",
  }, {
    tag: "tab-setup",
    friendly: "Setup",
  }, {
    tag: "tab-led",
    friendly: "LED",
  }, {
    tag: "tab-radio",
    friendly: "Radio",
  }, {
    tag: "tab-mqtt",
    friendly: "MQTT"
  }
];

var UI_FIELDS = [ {
    tag: "admin_username",
    friendly: "Admin username",
    help: "Username for logging into this webpage",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "admin_password",
    friendly: "Password",
    help: "Password for logging into this webpage",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "hostname",
    friendly: "Hostname",
    help: "Self-reported hostname to send along with DCHP request",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "wifi_static_ip",
    friendly: "Static IP Address",
    help: "Static IP address (leave blank to use DHCP)",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "wifi_static_ip_netmask",
    friendly: "Static IP Netmask",
    help: "Netmask to use with Static IP",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "wifi_static_ip_gateway",
    friendly: "Static IP Gateway Address",
    help: "IP address to use as gateway when a Static IP is speicifed",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "ce_pin",
    friendly: "CE / PKT pin",
    help: "Pin on ESP8266 used for 'CE' (for NRF24L01 interface) or 'PKT' (for 'PL1167/LT8900' interface)",
    type: "string",
    tab: "tab-setup"
  }, {
    tag: "csn_pin",
    friendly: "CSN pin",
    help: "Pin on ESP8266 used for 'CSN'",
    type: "string",
    tab: "tab-setup"
  }, {
    tag: "reset_pin",
    friendly: "RESET pin",
    help: "Pin on ESP8266 used for 'RESET'",
    type: "string",
    tab: "tab-setup"
  }, {
    tag: "led_pin",
    friendly: "LED pin",
    help: "Pin to use for LED status display (0=disabled); negative inverses signal (recommend -2 for on-board LED)",
    type: "string",
    tab: "tab-setup"
  }, {
    tag: "packet_repeats",
    friendly: "Packet repeats",
    help: "The number of times to repeat RF packets sent to bulbs",
    type: "string",
    tab: "tab-radio"
  }, {
    tag: "packet_repeats_per_loop",
    friendly: "Packet repeats per loop",
    help: "Number of repeats to send in a single go.  Higher values mean more throughput, but less multitasking.",
    type: "string",
    tab: "tab-radio"
  }, {
    tag: "http_repeat_factor",
    friendly: "HTTP repeat factor",
    help: "Multiplicative factor on packet_repeats for requests initiated by the HTTP API. UDP API typically receives " +
    "duplicate packets, so more repeats should be used for HTTP",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "auto_restart_period",
    friendly: "Auto-restart period",
    help: "Automatically restart the device every number of minutes specified. Use 0 for disabled",
    type: "string",
    tab: "tab-setup"
  }, {
    tag: "discovery_port",
    friendly: "Discovery port",
    help: "UDP port to listen for discovery packets on. Defaults to the same port used by MiLight devices, 48899. Use 0 to disable",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag: "mqtt_server",
    friendly: "MQTT server",
    help: "Domain or IP address of MQTT broker. Optionally specify a port with (example) myMQTTbroker.com:1884",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag: "mqtt_topic_pattern",
    friendly: "MQTT topic pattern",
    help: "Pattern for MQTT topics to listen on. Example: lights/:device_id/:device_type/:group_id. See README for further details",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "mqtt_update_topic_pattern",
    friendly: "MQTT update topic pattern",
    help: "Pattern to publish MQTT updates. Packets that are received from other devices, and packets that are sent from this device will " +
    "result in updates being sent",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "mqtt_state_topic_pattern",
    friendly: "MQTT state topic pattern",
    help: "Pattern for MQTT topic to publish state to. When a group changes state, the full known state of the group will be published to this topic pattern",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "mqtt_username",
    friendly: "MQTT user name",
    help: "User name to log in to MQTT server",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "mqtt_password",
    friendly: "MQTT password",
    help: "Password to log into MQTT server",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "mqtt_client_status_topic",
    friendly: "MQTT Client Status Topic",
    help: "Connection status messages will be published to this topic.  This includes LWT and birth.  See README for further detail.",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "simple_mqtt_client_status",
    friendly: "Client Status Messages Mode",
    help: "In simple mode, only the strings 'connected' and 'disconnected' will be published.  In detailed mode, data about the version, IP address, etc. will be included.",
    type: "option_buttons",
    options: {
      true: "Simple",
      false: "Detailed"
    },
    tab: "tab-mqtt"
  }, {
    tag:   "radio_interface_type",
    friendly: "Radio interface type",
    help: "2.4 GHz radio model. Only change this if you know you're not using an NRF24L01!",
    type: "option_buttons",
    options: {
      'nRF24': 'nRF24',
      'LT8900': 'PL1167/LT8900'
    },
    tab: "tab-radio"
  }, {
    tag:   "rf24_power_level",
    friendly: "nRF24 Power Level",
    help: "Power level for nRF24L01",
    type: "option_buttons",
    options: {
      'MIN': 'Min',
      'LOW': 'Low',
      'HIGH': 'High',
      'MAX': 'Max'
    },
    tab: "tab-radio"
  }, {
    tag:   "rf24_listen_channel",
    friendly: "nRF24 Listen Channel",
    help: "Which channels to listen for messages on the nRF24",
    type: "option_buttons",
    options: {
      'LOW': 'Min',
      'MID': 'Mid',
      'HIGH': 'High'
    },
    tab: "tab-radio"
  }, {
    tag:   "rf24_channels",
    friendly: "nRF24 Send Channels",
    help: "Which channels to send messages on for the nRF24.  Using fewer channels speeds up sends.",
    type: "option_buttons",
    settings: {
      multiple: true,
    },
    options: {
      'LOW': 'Min',
      'MID': 'Mid',
      'HIGH': 'High'
    },
    tab: "tab-radio"
  }, {
    tag:   "listen_repeats",
    friendly: "Listen repeats",
    help: "Increasing this increases the amount of time spent listening for " +
    "packets. Set to 0 to disable listening. Default is 3.",
    type: "string",
    tab: "tab-wifi"
  }, {
    tag:   "state_flush_interval",
    friendly: "State flush interval",
    help: "Minimum number of milliseconds between flushing state to flash. " +
    "Set to 0 to disable delay and immediately persist state to flash",
    type: "string",
    tab: "tab-setup"
  }, {
    tag:   "mqtt_state_rate_limit",
    friendly: "MQTT state rate limit",
    help: "Minimum number of milliseconds between MQTT updates of bulb state (defaults to 500)",
    type: "string",
    tab: "tab-mqtt"
  }, {
    tag:   "packet_repeat_throttle_threshold",
    friendly: "Packet repeat throttle threshold",
    help: "Controls how packet repeats are throttled.  Packets sent " +
    "with less time between them than this value (in milliseconds) will cause " +
    "packet repeats to be throttled down.  More than this value will unthrottle " +
    "up.  Defaults to 200ms",
    type: "string",
    tab: "tab-radio"
  }, {
    tag:   "packet_repeat_throttle_sensitivity",
    friendly: "Packet repeat throttle sensitivity",
    help: "Controls how packet repeats are throttled. " +
    "Higher values cause packets to be throttled up and down faster " +
    "(defaults to 0, maximum value 1000, 0 disables)",
    type: "string",
    tab: "tab-radio"
  }, {
    tag:   "packet_repeat_minimum",
    friendly: "Packet repeat minimum",
    help: "Controls how far throttling can decrease the number " +
    "of repeated packets (defaults to 3)",
    type: "string",
    tab: "tab-radio"
  }, {
    tag:   "group_state_fields",
    friendly: "Group state fields",
    help: "Selects which fields should be included in MQTT state updates and REST responses for bulb state",
    type: "group_state_fields",
    tab: "tab-mqtt"
  }, {
    tag:   "enable_automatic_mode_switching",
    friendly: "Enable automatic mode switching",
    help: "For RGBWW bulbs (using RGB+CCT or FUT089), enables automatic switching between modes in order to affect changes to " +
    "temperature and saturation when otherwise it would not work",
    type: "option_buttons",
    options: {
      true: 'Enable',
      false: 'Disable'
    },
    tab: "tab-radio"
  }, {
    tag:   "led_mode_wifi_config",
    friendly: "LED mode during wifi config",
    help: "LED mode when the device is in Access Point mode waiting to configure Wifi",
    type: "led_mode",
    tab: "tab-led"
  }, {
    tag:   "led_mode_wifi_failed",
    friendly: "LED mode when wifi failed to connect",
    help: "LED mode when the device has failed to connect to the wifi network",
    type: "led_mode",
    tab: "tab-led"
  }, {
    tag:   "led_mode_operating",
    friendly: "LED mode when operating",
    help: "LED mode when the device is in successfully running",
    type: "led_mode",
    tab: "tab-led"
  }, {
    tag:   "led_mode_packet",
    friendly: "LED mode on packets",
    help: "LED mode when the device is sending or receiving packets",
    type: "led_mode",
    tab: "tab-led"
  }, {
    tag:   "led_mode_packet_count",
    friendly: "Flash count on packets",
    help: "Number of times the LED will flash when packets are changing",
    type: "string",
    tab: "tab-led"
  }
];

// TODO: sync this with GroupStateField.h
var GROUP_STATE_KEYS = [
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
  "oh_color"
];

var LED_MODES = [
  "Off",
  "Slow toggle",
  "Fast toggle",
  "Slow blip",
  "Fast blip",
  "Flicker",
  "On"
];

var UDP_PROTOCOL_VERSIONS = [ 5, 6 ];
var DEFAULT_UDP_PROTOCL_VERSION = 5;

var selectize;
var sniffing = false;
var loadingSettings = false;

// don't attempt websocket if we are debugging locally
if (location.hostname != "") {
  var webSocket = new WebSocket("ws://" + location.hostname + ":81");
  webSocket.onmessage = function(e) {
    if (sniffing) {
      var message = e.data;
      $('#sniffed-traffic').prepend('<pre>' + message + '</pre>');
    }
  }
}

var toHex = function(v) {
  return "0x" + (v).toString(16).toUpperCase();
}

var activeUrl = function() {
  var deviceId = $('#deviceId option:selected').val()
    , groupId = $('#groupId .active input').data('value')
    , mode = getCurrentMode();

  if (deviceId == "") {
    throw "Must enter device ID";
  }

  if (! $('#group-option').data('for').split(',').includes(mode)) {
    groupId = 0;
  }

  if (typeof groupId === "undefined") {
    throw "Must enter group ID";
  }

  return "/gateways/" + deviceId + "/" + mode + "/" + groupId;
}

var getCurrentMode = function() {
  return $('#mode li.active').data('value');
};

var updateGroup = _.throttle(
  function(params) {
    try {
      $.ajax({
        url: activeUrl(),
        method: 'PUT',
        data: JSON.stringify(params),
        contentType: 'application/json',
        success: function(e) {
          handleStateUpdate(e);
        }
      });
    } catch (e) {
      alert(e);
    }
  },
  1000
);

var sendCommand = _.throttle(
  function(params) {
    $.ajax(
      '/system',
      {
        method: 'POST',
        data: JSON.stringify(params),
        contentType: 'application/json'
      }
    );
  },
  1000
)

var gatewayServerRow = function(deviceId, port, version) {
  var elmt = '<tr>';
  elmt += '<td>';
  elmt += '<input name="deviceIds[]" class="form-control" value="' + deviceId + '"/>';
  elmt += '</td>';
  elmt += '<td>'
  elmt += '<input name="ports[]" class="form-control" value="' + port + '"/>';;
  elmt += '</td>';
  elmt += '<td>';
  elmt += '<div class="btn-group" data-toggle="buttons">';

  for (var i = 0; i < UDP_PROTOCOL_VERSIONS.length; i++) {
    var val = UDP_PROTOCOL_VERSIONS[i]
      , selected = (version == val || (val == DEFAULT_UDP_PROTOCL_VERSION && !UDP_PROTOCOL_VERSIONS.includes(version)));

    elmt += '<label class="btn btn-secondary' + (selected ? ' active' : '') + '">';
    elmt += '<input type="radio" name="versions[]" autocomplete="off" data-value="' + val + '" '
      + (selected ? 'checked' : '') +'> ' + val;
    elmt += '</label>';
  }

  elmt += '</div></td>';
  elmt += '<td>';
  elmt += '<button class="btn btn-danger remove-gateway-server">';
  elmt += '<i class="glyphicon glyphicon-remove"></i>';
  elmt += '</button>';
  elmt += '</td>';
  elmt += '</tr>';
  return elmt;
}

var loadSettings = function() {
  $('select.select-init').selectpicker();
  if (location.hostname == "") {
    // if deugging locally, don't try get settings
    return;
  }
  $.getJSON('/settings', function(val) {
    loadingSettings = true;

    Object.keys(val).forEach(function(k) {
      var field = $('#settings input[name="' + k + '"]');
      var selectVal = function(selectVal) {
        field.filter('[value="' + selectVal + '"]').click();
      };

      if (field.length > 0) {
        if (field.attr('type') === 'radio' || field.attr('type') === 'checkbox') {
          if (Array.isArray(val[k])) {
            val[k].forEach(function(x) {
              selectVal(x);
            });
          } else {
            selectVal(val[k]);
          }
        } else {
          field.val(val[k]);
        }
      }
    });

    if (val.device_ids) {
      selectize.clearOptions();
      val.device_ids.forEach(function(v) {
        selectize.addOption({text: toHex(v), value: v});
      });
      selectize.refreshOptions();
    }

    if (val.group_state_fields) {
      var elmt = $('select[name="group_state_fields"]');
      elmt.selectpicker('val', val.group_state_fields);
    }

    if (val.led_mode_wifi_config) {
      var elmt = $('select[name="led_mode_wifi_config"]');
      elmt.selectpicker('val', val.led_mode_wifi_config);
    }

    if (val.led_mode_wifi_failed) {
      var elmt = $('select[name="led_mode_wifi_failed"]');
      elmt.selectpicker('val', val.led_mode_wifi_failed);
    }

    if (val.led_mode_operating) {
      var elmt = $('select[name="led_mode_operating"]');
      elmt.selectpicker('val', val.led_mode_operating);
    }

    if (val.led_mode_packet) {
      var elmt = $('select[name="led_mode_packet"]');
      elmt.selectpicker('val', val.led_mode_packet);
    }

    var gatewayForm = $('#gateway-server-configs').html('');
    if (val.gateway_configs) {
      val.gateway_configs.forEach(function(v) {
        gatewayForm.append(gatewayServerRow(toHex(v[0]), v[1], v[2]));
      });
    }

    loadingSettings = false;
  });
};

var saveGatewayConfigs = function() {
  var form = $('#tab-udp-gateways')
    , errors = false;

  $('input', form).removeClass('error');

  var deviceIds = $('input[name="deviceIds[]"]', form).map(function(i, v) {
    var val = $(v).val();

    if (isNaN(val)) {
      errors = true;
      $(v).addClass('error');
      return null;
    } else {
      return val;
    }
  });

  var ports = $('input[name="ports[]"]', form).map(function(i, v) {
    var val = $(v).val();

    if (isNaN(val)) {
      errors = true;
      $(v).addClass('error');
      return null;
    } else {
      return val;
    }
  });

  var versions = $('.active input[name="versions[]"]', form).map(function(i, v) {
    return $(v).data('value');
  });

  if (!errors) {
    var data = [];
    for (var i = 0; i < deviceIds.length; i++) {
      data[i] = [deviceIds[i], ports[i], versions[i]];
    }
    $.ajax(
      '/settings',
      {
        method: 'put',
        contentType: 'application/json',
        data: JSON.stringify({gateway_configs: data})
      }
    )
  }
};

var saveDeviceIds = function() {
  if (!loadingSettings) {
    var deviceIds = _.map(
      $('#deviceId')[0].selectize.options,
      function(option) {
        return option.value;
      }
    );

    $.ajax(
      "/settings",
      {
        method: 'put',
        contentType: 'application/json',
        data: JSON.stringify({device_ids: deviceIds})
      }
    );
  }
};

var deleteDeviceId = function() {
  selectize.removeOption($(this).data('value'));
  selectize.refreshOptions();
  saveDeviceIds();
};

var deviceIdError = function(v) {
  if (!v) {
    $('#device-id-label').removeClass('error');
  } else {
    $('#device-id-label').addClass('error');
    $('#device-id-label .error-info').html(v);
  }
};

var updateModeOptions = function() {
  var currentMode = getCurrentMode()
    , modeLabel = $('#mode li[data-value="' + currentMode + '"] a').html();

  $('label', $('#mode').closest('.dropdown')).html(modeLabel);

  $('.mode-option').map(function() {
    if ($(this).data('for').split(',').includes(currentMode)) {
      $(this).show();
    } else {
      $(this)
        // De-select unselectable group
        .removeClass('active')
        .hide();
    }
  });
};

var parseVersion = function(v) {
  var matches = v.match(/(\d+)\.(\d+)\.(\d+)(-(.*))?/);

  return {
    major: matches[1],
    minor: matches[2],
    patch: matches[3],
    revision: matches[5],
    parts: [matches[1], matches[2], matches[3], matches[5]]
  };
};

var isNewerVersion = function(a, b) {
  var va = parseVersion(a)
    , vb = parseVersion(b);

  return va.parts > vb.parts;
};

var handleCheckForUpdates = function() {
  var currentVersion = null
    , latestRelease = null;

  var handleReceiveData = function() {
    if (currentVersion != null) {
      $('#current-version').html(currentVersion.version + " (" + currentVersion.variant + ")");
    }

    if (latestRelease != null) {
      $('#latest-version .info-key').each(function() {
        var value = latestRelease[$(this).data('key')];
        var prop = $(this).data('prop');

        if (prop) {
          $(this).prop(prop, value);
        } else {
          $(this).html(value);
        }
      });
    }

    if (currentVersion != null && latestRelease != null) {
      $('#latest-version .status').html('');
      $('#latest-version-info').show();

      var summary;
      if (isNewerVersion(latestRelease.tag_name, currentVersion.version)) {
        summary = "New version available!";
      } else {
        summary = "You're on the most recent version.";
      }
      $('#version-summary').html(summary);

      var releaseAsset = latestRelease.assets.filter(function(x) {
        return x.name.indexOf(currentVersion.variant) != -1;
      });

      if (releaseAsset.length > 0) {
        $('#firmware-link').prop('href', releaseAsset[0].browser_download_url);
      }
    }
  }

  var handleError = function(e, d) {
    console.log(e);
    console.log(d);
  }

  $('#current-version,#latest-version .status').html('<i class="spinning glyphicon glyphicon-refresh"></i>');
  $('#version-summary').html('');
  $('#latest-version-info').hide();

  $.ajax(
    '/about',
    {
      success: function(data) {
        currentVersion = data;
        handleReceiveData();
      },
      failure: handleError
    }
  );

  $.ajax(
    'https://api.github.com/repos/sidoh/esp8266_milight_hub/releases/latest',
    {
      success: function(data) {
        latestRelease = data;
        handleReceiveData();
      },
      failure: handleError
    }
  );
};

var handleStateUpdate = function(state) {
  if (state.state) {
    // Set without firing an event
    $('input[name="status"]')
      .prop('checked', state.state == 'ON')
      .bootstrapToggle('destroy')
      .bootstrapToggle();
  }
  if (state.color) {
    // Browsers don't support HSV, but saturation from HSL doesn't match
    // saturation from bulb state.
    var hsl = rgbToHsl(state.color.r, state.color.g, state.color.b);
    var hsv = RGBtoHSV(state.color.r, state.color.g, state.color.b);

    $('input[name="saturation"]').slider('setValue', hsv.s*100);
    updatePreviewColor(hsl.h*360,hsl.s*100,hsl.l*100);
  }
  if (state.color_temp) {
    var scaledTemp
      = 100*(state.color_temp - UNIT_PARAMS.minMireds) / (UNIT_PARAMS.maxMireds - UNIT_PARAMS.minMireds);
    $('input[name="temperature"]').slider('setValue', scaledTemp);
  }
  if (state.brightness) {
    var scaledBrightness = state.brightness * (100 / UNIT_PARAMS.maxBrightness);
    $('input[name="level"]').slider('setValue', scaledBrightness);
  }
};

var updatePreviewColor = function(hue, saturation, lightness) {
  if (! saturation) {
    saturation = 100;
  }
  if (! lightness) {
    lightness = 50;
  }
  $('.hue-value-display').css({
    backgroundColor: "hsl(" + hue + "," + saturation + "%," + lightness + "%)"
  });
};

var stopSniffing = function() {
  var elmt = $('#sniff');

  sniffing = false;
  $('i', elmt)
    .removeClass('glyphicon-stop')
    .addClass('glyphicon-play');
  $('span', elmt).html('Start Sniffing');
};

var startSniffing = function() {
  var elmt = $('#sniff');

  sniffing = true;
  $('i', elmt)
    .removeClass('glyphicon-play')
    .addClass('glyphicon-stop');
  $('span', elmt).html('Stop Sniffing');
  $("#traffic-sniff").show();
};

var generateDropdownField = function(fieldName, options, settings) {
  var s = '<div class="btn-group" id="' + fieldName + '" data-toggle="buttons">';
  var inputType = settings.multiple ? 'checkbox' : 'radio';

  Object.keys(options).forEach(function(optionValue) {
    var optionLabel = options[optionValue];
    s += '<label class="btn btn-secondary">' +
           '<input type="' + inputType + '" id="' + fieldName + '" name="' + fieldName + '" autocomplete="off" value="' + optionValue + '" /> ' + optionLabel +
         '</label>';
  });

  s += '</div>';

  return s;
};

$(function() {
  $('.radio-option').click(function() {
    $(this).prev().prop('checked', true);
  });

  var hueDragging = false;
  var colorUpdated = function(e) {
    var x = e.pageX - $(this).offset().left
      , pct = x/(1.0*$(this).width())
      , hue = Math.round(360*pct)
      ;

    updatePreviewColor(hue);

    updateGroup({hue: hue});
  };

  $('.hue-picker-inner')
    .mousedown(function(e) {
      hueDragging = true;
      colorUpdated.call(this, e);
    })
    .mouseup(function(e) {
      hueDragging = false;
    })
    .mouseout(function(e) {
      hueDragging = false;
    })
    .mousemove(function(e) {
      if (hueDragging) {
        colorUpdated.call(this, e);
      }
    });

  $('.slider').slider();

  $('.raw-update').change(function() {
    var data = {}
      , val = $(this).attr('type') == 'checkbox' ? ($(this).is(':checked') ? 'on' : 'off') : $(this).val()
      ;

    data[$(this).attr('name')] = val;
    updateGroup(data);
  });

  $('.command-btn').click(function() {
    updateGroup({command: $(this).data('command')});
  });

  $('.system-btn').click(function() {
    sendCommand({command: $(this).data('command')});
  });

  $('#sniff').click(function(e) {
    e.preventDefault();

    if (sniffing) {
      stopSniffing();
    } else {
      startSniffing();
    }
  });

  $('#traffic-sniff-close').click(function() {
    stopSniffing();
    $('#traffic-sniff').hide();
  });

  $('body').on('click', '#add-server-btn', function(e) {
    e.preventDefault();
    $('#gateway-server-configs').append(gatewayServerRow('', ''));
  });

  $('#mode li').click(function(e) {
    e.preventDefault();

    $('li', $(this).parent()).removeClass('active');
    $(this).addClass('active');

    updateModeOptions.bind(this)();
  });

  $('body').on('click', '.remove-gateway-server', function() {
    $(this).closest('tr').remove();
  });

  for (var i = 0; i < 9; i++) {
    $('.mode-dropdown').append('<li><a href="#" data-mode-value="' + i + '">' + i + '</a></li>');
  }

  $('body').on('click', '.mode-dropdown li a', function(e) {
    updateGroup({mode: $(this).data('mode-value')});
    e.preventDefault();
    return false;
  });

  $('input[name="mode"],input[name="options"],#deviceId').change(function(e) {
    try {
      $.getJSON(
        activeUrl(),
        function(e) {
          handleStateUpdate(e);
        }
      );
    } catch (e) {
      // Skip
    }
  });

  selectize = $('#deviceId').selectize({
    create: true,
    sortField: 'value',
    allowEmptyOption: true,
    render: {
      option: function(data, escape) {
        // Mousedown selects an option -- prevent event from bubbling up to select option
        // when delete button is clicked.
        var deleteBtn = $('<span class="selectize-delete"><a href="#"><i class="glyphicon glyphicon-trash"></i></a></span>')
          .mousedown(function(e) {
            e.preventDefault();
            return false;
          })
          .click(function(e) {
            deleteDeviceId.call($(this).closest('.c-selectize-item'));
            e.preventDefault();
            return false;
          });

        var elmt = $('<div class="c-selectize-item"></div>');
        elmt.append('<span>' + data.text + '</span>');
        elmt.append(deleteBtn);

        return elmt;
      }
    },
    onOptionAdd: function(v, item) {
      var unparsedValue = item.value;
      item.value = parseInt(unparsedValue);
      selectize.updateOption(unparsedValue, item);
      selectize.addItem(item.value);

      saveDeviceIds();
    },
    createFilter: function(v) {
      if (! v.match(/^(0x[a-fA-F0-9]{1,4}|[0-9]{1,5})$/)) {
        deviceIdError("Must be an integer between 0x0000 and 0xFFFF");
        return false;
      }

      var value = parseInt(v);

      if (! (0 <= v && v <= 0xFFFF)) {
        deviceIdError("Must be an integer between 0x0000 and 0xFFFF");
        return false;
      }

      deviceIdError(false);

      return true;
    }
  });
  selectize = selectize[0].selectize;

  var settings = '<ul class="nav nav-tabs" id="setupTabs">';
  var tabClass = 'active';
  UI_TABS.forEach(function(t) {
    settings += '<li class="' + tabClass + '"><a href="#' + t.tag + '" data-toggle="tab">' + t.friendly + '</a></li>';
    tabClass = '';
  });
  settings += '<li><a href="#tab-udp-gateways" data-toggle="tab">UDP</a></li>';
  settings += "</ul>";

  settings += '<div class="tab-content">';

  tabClass = 'active in';

  UI_TABS.forEach(function(t) {
    settings += '<div class="tab-pane fade ' + tabClass + '" id="' + t.tag + '">';
    tabClass = '';
    UI_FIELDS.forEach(function(k) {
      if (k.tab == t.tag) {
        var elmt = '<div class="form-entry">';
        elmt += '<div>';
        elmt += '<label for="' + k.tag + '">' + k.friendly + '</label>';

        if (k.help) {
          elmt += '<div class="field-help" data-help-text="' + k.help + '"></div>';
        }

        elmt += '</div>';

        if (k.type == 'group_state_fields') {
          elmt += '<select class="selectpicker select-init" name="group_state_fields" multiple>';
          GROUP_STATE_KEYS.forEach(function(stateKey) {
            elmt += '<option>' + stateKey + '</option>';
          });
          elmt += '</select>';
        } else if (k.type == 'led_mode') {
          elmt += '<select class="selectpicker select-init" name="' + k.tag + '">';
          LED_MODES.forEach(function(stateKey) {
            elmt += '<option>' + stateKey + '</option>';
          });
          elmt += '</select>';
        } else if (k.type == 'option_buttons') {
          elmt += generateDropdownField(k.tag, k.options, k.settings || {});
        } else {
          elmt += '<input type="text" class="form-control" name="' + k.tag + '"/>';
        }
        elmt += '</div>';

        settings += elmt;
      }
    });
    settings += "</div>";
  });

  // UDP gateways tab
  settings += '<div class="tab-pane fade ' + tabClass + '" id="tab-udp-gateways">';
  settings += $('#gateway-servers-modal .modal-body').remove().html();
  settings += '</div>';

  settings += "</div>";

  $('#settings').prepend(settings);

  $('#settings').submit(function(e) {
    e.preventDefault();

    // Save UDP settings separately from the rest of the stuff since input is handled differently
    if ($('#tab-udp-gateways').hasClass('active')) {
      saveGatewayConfigs();
    } else {
      var obj = $('#settings').serializeArray();

      obj = obj
        .reduce(function(a, x) {
          var val = a[x.name];

          if (! val) {
            a[x.name] = x.value;
          } else if (! Array.isArray(val)) {
            a[x.name] = [val, x.value];
          } else {
            val.push(x.value);
          }

          return a;
        },
        {
          // Make sure the value is always an array, even if a single item is selected
          rf24_channels: []
        });

      // Make sure we're submitting a value for group_state_fields (will be empty
      // if no values were selected).
      obj = $.extend({group_state_fields: []}, obj);

      $.ajax(
        "/settings",
        {
          method: 'put',
          contentType: 'application/json',
          data: JSON.stringify(obj)
        }
      );
    }

    $('#settings-modal').modal('hide');

    return false;
  });

  $('#gateway-server-form').submit(function(e) {
    saveGatewayConfigs();
    e.preventDefault();
    $('#gateway-servers-modal').modal('hide');
    return false;
  });

  $('.field-help').each(function() {
    var elmt = $('<i></i>')
      .addClass('glyphicon glyphicon-question-sign')
      .tooltip({
        placement: 'top',
        title: $(this).data('help-text'),
        container: 'body'
      });
    $(this).append(elmt);
  });

  $('#updates-btn').click(handleCheckForUpdates);

  loadSettings();
  updateModeOptions();
});

$(function() {
  $(document).on('change', ':file', function() {
    var input = $(this),
        label = input.val().replace(/\\/g, '/').replace(/.*\//, '');
    input.trigger('fileselect', [label]);
  });

  $(document).ready( function() {
    $(':file').on('fileselect', function(event, label) {

        var input = $(this).parents('.input-group').find(':text'),
            log = label;

        if( input.length ) {
            input.val(log);
        }
    });
  });
});