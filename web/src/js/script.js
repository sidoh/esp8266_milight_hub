var DEBUG_LEVEL = 0 ; // lover = more verbose
var UNIT_PARAMS = {
  minMireds: 153,
  maxMireds: 370,
  maxBrightness: 255
};

var FORM_SETTINGS = [
  "admin_username", "admin_password",
  "hostname", "static_ip" , "static_mask" , "static_gate", "ota_pass",
  "ce_pin", "csn_pin", "reset_pin",
  "sda_pin", "scl_pin" ,"mqtt_pin1", "mqtt_pin2", "mqtt_pin3", "mqtt_pin4",
  "packet_repeats",
  "http_repeat_factor", "auto_restart_period", "discovery_port", "mqtt_server",
  "mqtt_topic_pattern", "mqtt_update_topic_pattern", "mqtt_state_topic_pattern",
  "mqtt_sensor_topic_pattern", "mqtt_client_id",
  "mqtt_username", "mqtt_password", "radio_interface_type", "listen_repeats",
  "state_flush_interval", "mqtt_state_rate_limit", "packet_repeat_throttle_threshold",
  "packet_repeat_throttle_sensitivity", "packet_repeat_minimum", "group_state_fields"
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
  "bulb_mode"
];

var FORM_SETTINGS_HEADLINES = {
  admin_username : "Frontend Settings",
  hostname : "IP Settings",
  ota_pass : "Over the Air Update Settings",
  ce_pin : "Hardware Settings",
  sda_pin : "I2C Interface Settings",
  packet_repeats : "Milight Settings",
  mqtt_server : "Mqtt Settings",
  radio_interface_type : "2.4G Module Settings"
};

var FORM_SETTINGS_HELP = {
  hostname : "After the next reboot the device is reachable by ne new hostname.",
  ota_pass : "Password for Over The Air programming update",
  sda_pin : "'SDA' for I2C interface",
  scl_pin : "'SCL' for I2C interface" ,
  mqtt_sensor_topic_pattern : "Pattern to publish i2c Sensor updates. Example" +
    "sensors/:sensor_type ",
    mqtt_pin1 : "first pin whose status change is to be sent",
    mqtt_pin2 : "second pin whose status change is to be sent",
    mqtt_pin3 : "third pin whose status change is to be sent",
    mqtt_pin4 : "fourth pin whose status change is to be sent",

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
  state_flush_interval : "Minimum number of milliseconds between flushing state to flash. " +
    "Set to 0 to disable delay and immediately persist state to flash.",
  mqtt_state_rate_limit : "Minimum number of milliseconds between MQTT updates of bulb state. " +
    "Defaults to 500.",
  packet_repeat_throttle_threshold : "Controls how packet repeats are throttled.  Packets sent " +
    "with less time between them than this value (in milliseconds) will cause " +
    "packet repeats to be throttled down.  More than this value will unthrottle " +
    "up.  Defaults to 200ms",
  packet_repeat_throttle_sensitivity : "Controls how packet repeats are throttled. " +
    "Higher values cause packets to be throttled up and down faster.  Set to 0 " +
    "to disable throttling.  Defaults to 1.  Maximum value 1000.",
  packet_repeat_minimum : "Controls how far throttling can decrease the number " +
    "of repeated packets.  Defaults to 3.",
  group_state_fields : "Selects which fields should be included in MQTT state updates and " +
    "REST responses for bulb state."
}

