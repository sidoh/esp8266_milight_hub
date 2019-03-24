require 'api_client'
require 'mqtt_client'

RSpec.describe 'State' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    @topic_prefix = ENV.fetch('ESPMH_MQTT_TOPIC_PREFIX')

    @client.put(
      '/settings', 
      mqtt_server: ENV.fetch('ESPMH_MQTT_SERVER'),
      mqtt_username: ENV.fetch('ESPMH_MQTT_USERNAME'),
      mqtt_password: ENV.fetch('ESPMH_MQTT_PASSWORD'),
      mqtt_topic_pattern: "#{@topic_prefix}commands/:device_id/:device_type/:group_id",
      mqtt_state_topic_pattern: "#{@topic_prefix}state/:device_id/:device_type/:group_id",
      mqtt_update_topic_pattern: "#{@topic_prefix}updates/:device_id/:device_type/:group_id",
    )

    @mqtt_client = MqttClient.new(
      *%w(SERVER USERNAME PASSWORD).map { |x| ENV.fetch("ESPMH_MQTT_#{x}") } << @topic_prefix
    )
  end

  after(:all) do
    @mqtt_client.disconnect
  end

  before(:each) do
    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
  end

  context 'birth and LWT' do
    # Unfortunately, no way to easily simulate an unclean disconnect, so only test birth
    it 'should send birth message when configured' do
      birth_topic = "#{@topic_prefix}birth"

      @client.put(
        '/settings',
        mqtt_birth_topic: birth_topic
      )

      seen_birth = false

      @mqtt_client.on_message(birth_topic) do |topic, message|
        seen_birth = true
      end

      # Force MQTT reconnect by updating settings
      @client.put('/settings', fakekey: 'fakevalue')

      @mqtt_client.wait_for_listeners

      expect(seen_birth).to be(true)
    end
  end

  context 'commands and state' do
    # Check state using HTTP
    it 'should affect state' do
      @client.patch_state({level: 50, status: 'off'}, @id_params)

      @mqtt_client.patch_state(@id_params, status: 'on', level: 70)
      state = @client.get_state(@id_params)

      expect(state.keys).to      include(*%w(level status))
      expect(state['status']).to eq('ON')
      expect(state['level']).to  eq(70)
    end

    it 'should publish to state topics' do
      desired_state = {'status' => 'ON', 'level' => 80}
      seen_state = false

      @client.patch_state({status: 'off'}, @id_params)

      @mqtt_client.on_state(@id_params) do |id, message|
        seen_state = (id == @id_params && desired_state.all? { |k,v| v == message[k] })
      end

      @mqtt_client.patch_state(@id_params, desired_state)
      @mqtt_client.wait_for_listeners

      expect(seen_state).to be(true)
    end

    it 'should publish an update message for each new command' do
      tweak_params = {'hue' => 49, 'brightness' => 128, 'saturation' => 50}
      desired_state = {'state' => 'ON'}.merge(tweak_params)

      init_state = desired_state.merge(Hash[
        tweak_params.map do |k, v|
          [k, v + 10]
        end
      ])

      @client.patch_state(@id_params, init_state)

      accumulated_state = {}
      @mqtt_client.on_update(@id_params) do |id, message|
        desired_state == accumulated_state.merge!(message)
      end

      @mqtt_client.patch_state(@id_params, desired_state)
      @mqtt_client.wait_for_listeners

      expect(accumulated_state).to eq(desired_state)
    end
  end
end