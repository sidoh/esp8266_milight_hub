require 'mqtt'
require 'api_client'

RSpec.describe 'State' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    @client.put(
      '/settings', 
      mqtt_server: ENV.fetch('ESPMH_MQTT_SERVER'),
      mqtt_username: ENV.fetch('ESPMH_MQTT_USERNAME'),
      mqtt_password: ENV.fetch('ESPMH_MQTT_PASSWORD'),
    )

    @topic_prefix = ENV.fetch('ESPMH_MQTT_TOPIC_PREFIX')
  end

  context 'birth and LWT' do
    it 'should send birth and LWT messages when configured' do
      lwt_topic = "#{@topic_prefix}lwt"
      birth_topic = "#{@topic_prefix}birth"

      seen_birth = false
      seen_lwt = false

      MQTT::Client.connect("mqtt://#{ENV.fetch('ESPMH_MQTT_USERNAME')}:#{ENV.fetch('ESPMH_MQTT_PASSWORD')}@#{ENV.fetch('ESPMH_MQTT_SERVER')}") do |c|
        birth_listen_thread = Thread.new do
          begin
            Timeout.timeout(10) do
              c.get(birth_topic)
              seen_birth = true
            end
          rescue Timeout::Error 
          end
        end

        lwt_listen_thread = Thread.new do
          begin
            Timeout.timeout(10) do
              c.get(lwt_topic)
              seen_lwt = true
            end
          rescue Timeout::Error
          end
        end

        @client.put(
          '/settings',
          mqtt_lwt_topic: lwt_topic,
          mqtt_lwt_message: 'disconnected',
          mqtt_birth_topic: birth_topic
        )
        @client.post('/system', command: 'restart')

        lwt_listen_thread.join
        birth_listen_thread.join
      end

      expect(seen_birth).to be(true)
      expect(seen_lwt).to be(true)
    end
  end
end