var UDP_PROTOCOL_VERSIONS = [ 2 , 3 , 4 , 5 , 6 ];
var DEFAULT_UDP_PROTOCL_VERSION = 5;


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
      $.ajax({
        url: activeUrl(),
        method: 'PUT',
        data: JSON.stringify(params),
        contentType: 'application/json',
        success: function(e) {
          console.log(e);
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
);

var gatewayServerRow = function(deviceIdHex, port, version , name) {
  debug( 5 , 'gatewayServerRow');
  var elmt = '<tr><td class="col col-sm-2">';
  elmt += '<div class="form-group  has-feedback">';
  elmt += '<input name="deviceIds[]" class="form-control" value="' + deviceIdHex + '" placeholder="Enter Device ID" ';
  if(deviceIdHex){
    elmt += 'disabled';
  }
  elmt += ' />';
  elmt += '<span class="glyphicon form-control-feedback"></span>';
  elmt += '<span class="help-block"></span>';
  elmt += '</div>';
  elmt += '</td>';

  elmt += '<td class="col col-sm-2"><div class="form-group  has-feedback">';
  elmt += '<input name="ports[]" class="form-control" value="' + port + '" placeholder="Enter Port" ';
  if(deviceIdHex){
    elmt += 'disabled';
  }
  elmt += ' />';
  elmt += '<span class="glyphicon form-control-feedback"></span>';
  elmt += '<span class="help-block"></span>';
  elmt += '</div></td>';

  elmt += '<td class="col col-sm-2">'
  if(deviceIdHex){
    debug( 0 , 'gatewayRow containig data');
    elmt += '<span class="help-block" name="deviceNames[]">' + name + '<span>';
  }else {
    debug( 2 , 'gatewayRow empty');
    elmt += '<input name="deviceNames[]" class="form-control" value="" placeholder="' + name + '"/>';
  }
  elmt += '</td>';




    elmt += '<td class="col col-sm-2" ><div class="btn-group' + '" data-toggle="buttons">';
    for (var i = 0; i < UDP_PROTOCOL_VERSIONS.length; i++) {
      var val = UDP_PROTOCOL_VERSIONS[i]
        , selected = (version == val || (val == DEFAULT_UDP_PROTOCL_VERSION && !UDP_PROTOCOL_VERSIONS.includes(version)));
      elmt += '<label class="btn btn-secondary' + (deviceIdHex ? ' disabled' : '')   + (selected ? ' active' : '') + '">';
      elmt += '<input type="radio" name="versions[]" autocomplete="off" data-value="' + val + '" '
      elmt +=  (deviceIdHex ? 'disabled="disabled" ' : '') +  (selected ? 'checked' : '') +'> ' + val;
      elmt += '</label>';
    }
    elmt += '</div></td>';


  elmt += '<td class="col col-sm-4">';
  if(deviceIdHex){
    elmt += ' <button class="btn btn-warning edit-gateway-btn">';
    elmt += '<span class="glyphicon glyphicon-pencil"></span> Edit Gateway';
    elmt += '</button>';

    elmt += ' <button class="btn btn-success update-gateway-btn" style="display: none;" >';
    elmt += '<span class="glyphicon glyphicon-ok"></span> Save Gateway';
    elmt += '</button>';

    elmt += ' <button class="btn btn-danger remove-gateway-btn">';
    elmt += '<span class="glyphicon glyphicon-remove"></span> Delete Gateway';
    elmt += '</button>';
}else {
  elmt += ' <button class="btn btn-success add-gateway-btn" disabled>';
  elmt += '<span class="glyphicon glyphicon-ok"></span> Add Gateway';
  elmt += '</button>';
}

  elmt += '</td>';
  elmt += '</tr>';
  return elmt;
}



var deviceIdRow = function(deviceIdHex, deviceIdDec , name) {
  debug( 5 , 'deviceIdRow');
  var elmt = '<tr>';
  elmt += '<td class="col col-sm-2">';
  elmt += '<div class="form-group  has-feedback">';
  elmt += '<input type="text" class="form-control" name="deviceIds[]" value="' + deviceIdHex + '" placeholder="Enter hub ID" ';
  if(deviceIdDec){
    elmt += 'disabled';
  }
  elmt += ' />';
  elmt += '<span class="glyphicon form-control-feedback"></span>';
  elmt += '<span class="help-block"></span>';
  elmt += '</div>';
  elmt += '</td>';

  elmt += '<td class="col col-sm-7">';
  if(deviceIdDec){
    debug( 0 , 'deviceRow containig data');
    elmt += '<span class="help-block" name="deviceNames[]" >' + name +'<span>';
  }else {
    debug( 2 , 'deviceRow empty');
    elmt += '<input name="deviceNames[]" class="form-control" value="" placeholder="Enter hub name ' + name + '"/>';
  }
  elmt += '</td>';

  elmt += '<td class="col col-sm-3">';
  //elmt += '<div class="btn-group">';
  if(deviceIdDec){
    elmt += ' <button class="btn btn-warning edit-device-btn">';
    elmt += '<span class="glyphicon glyphicon-pencil"></span> Edit Device';
    elmt += '</button>';

    elmt += ' <button class="btn btn-success update-device-btn" style="display: none;" >';
    elmt += '<span class="glyphicon glyphicon-ok"></span> Save Device';
    elmt += '</button>';

    elmt += ' <button class="btn btn-danger remove-device-btn">';
    elmt += '<span class="glyphicon glyphicon-remove"></span> Delete Device';
    elmt += '</button>';
}else {
  elmt += ' <button class="btn btn-success add-device-btn" disabled>';
  elmt += '<span class="glyphicon glyphicon-ok"></span> Add Device';
  elmt += '</button>';
}

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
          if(field.val().length > 50){
              field.width('40em');
          }else if(field.val().length < 6 && field.val().length > 0  ){
              field.width('5em');
          }else{
              field.width('20em');
          }
        }
      }
    });


    if (val.device_ids) {
      val.device_ids.forEach(function(v) {
        $('#deviceId').append($('<option>', {
            value: v,
            text: toHex(v)
        }));
      });
      $('#deviceId').selectpicker('refresh');
      $('#deviceId').selectpicker('render');
    }

