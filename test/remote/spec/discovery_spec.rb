require 'api_client'

RSpec.describe 'MQTT Discovery' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    @test_id = 1
    @topic_prefix = mqtt_topic_prefix()
    @discovery_prefix = "#{@topic_prefix}discovery/"

    @mqtt_client = create_mqtt_client()
  end

  after(:all) do
    # Clean up any leftover cruft
    @mqtt_client.on_message("#{@discovery_prefix}#", 1, false) do |topic, message|
      if message.length > 0
        @mqtt_client.publish(topic, '', true)
      end
      false
    end
    @mqtt_client.wait_for_listeners
  end

  before(:each) do
    mqtt_params = mqtt_parameters()

    @client.put(
      '/settings',
      mqtt_params
    )

    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @discovery_suffix = "#{@id_params[:type]}_#{sprintf("0x%04x", @id_params[:id])}_#{@id_params[:group_id]}/config"
    @test_discovery_prefix = "#{@discovery_prefix}#{@id_params[:id]}/"
  end

  context 'when not configured' do
    it 'should behave appropriately when MQTT is not configured' do
      @client.patch_settings(mqtt_server: '', home_assistant_discovery_prefix: '')
      expect { @client.get('/settings') }.to_not raise_error
    end

    it 'should behave appropriately when MQTT is configured, but discovery is not' do
      @client.patch_settings(mqtt_parameters().merge(home_assistant_discovery_prefix: ''))
      expect { @client.get('/settings') }.to_not raise_error
    end
  end

  context 'discovery topics' do
    it 'should send discovery messages' do
      saw_message = false

      @mqtt_client.on_message("#{@test_discovery_prefix}light/+/#{@discovery_suffix}") do |topic, message|
        saw_message = true
      end

      @client.patch_settings(
        home_assistant_discovery_prefix: @test_discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      @mqtt_client.wait_for_listeners

      expect(saw_message).to be(true)
    end

    it 'config should have expected keys' do
      saw_message = false
      config = nil

      @mqtt_client.on_message("#{@test_discovery_prefix}light/+/#{@discovery_suffix}") do |topic, message|
        config = JSON.parse(message)
        saw_message = true
      end

      @client.patch_settings(
        home_assistant_discovery_prefix: @test_discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      @mqtt_client.wait_for_listeners

      expect(saw_message).to be(true)
      expected_keys = %w(
        schema
        name
        command_topic
        state_topic
        brightness
        rgb
        color_temp
        effect
        effect_list
        device
      )
      expect(config.keys).to include(*expected_keys)

      expect(config['effect_list']).to include(*%w(white_mode night_mode))
      expect(config['effect_list']).to include(*(0..8).map(&:to_s))
    end

    it 'should list identifiers for ESP and bulb' do
      saw_message = false
      config = nil

      @mqtt_client.on_message("#{@test_discovery_prefix}light/+/#{@discovery_suffix}") do |topic, message|
        config = JSON.parse(message)
        saw_message = config['device'] && config['device']['identifiers'] && config['device']['identifiers'][1] == @id_params[:id]
      end

      @client.patch_settings(
        home_assistant_discovery_prefix: @test_discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      @mqtt_client.wait_for_listeners

      expect(config.keys).to include('device')

      device_data = config['device']

      expect(device_data.keys).to include(*%w(manufacturer sw_version identifiers))
      expect(device_data['manufacturer']).to eq('esp8266_milight_hub')

      ids = device_data['identifiers']
      expect(ids.length).to eq(4)
      expect(ids[1]).to eq(@id_params[:id])
      expect(ids[2]).to eq(@id_params[:type])
      expect(ids[3]).to eq(@id_params[:group_id])
    end

    it 'should remove discoverable devices when alias is removed' do
      seen_config = false
      seen_blank_message = false

      @mqtt_client.on_message("#{@test_discovery_prefix}light/+/#{@discovery_suffix}") do |topic, message|
        seen_config = seen_config || message.length > 0
        seen_blank_message = seen_blank_message || message.length == 0

        seen_config && seen_blank_message
      end

      # This should create the device
      @client.patch_settings(
        home_assistant_discovery_prefix: @test_discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      # This should clear it
      @client.patch_settings(
        group_id_aliases: { }
      )

      @mqtt_client.wait_for_listeners

      expect(seen_config).to be(true)
      expect(seen_blank_message).to be(true), "should see deletion message"
    end

    it 'should configure devices with an availability topic if client status is configured' do
      expected_keys = %w(
        availability_topic
        payload_available
        payload_not_available
      )
      config = nil

      @mqtt_client.on_message("#{@test_discovery_prefix}light/+/#{@discovery_suffix}") do |topic, message|
        config = JSON.parse(message)
        (expected_keys - config.keys).empty?
      end

      # This should create the device
      @client.patch_settings(
        home_assistant_discovery_prefix: @test_discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        },
        mqtt_client_status_topic: "#{@topic_prefix}status",
        simple_mqtt_client_status: true
      )

      @mqtt_client.wait_for_listeners

      expect(config.keys).to include(*expected_keys)
    end
  end
end