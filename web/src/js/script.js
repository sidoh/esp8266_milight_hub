var FORM_SETTINGS = [
  "admin_username", "admin_password", "ce_pin", "csn_pin", "reset_pin","packet_repeats",
  "http_repeat_factor", "auto_restart_period", "discovery_port", "mqtt_server",
  "mqtt_topic_pattern", "mqtt_update_topic_pattern", "mqtt_state_topic_pattern",
  "mqtt_username", "mqtt_password", "radio_interface_type", "listen_repeats",
  "state_flush_interval", "mqtt_state_rate_limit"
];

var FORM_SETTINGS_HELP = {
  ce_pin : "'CE' for NRF24L01 interface, and 'PKT' for 'PL1167/LT8900' interface",
  packet_repeats : "The number of times to repeat RF packets sent to bulbs",
  http_repeat_factor : "Multiplicative factor on packet_repeats for " +
    "requests initiated by the HTTP API. UDP API typically receives " +
    "duplicate packets, so more repeats should be used for HTTP.",
  auto_restart_period : "Automatically restart the device every number of " +
    "minutes specified. Use 0 for disabled.",
  radio_interface_type : "2.4 GHz radio model. Only change this if you know " +
    "You're not using an NRF24L01!",
  mqtt_server : "Domain or IP address of MQTT broker. Optionally specify a port " +
    "with (example) mymqqtbroker.com:1884.",
  mqtt_topic_pattern : "Pattern for MQTT topics to listen on. Example: " +
    "lights/:device_id/:device_type/:group_id. See README for further details.",
  mqtt_update_topic_pattern : "Pattern to publish MQTT updates. Packets that " +
    "are received from other devices, and packets that are sent from this device will " +
    "result in updates being sent.",
  mqtt_state_topic_pattern : "Pattern for MQTT topic to publish state to. When a group " +
    "changes state, the full known state of the group will be published to this topic " +
    "pattern.",
  discovery_port : "UDP port to listen for discovery packets on. Defaults to " +
    "the same port used by MiLight devices, 48899. Use 0 to disable.",
  listen_repeats : "Increasing this increases the amount of time spent listening for " +
    "packets. Set to 0 to disable listening. Default is 3.",
  state_flush_interval : "Number of seconds between flushing states from RAM to flash. " +
    "Set to 0 to disable delay and immediately persist state to flash.",
  mqtt_state_rate_limit : "Minimum number of milliseconds between MQTT updates of bulb state. " +
    "Defaults to 500."
}

var UDP_PROTOCOL_VERSIONS = [ 5, 6 ];
var DEFAULT_UDP_PROTOCL_VERSION = 5;

var selectize;
var sniffing = false;

var webSocket = new WebSocket("ws://" + location.hostname + ":81");
webSocket.onmessage = function(e) {
  if (sniffing) {
    var message = e.data;
    $('#sniffed-traffic').prepend('<pre>' + message + '</pre>');
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

  return "/gateways/" + deviceId + "/" + mode + "/" + groupId;
}

var getCurrentMode = function() {
  return $('input[name="mode"]:checked').data('value');
};

var updateGroup = _.throttle(
  function(params) {
    try {
      $.ajax(
        activeUrl(),
        {
          method: 'PUT',
          data: JSON.stringify(params),
          contentType: 'application/json'
        }
      );
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
  $.getJSON('/settings', function(val) {
    Object.keys(val).forEach(function(k) {
      var field = $('#settings input[name="' + k + '"]');

      if (field.length > 0) {
        if (field.attr('type') === 'radio') {
          field.filter('[value="' + val[k] + '"]').click();
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

    var gatewayForm = $('#gateway-server-configs').html('');
    if (val.gateway_configs) {
      val.gateway_configs.forEach(function(v) {
        gatewayForm.append(gatewayServerRow(toHex(v[0]), v[1], v[2]));
      });
    }
  });
};

var saveGatewayConfigs = function() {
  var form = $('#gateway-server-form')
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

var deviceIdError = function(v) {
  if (!v) {
    $('#device-id-label').removeClass('error');
  } else {
    $('#device-id-label').addClass('error');
    $('#device-id-label .error-info').html(v);
  }
};

var updateModeOptions = function() {
  var currentMode = getCurrentMode();

  $('.mode-option').map(function() {
    if ($(this).data('for').split(',').includes(currentMode)) {
      $(this).show();
    } else {
      $(this).hide();
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
  $('#updates-modal').modal();

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
    $('input[name="status"]').prop('checked', state.state == 'ON').click();
  }
  if (state.color) {
    var hsv = RGBtoHSV(state.color.r, state.color.g, state.color.b);
    $('input[name="saturation"]').slider('setValue', hsv.s*100);
    updatePreviewColor(hsv.h*360,hsv.s*100,hsv.v*100);
  }
};

var updatePreviewColor = function(hue, saturation, value) {
  if (! saturation) {
    saturation = 100;
  }
  if (! value) {
    value = 50;
  }
  $('.hue-value-display').css({
    backgroundColor: "hsl(" + hue + "," + saturation + "%," + value + "%)"
  });
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
      , val = $(this).attr('type') == 'checkbox' ? $(this).is(':checked') : $(this).val()
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

  $('#sniff').click(function() {
    if (sniffing) {
      sniffing = false;
      $(this).html('Start Sniffing');
    } else {
      sniffing = true;
      $(this).html('Stop Sniffing');
    }
  });

  $('#add-server-btn').click(function() {
    $('#gateway-server-configs').append(gatewayServerRow('', ''));
  });

  $('#mode').change(updateModeOptions);

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
    sortField: 'text',
    onOptionAdd: function(v, item) {
      item.value = parseInt(item.value);
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

  var settings = "";

  FORM_SETTINGS.forEach(function(k) {
    var elmt = '<div class="form-entry">';
    elmt += '<div>';
    elmt += '<label for="' + k + '">' + k + '</label>';

    if (FORM_SETTINGS_HELP[k]) {
      elmt += '<div class="field-help" data-help-text="' + FORM_SETTINGS_HELP[k] + '"></div>';
    }

    elmt += '</div>';

    if(k === "radio_interface_type") {
      elmt += '<div class="btn-group" id="radio_interface_type" data-toggle="buttons">' +
        '<label class="btn btn-secondary active">' +
          '<input type="radio" id="nrf24" name="radio_interface_type" autocomplete="off" value="nRF24" /> nRF24' +
        '</label>'+
        '<label class="btn btn-secondary">' +
          '<input type="radio" id="lt8900" name="radio_interface_type" autocomplete="off" value="LT8900" /> PL1167/LT8900' +
        '</label>' +
      '</div>';
    } else {
      elmt += '<input type="text" class="form-control" name="' + k + '"/>';
      elmt += '</div>';
    }

    settings += elmt;
  });

  $('#settings').prepend(settings);
  $('#settings').submit(function(e) {
    var obj = {};

    FORM_SETTINGS.forEach(function(k) {
      var elmt = $('#settings input[name="' + k + '"]');

      if (elmt.attr('type') === 'radio') {
        obj[k] = elmt.filter(':checked').val();
      } else {
        obj[k] = elmt.val();
      }
    });

    // pretty hacky. whatever.
    obj.device_ids = _.map(
      $('.selectize-control .option'),
      function(x) {
        return $(x).data('value')
      }
    );

    $.ajax(
      "/settings",
      {
        method: 'put',
        contentType: 'application/json',
        data: JSON.stringify(obj)
      }
    );

    e.preventDefault();
    return false;
  });

  $('#gateway-server-form').submit(function(e) {
    saveGatewayConfigs();
    e.preventDefault();
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