<<<<<<< HEAD
    var deviceForm = $('#device-id-configs');
    if (val.device_configs && Array.isArray(val.device_configs)){
      val.device_configs.forEach(function(v) {
        //deviceIdRow( hex , dec , text )
  //      deviceForm.append(deviceIdRow( v[0] ,  parseInt(v[0] , 16) , v[1]));
        if(v.length == 3){
          deviceForm.append(deviceIdRow(toHex(v[0]), v[0] , v[2]));
        }
      });
    }else if (val.device_ids) {
        val.device_ids.forEach(function(v) {
            deviceForm.append(deviceIdRow(toHex(v), v , 'unnamed device'));
        });
    }
    deviceForm.append(deviceIdRow('', '' , 'new Device'));

=======
    if (val.group_state_fields) {
      var elmt = $('select[name="group_state_fields"]');
      elmt.selectpicker('val', val.group_state_fields);
    }
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c

    var gatewayForm = $('#gateway-server-configs').html('');
    if (val.gateway_configs && Array.isArray(val.gateway_configs)) {
      val.gateway_configs.forEach(function(v) {
        //gatewayServerRow( hex , port , protocol , text)
        if(v.length == 3){
          gatewayForm.append(gatewayServerRow(toHex(v[0]), v[1], v[2] , 'unnamed gateway' ));
        }else if(v.length == 4){
          gatewayForm.append(gatewayServerRow(toHex(v[0]), v[1], v[2] , v[3]));
        }
      });
    }
    gatewayForm.append(gatewayServerRow('','' , null , 'new Gateway'));


  });

};

var loadStatistics = function() {
  $.getJSON('/statistics', function(val) {
  var elmt= '';
  Object.keys(val).forEach(function(k) {
    elmt +='<div class="col-sm-12" >' +
    '<h4 >' + k  +
    '<input class="form-entry form-control col-sm-xs" name="' + k + '" value="' +val[k] + '" style="width:5em; display:inline;position:absolute;left: 150px" disabled ></div>' +
    '</h4>';
  });


    $('#statistics').prepend(elmt);
  });
};



