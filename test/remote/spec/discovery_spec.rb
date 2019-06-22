require 'api_client'

RSpec.describe 'MQTT Discovery' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')
  end

  before(:each) do
    mqtt_params = mqtt_parameters()
    @topic_prefix = mqtt_topic_prefix()
    @discovery_prefix = "#{@topic_prefix}/discovery"

    @client.put(
      '/settings',
      mqtt_params
    )

    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @discovery_suffix = "#{@id_params[:type]}_#{sprintf("%x", @id_params[:id])}_#{@id_params[:group_id]}"

    @mqtt_client = create_mqtt_client()
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

      @mqtt_client.on_message("#{@discovery_prefix}/light/+/#{@discovery_suffix}") do |topic, message|
        saw_message = true
      end

      @client.patch_settings(
        home_assistant_discovery_prefix: @discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      expect(saw_message).to be_true
    end

    it 'config should have expected keys' do
      saw_message = false
      config = nil

      @mqtt_client.on_message("#{@discovery_prefix}/light/+/#{@discovery_suffix}") do |topic, message|
        config = JSON.parse(message)
        saw_message = true
      end

      @client.patch_settings(
        home_assistant_discovery_prefix: @discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      expect(saw_message).to be_true
      expected_keys = %w(
        schema
        name
        command_topic
        state_topic
        availability_topic
        payload_available
        payload_not_available
        brightness
        rgb
        color_temp
        effect
        effect_list
      )
      expect(config.keys).to include(*expected_keys)
    end

    it 'should remove discoverable devices when alias is removed' do
      seen_config = false
      seen_blank_message = false

      @mqtt_client.on_message("#{@discovery_prefix}/light/+/#{@discovery_suffix}") do |topic, message|
        seen_config = message.length > 0
        seen_blank_message = message.empty?

        seen_config && seen_blank_message
      end

      # This should create the device
      @client.patch_settings(
        home_assistant_discovery_prefix: @discovery_prefix,
        group_id_aliases: {
          'test_group' => [@id_params[:type], @id_params[:id], @id_params[:group_id]]
        }
      )

      # This should clear it
      @client.patch_settings(
        group_id_aliases: { }
      )
    end
  end
end