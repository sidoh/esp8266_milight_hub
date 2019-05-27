require 'api_client'

RSpec.describe 'State' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    mqtt_params = mqtt_parameters()
    @updates_topic = mqtt_params[:updates_topic]
    @topic_prefix = mqtt_topic_prefix()

    @client.put(
      '/settings',
      mqtt_params
    )
  end

  before(:each) do
    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @client.delete_state(@id_params)

    @mqtt_client = create_mqtt_client()
  end

  context 'deleting' do
    it 'should remove retained state' do
      @client.patch_state(@id_params, status: 'ON')

      seen_blank = false

      @mqtt_client.on_state(@id_params) do |topic, message|
        seen_blank = (message == "")
      end

      @client.delete_state(@id_params)
      @mqtt_client.wait_for_listeners

      expect(seen_blank).to eq(true)
    end
  end

  context 'client status topic' do
    before(:all) do
      @status_topic = "#{@topic_prefix}client_status"
      @client.patch_settings(mqtt_client_status_topic: @status_topic)
    end

    it 'should send client status messages when configured' do
      # Clear any retained messages
      @mqtt_client.publish(@status_topic, nil)

      # Unfortunately, no way to easily simulate an unclean disconnect, so only test birth
      # and forced disconnect
      seen_statuses = Set.new
      required_statuses = %w(connected disconnected_clean)

      @mqtt_client.on_message(@status_topic, 20) do |topic, message|
        message = JSON.parse(message)

        seen_statuses << message['status']
        required_statuses.all? { |x| seen_statuses.include?(x) }
      end

      # Force MQTT reconnect by updating settings
      @client.put('/settings', fakekey: 'fakevalue')

      @mqtt_client.wait_for_listeners

      expect(seen_statuses).to include(*required_statuses)
    end

    it 'should send simple client status message when configured' do
      @client.patch_settings(simple_mqtt_client_status: true)

      # Clear any retained messages
      @mqtt_client.publish(@status_topic, nil)

      # Unfortunately, no way to easily simulate an unclean disconnect, so only test birth
      # and forced disconnect
      seen_statuses = Set.new
      required_statuses = %w(connected disconnected)

      @mqtt_client.on_message(@status_topic, 20) do |topic, message|
        seen_statuses << message
        required_statuses.all? { |x| seen_statuses.include?(x) }
      end

      # Force MQTT reconnect by updating settings
      @client.patch_settings(fakekey: 'fakevalue')

      @mqtt_client.wait_for_listeners

      expect(seen_statuses).to include(*required_statuses)
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

    it 'should respect the state update interval' do
      # Disable updates to prevent the negative effects of spamming commands
      @client.put(
        '/settings',
        mqtt_update_topic_pattern: '',
        mqtt_state_rate_limit: 500,
        packet_repeats: 1
      )

      # Set initial state
      @client.patch_state({status: 'ON', level: 0}, @id_params)

      last_seen = 0
      update_timestamp_gaps = []
      num_updates = 50

      @mqtt_client.on_state(@id_params) do |id, message|
        next_time = Time.now
        if last_seen != 0
          update_timestamp_gaps << next_time - last_seen
        end
        last_seen = next_time

        message['level'] == num_updates
      end

      (1..num_updates).each do |i|
        @mqtt_client.patch_state(@id_params, level: i)
        sleep 0.1
      end

      @mqtt_client.wait_for_listeners

      # Discard first, retained messages mess with it
      avg = update_timestamp_gaps.sum / update_timestamp_gaps.length

      expect(update_timestamp_gaps.length).to be >= 3
      expect((avg - 0.5).abs).to be < 0.02
    end
  end

  context ':device_id token for command topic' do
    it 'should support hexadecimal device IDs' do
      seen = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen = (message['status'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen).to eq(true), "Should see update for hex param"
    end

    it 'should support decimal device IDs' do
      seen = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen = (message['status'] == 'ON')
      end

      @mqtt_client.publish(
        "#{@topic_prefix}commands/#{@id_params[:id]}/rgb_cct/1",
        status: 'ON'
      )
      @mqtt_client.wait_for_listeners

      expect(seen).to eq(true), "Should see update for decimal param"
    end
  end

  context ':hex_device_id for command topic' do
    before(:all) do
      @client.put(
        '/settings',
        mqtt_topic_pattern: "#{@topic_prefix}commands/:hex_device_id/:device_type/:group_id",
      )
    end

    after(:all) do
      @client.put(
        '/settings',
        mqtt_topic_pattern: "#{@topic_prefix}commands/:device_id/:device_type/:group_id",
      )
    end

    it 'should respond to commands' do
      seen = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen = (message['status'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen).to eq(true), "Should see update for hex param"
    end
  end

  context ':dec_device_id for command topic' do
    before(:all) do
      @client.put(
        '/settings',
        mqtt_topic_pattern: "#{@topic_prefix}commands/:dec_device_id/:device_type/:group_id",
      )
    end

    after(:all) do
      @client.put(
        '/settings',
        mqtt_topic_pattern: "#{@topic_prefix}commands/:device_id/:device_type/:group_id",
      )
    end

    it 'should respond to commands' do
      seen = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen = (message['status'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen).to eq(true), "Should see update for hex param"
    end
  end

  context ':hex_device_id for update/state topics' do
    before(:all) do
      @client.put(
        '/settings',
        mqtt_state_topic_pattern: "#{@topic_prefix}state/:hex_device_id/:device_type/:group_id",
        mqtt_update_topic_pattern: "#{@topic_prefix}updates/:hex_device_id/:device_type/:group_id"
      )
    end

    after(:all) do
      @client.put(
        '/settings',
        mqtt_state_topic_pattern: "#{@topic_prefix}state/:device_id/:device_type/:group_id",
        mqtt_update_topic_pattern: "#{@topic_prefix}updates/:device_id/:device_type/:group_id"
      )
    end

    it 'should publish updates with hexadecimal device ID' do
      seen_update = false

      @mqtt_client.on_update(@id_params) do |id, message|
        seen_update = (message['state'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen_update).to eq(true)
    end

    it 'should publish state with hexadecimal device ID' do
      seen_state = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen_state = (message['status'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen_state).to eq(true)
    end
  end

  context ':dec_device_id for update/state topics' do
    before(:all) do
      @client.put(
        '/settings',
        mqtt_state_topic_pattern: "#{@topic_prefix}state/:dec_device_id/:device_type/:group_id",
        mqtt_update_topic_pattern: "#{@topic_prefix}updates/:dec_device_id/:device_type/:group_id"
      )
    end

    after(:all) do
      @client.put(
        '/settings',
        mqtt_state_topic_pattern: "#{@topic_prefix}state/:device_id/:device_type/:group_id",
        mqtt_update_topic_pattern: "#{@topic_prefix}updates/:device_id/:device_type/:group_id"
      )
    end

    it 'should publish updates with hexadecimal device ID' do
      seen_update = false
      @id_params = @id_params.merge(id_format: 'decimal')

      @mqtt_client.on_update(@id_params) do |id, message|
        seen_update = (message['state'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen_update).to eq(true)
    end

    it 'should publish state with hexadecimal device ID' do
      seen_state = false
      @id_params = @id_params.merge(id_format: 'decimal')

      @mqtt_client.on_state(@id_params) do |id, message|
        seen_state = (message['status'] == 'ON')
      end

      # Will use hex by default
      @mqtt_client.patch_state(@id_params, status: 'ON')
      @mqtt_client.wait_for_listeners

      expect(seen_state).to eq(true)
    end
  end
end