function saveGatewayConfigs( send ) {
  var form = $('#gateway-server-form');
  var invalid = {};
  debug( 5 , 'saveGatewayConfigs');

  var deviceIds = $('input[name="deviceIds[]"]', form).map(function(i, v) {
    var val = parseInt($(v).val() , 16);
    debug( 0 , 'Gateway ID dec ' + val);

    if (isNaN(val)) {
      debug( 8 , 'ERROR Gateway ID d ' + i );
      invalid[i] = true;
      return null;
    } else {
      return val;
    }
  });

  var names = $('span[name="deviceNames[]"]', form).map(function(i, v) {
    var val = $(v).text();
    debug( 0 , 'Gateway Name ' + val);


    if (v == undefined ) {
      debug( 8 , 'ERROR Gateway ID Name ' + i );
      invalid[i] = true;
      return null;
    } else {
      return val;
    }
  });

  var ports = $('input[name="ports[]"]', form).map(function(i, v) {
    var val = $(v).val();
    debug( 0 , 'Gateway Port ' + val);

    if (isNaN(val)) {
      debug( 8 , 'ERROR Gateway Port ' + i );
      invalid[i] = true;
      return null;
    } else {
      return val;
    }
  });

  var versions = $('.active input[name="versions[]"]', form).map(function(i, v) {
    var val = $(v).data('value');
    debug( 0 , 'Gateway Version ' + val);
    return val;
  });


    var data = [];
    var emerc = false;
    debug( 3 , 'Gateway ' + deviceIds.length  + ' entrys found');
    for (var i = 0; i < deviceIds.length ; i++) {
      if(!invalid[i]){
        data[i] = [deviceIds[i], ports[i], versions[i]];
        //TODO new Version needs to be supported by Device
        // data[i] = [deviceIds[i], ports[i], versions[i] , names[i]];
        debug( 0 , 'Gateway nr '+i+' : hex ' + deviceIds[i] +' port '+ ports[i] +' version '+ versions[i] +' text '+ names[i] );
      }else {
        debug( 9 , 'ERROR @ Gateway nr '+i+' : hex ' + deviceIds[i] +' port '+ ports[i] +' version '+ versions[i] +' text '+ names[i] );
        if( i < deviceIds.length ){
          $(v).closest('tr').removeClass('danger');
          emerc = true;
        }
      }
    }

    if(emerc){
      debug( 9 , 'REEORS IN saveGatewayConfigs ');
      return false;
    }

    if(send){
      $.ajax(
        '/settings',
        {
          method: 'put',
          contentType: 'application/json',
          data: JSON.stringify({gateway_configs: data})
        });
    }else{
      return data;
    }
};

function debug( level , msg){
  if(level >= DEBUG_LEVEL )
  console.log(msg);
}

