require 'mqtt_client'

module MqttHelpers
  def mqtt_topic_prefix
    ENV.fetch('ESPMH_MQTT_TOPIC_PREFIX')
  end

  def mqtt_parameters(overrides = {})
    topic_prefix = mqtt_topic_prefix()

    {
      mqtt_server: ENV.fetch('ESPMH_MQTT_SERVER'),
      mqtt_username: ENV.fetch('ESPMH_MQTT_USERNAME'),
      mqtt_password: ENV.fetch('ESPMH_MQTT_PASSWORD'),
      mqtt_topic_pattern: "#{topic_prefix}commands/:device_id/:device_type/:group_id",
      mqtt_state_topic_pattern: "#{topic_prefix}state/:device_id/:device_type/:group_id",
      mqtt_update_topic_pattern: "#{topic_prefix}updates/:device_id/:device_type/:group_id"
    }.merge(overrides)
  end

  def create_mqtt_client(overrides = {})
    params =
      mqtt_parameters
      .merge({topic_prefix: mqtt_topic_prefix()})
      .merge(overrides)

    MqttClient.new(
      ENV['ESPMH_LOCAL_MQTT_SERVER'] || params[:mqtt_server],
      params[:mqtt_username],
      params[:mqtt_password],
      params[:topic_prefix]
    )
  end
end