function saveDeviceConfigs( send ) {
  var form = $('#device-id-form');
  var invalid = {};

  debug( 5 , 'saveDeviceConfigs');

  var deviceIds = $('input[name="deviceIds[]"]', form).map(function(i, v) {
    var val = $(v).val();
    debug( 0 , 'Device ID Hex ' + val );

    if (isNaN(val) || val == 0 ) {
      debug( 8 , 'ERROR Device ID h ' + i );
      invalid[i] = true;
      return null;
    } else {
      return val;
    }
  });

  var deviceNumbs = $('input[name="deviceIds[]"]', form).map(function(i, v) {
    var val = parseInt($(v).val() , 16);
    debug( 0 , 'Device ID dec ' + val );

    if (isNaN(val)) {
      debug( 8 , 'ERROR Device ID d ' + i );
      invalid[i] = true;
      return null;
    } else {
      return val;
    }
  });

  var names = $('span[name="deviceNames[]"]', form).map(function(i, v) {
    var val = $(v).text();
    debug( 0 , 'Device Name ' + val );

    if (v == undefined ) {
        invalid[i] = true;
        debug( 8 , 'ERROR Device Name ' + i );
      return null;
    } else {
      return val;
    }
  });


    var data = [];
    var emerc = false;
    debug( 3 , 'Device ' + deviceIds.length  + ' entrys found');
    for (var i = 0; i < deviceIds.length ; i++) {
      if(!invalid[i]){
        data[i] = [ deviceNumbs[i] , deviceIds[i] , names[i]];
        debug( 0 , 'Device nr '+i+' : dec ' + deviceNumbs[i] +' hex '+ deviceIds[i] +' text '+ names[i] );
      }else {
        debug( 9 , 'ERROR @ Device nr '+i+' : dec ' + deviceNumbs[i] +' hex '+ deviceIds[i] +' text '+ names[i] );
        if( i < deviceIds.length ){
          $(v).closest('tr').removeClass('danger');
          emerc = true;
        }
      }
    }

    if(emerc){
      debug( 8 , 'REEORS IN saveDeviceConfigs ');
      return false;
    }

    if(send){
      $.ajax(
        '/settings',
        {
          method: 'put',
          contentType: 'application/json',
          data: JSON.stringify({device_configs: data})
        });
    }else{
      return data;
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
  debug( 5 , 'handleCheckForUpdates');
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
  console.log(state);
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

// remove
    $('#gateway-server-form').on('click', '.remove-gateway-btn', function() {
      debug( 5 ,'remove-gateway' );
      $(this).closest('tr').remove();
      return false;
    });

    $('#device-id-form').on('click', '.remove-device-btn', function() {
      debug( 5 ,'remove-device' );
      $(this).closest('tr').remove();
      return false;
    });

//edit
    $('#gateway-server-form').on('click', '.edit-gateway-btn', function() {
      debug( 5 ,'edit-gateway' );
      var activeTr = $(this).closest('tr');
      idCheck(activeTr.find('input[name="deviceIds[]"]'));
      gatewayServerPortCheck(activeTr.find('input[name="ports[]"]'));
      activeTr.find('input[name="deviceIds[]"]').prop('disabled', false);
      activeTr.find('input[name="ports[]"]').prop('disabled', false);
      activeTr.find('input[name="versions[]"]').closest('.btn-group').find('.btn').removeClass('disabled');
      activeTr.find('.edit-gateway-btn').hide();
      activeTr.find('.update-gateway-btn').show();
      var $el = activeTr.find('span[name="deviceNames[]"]');
      var $input = $('<input name="deviceNames[]" class="form-control" />').val( $el.text() );
      $el.replaceWith( $input );
      return false;
    });

    $('#device-id-form').on('click', '.edit-device-btn', function() {
      debug( 5 ,'edit-device' );
      var activeTr = $(this).closest('tr');
      idCheck(activeTr.find('input[name="deviceIds[]"]'));
      activeTr.find('input[name="deviceIds[]"]').prop('disabled', false);
      activeTr.find('.edit-device-btn').hide();
      activeTr.find('.update-device-btn').show();
      var $el = activeTr.find('span[name="deviceNames[]"]');
      var $input = $('<input name="deviceNames[]" class="form-control" />').val( $el.text() );
      $el.replaceWith( $input );
      return false;
    });

//update
  $('#gateway-server-form').on('click', '.update-gateway-btn', function() {
      debug( 5 ,'update-gateway' );
      var activeTr = $(this).closest('tr');
      activeTr.find('input[name="deviceIds[]"]').prop('disabled', true);
      activeTr.find('input[name="ports[]"]').prop('disabled', true);
      activeTr.find('input[name="versions[]"]').closest('.btn-group').find('.btn').addClass('disabled');
      activeTr.find('.update-gateway-btn').hide();
      activeTr.find('.edit-gateway-btn').show();
      var $input = activeTr.find('input[name="deviceNames[]"]');
      var $el = $('<span class="help-block" name="deviceNames[]" />').text( $input.val() );
      $input.replaceWith( $el );
      return false;
    });

    $('#device-id-form').on('click', '.update-device-btn', function() {
      debug( 5 ,'update-device' );
      var activeTr = $(this).closest('tr');
      activeTr.find('input[name="deviceIds[]"]').prop('disabled', true);
      activeTr.find('.update-device-btn').hide();
      activeTr.find('.edit-device-btn').show();
      var $input = activeTr.find('input[name="deviceNames[]"]');
      var $el = $('<span class="help-block" name="deviceNames[]" />').text( $input.val() );
      $input.replaceWith( $el );
      return false;
    });

//add
  $('#gateway-server-form').on('click', '.add-gateway-btn', function() {
      debug( 5 ,'add-gateway' );
      var actEntry = $(this).closest('tr');
      $('#gateway-server-configs').append(gatewayServerRow(
        actEntry.find('input[name="deviceIds[]"]').val(),
        actEntry.find('input[name="ports[]"]').val(),
        actEntry.find('.active input[name="versions[]"]').data('value'),
        actEntry.find('input[name="deviceNames[]"]').val()
      ));
      actEntry.remove();
    $('#gateway-server-configs').append(gatewayServerRow('', '' , '' , 'next Gateway'));
    return false;
  });

  $('#device-id-form').on('click', '.add-device-btn', function() {
    debug( 5 ,'add-device' );
      var actEntry = $(this).closest('tr');
      $('#device-id-configs').append(deviceIdRow(
        actEntry.find('input[name="deviceIds[]"]').val(),
        parseInt(actEntry.find('input[name="deviceIds[]"]').val() , 16 ),
        actEntry.find('input[name="deviceNames[]"]').val()
      ));
      actEntry.remove();
    $('#device-id-configs').append(deviceIdRow('', '' , 'next Device'));
    return false;
  });


  $('#device-id-form').on('click', '.save-device-btn', function() { saveDeviceConfigs(true); });
  $('#gateway-server-form').on('click', '.save-gateway-btn' , function() { saveGatewayConfigs(true); });
  $('#settings').on('click', '.save-settings-btn' , function() { saveSettingConfigs(true); });
  $('#control_lights').on('click', '.save-ids-btn' , function() { saveIDConfigs(true); });

  $('#gateway-server-form').on('input', 'input[name="deviceIds[]"]', gatewayServerCheck );
  $('#gateway-server-form').on('input', 'input[name="ports[]"]', gatewayServerCheck );
  $('#device-id-form').on('input', 'input[name="deviceIds[]"]', deviceIdCheck );

  $('body').on('click', '.btn.disabled', function(){ return false; });


  function gatewayServerPortCheck(obj){
    debug( 5 , 'gatewayServerPortCheck');
    var activeTd = obj.closest('td');
    var v = activeTd.find('input[name="ports[]"]').val();
    var fb = activeTd.find('.has-feedback');
    var gl = activeTd.find('.form-control-feedback');
    var he =  activeTd.find('.help-block');
    var ok = true ;

    if(v == undefined  ){
      ok = false;
      return false;
    }

    if( v.length < 1 ){
      ok = false;
      he.html("at wich port?");
      return false;
    }

    if (v == '') {
      ok = false;
      he.html("Connect to which Port?");
    }

    if (! /^\d+$/.test(v)) {
      ok = false;
      he.html("Port should be Numeric");
    }

    v = parseInt(v);
    if ( v < 1 || v > 65535 ) {
      ok = false;
      he.html("out of Range 1-65535");
    }

    if(!ok){
      fb.removeClass('has-success');
      fb.addClass('has-warning');
      gl.removeClass('glyphicon-ok');
      gl.addClass('glyphicon-warning-sign');
      return false;
    }else{
      fb.addClass('has-success');
      fb.removeClass('has-warning');
      gl.addClass('glyphicon-ok');
      gl.removeClass('glyphicon-warning-sign');
      he.html("");
      return true;
    }
  };

  function gatewayServerCheck(){
    debug( 5 , 'gatewayServerCheck');
    var activeTr =  $(this).closest('tr');
    var ok = true;

    if(!idCheck(activeTr.find('input[name="deviceIds[]"]'))){
        ok = false;
    }
    if(!gatewayServerPortCheck(activeTr.find('input[name="ports[]"]'))){
        ok = false;
    }
    if(!ok){
      activeTr.find('.add-gateway-btn').prop('disabled', true);
      activeTr.find('.update-gateway-btn').prop('disabled', true);
      return false;
    }else {
      activeTr.find('.add-gateway-btn').prop('disabled', false);
      activeTr.find('.update-gateway-btn').prop('disabled', false);
      return true;
    }

  };


  function deviceIdCheck(){
    debug( 5 , 'deviceIdCheck');
    var activeTr =  $(this).closest('tr');
    var ok = true;
    if(!idCheck(activeTr.find('input[name="deviceIds[]"]'))){
      ok = false;
    }
    if(!ok){
      activeTr.find('.add-device-btn').prop('disabled', true);
      activeTr.find('.update-device-btn').prop('disabled', true);
      return false;
    }else{
      activeTr.find('.add-device-btn').prop('disabled', false);
      activeTr.find('.update-device-btn').prop('disabled', false);
      return true;
    }
  };


  function idCheck(obj){
    debug( 5 , 'idCheck');
    var activeTd = obj.closest('td');
    var v = activeTd.find('input[name="deviceIds[]"]').val();
    var fb = activeTd.find('.has-feedback');
    var gl = activeTd.find('.form-control-feedback');
    var he =  activeTd.find('.help-block');
    var ok = true ;

    if(v == undefined ){
      //something went wrong
      return false;
    }

    if (! v.match(/^(0x[a-fA-F0-9]{1,4}|[0-9]{1,5})$/)) {
      ok = false;
      he.html("needs an integer between 0x0000 and 0xFFFF");
    }

    var value = parseInt(v);
    if (! (1 <= v && v <= 0xFFFF)) {
      ok = false;
      he.html("integer between 0x0000 and 0xFFFF expected");
    }

    if(!ok){
      fb.removeClass('has-success');
      fb.addClass('has-warning');
      gl.removeClass('glyphicon-ok');
      gl.addClass('glyphicon-warning-sign');
      return false;
    }else{
      fb.addClass('has-success');
      fb.removeClass('has-warning');
      gl.addClass('glyphicon-ok');
      gl.removeClass('glyphicon-warning-sign');
      he.html("");
      return true;
    }
  };







  $('#mode').change(updateModeOptions);



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



  var settings = "";

  FORM_SETTINGS.forEach(function(k) {
    var elmt = '';

    if(FORM_SETTINGS_HEADLINES[k]){
      elmt += '</div> <div class="row header-row">' +
        '<div class="col-sm-12" >' +
        '<h3 data-toggle="collapse" data-target="#setting-part-'+k+'" >'+
          '<span class="caret"> </span> ' +
           FORM_SETTINGS_HEADLINES[k] +'</h3>' +
        '</div>' +
      '</div>' +
      '<div id="setting-part-'+k+'" class="collapse in">' ;
    }

    elmt += '<div class="form-entry">';
    elmt += '<div>';
    elmt += '<label for="' + k + '">' + k + '</label>';

    if (FORM_SETTINGS_HELP[k]) {
      elmt += '<div class="field-help" data-help-text="' + FORM_SETTINGS_HELP[k] + '"></div>';
    }

    elmt += '</div>';

    if (k === "radio_interface_type") {
      elmt += '<div class="btn-group" id="radio_interface_type" data-toggle="buttons">' +
        '<label class="btn btn-secondary active">' +
          '<input type="radio" id="nrf24" name="radio_interface_type" autocomplete="off" value="nRF24" /> nRF24' +
        '</label>'+
        '<label class="btn btn-secondary">' +
          '<input type="radio" id="lt8900" name="radio_interface_type" autocomplete="off" value="LT8900" /> PL1167/LT8900' +
        '</label>' +
      '</div>';
<<<<<<< HEAD
      elmt += '</div>';
=======
    } else if (k == 'group_state_fields') {
      elmt += '<select class="selectpicker" name="group_state_fields" multiple>';
      GROUP_STATE_KEYS.forEach(function(stateKey) {
        elmt += '<option>' + stateKey + '</option>';
      });
      elmt += '</select>';
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c
    } else {
      elmt += '<input type="text" class="form-control" name="' + k + '"/>';
      elmt += '</div>';
    }

    settings += elmt ;
  });

<<<<<<< HEAD

  $('#settings').prepend('<div >' + settings + '</div>');


  function saveSettingConfigs( send ){
    debug( 5 , 'saveSettingConfigs');
    var device_ids = {};
    var data = {};
=======
  $('#settings').prepend(settings);
  $('#settings').submit(function(e) {
    e.preventDefault();
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c

    var obj = $('#settings')
      .serializeArray()
      .reduce(function(a, x) {
        var val = a[x.name];

<<<<<<< HEAD
      if (elmt.attr('type') === 'radio') {
        data[k] = elmt.filter(':checked').val();
        debug( 0 , 'Setting Radio '+k+' : ' + elmt.filter(':checked').val());
      } else {
        data[k] = elmt.val();
        debug( 0 , 'Setting Normal '+k+' : ' + elmt.val());
      }
    });
=======
        if (! val) {
          a[x.name] = x.value;
        } else if (! Array.isArray(val)) {
          a[x.name] = [val, x.value];
        } else {
          val.push(x.value);
        }

        return a;
      }, {});
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c

    var form = $('#device-id-configs');
    device_ids = $('input[name="deviceIds[]"]', form).map(function(i, v) {
      var dec = parseInt($(v).val() , 16 );
      debug( 0 , 'Setting Device '+ dec );
        if(isNaN(dec) || dec == ''){
          return null;
        }else {
          return dec;
        }
      }).toArray();

      data['device_ids'] = device_ids;
    //  for (var i = 0; i < device_ids.length -1 ; i++) {
    //    data.device_ids = device_ids;
    //  }

      if(send){
        $.ajax(
          '/settings',
          {
            method: 'put',
            contentType: 'application/json',
            data: JSON.stringify({setting_configs: data})
          });
      }else {
        return data;
      }
  }

<<<<<<< HEAD


  function saveIdConfigs( send ){
    var data = {};
    debug( 5 , 'saveIdConfigs');
    var form = $('#device-id-configs');
    data = $('input[name="deviceIds[]"]', form).map(function(i, v) {
      var dec = parseInt($(v).val() , 16 );
      debug( 0 , 'ID '+ dec );
        if(isNaN(dec) || dec == ''){
          return null;
        }else {
          return dec;
        }
      }).toArray();

      if(send){
        $.ajax(
          '/settings',
          {
            method: 'put',
            contentType: 'application/json',
            data: JSON.stringify({id_configs: data})
          });
      }else {
        return data;
=======
    // Make sure we're submitting a value for group_state_fields (will be empty
    // if no values were selected).
    obj = $.extend({group_state_fields: []}, obj);

    $.ajax(
      "/settings",
      {
        method: 'put',
        contentType: 'application/json',
        data: JSON.stringify(obj)
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c
      }
  }

<<<<<<< HEAD
=======
    return false;
  });
>>>>>>> fc89ce4eacb6cec29dbdda4a5adeca252723a89c

  $('#settings').submit(function (e){
      var obj = {};

      //settings in old structure
      obj = saveSettingConfigs(false);

      //deviceIds dec basic
      obj.device_ids = saveIdConfigs(false);

      //settings in new structure
      obj.setting_configs = saveSettingConfigs(false);

      //deviceIds extendet
      obj.device_configs = saveDeviceConfigs(false);
      if(obj.device_configs === false ){ // to prevent sending in exeption
        delete obj.device_configs;
      }

      //gateways
      obj.gateway_configs = saveGatewayConfigs(false);
      if(obj.gateway_configs === false ){ // to prevent sending in exeption
        delete obj.gateway_configs;
      }

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
  loadStatistics();
  updateModeOptions();
});
