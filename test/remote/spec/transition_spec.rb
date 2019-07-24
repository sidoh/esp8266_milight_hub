require 'api_client'

RSpec.describe 'Transitions' do
  before(:all) do
    @client = ApiClient.from_environment
    @client.upload_json('/settings', 'settings.json')
    @transition_params = {
      field: 'level',
      start_value: 0,
      end_value: 100,
      duration: 2.0,
      period: 400
    }
    @num_transition_updates = (@transition_params[:duration]*1000)/@transition_params[:period]
  end

  before(:each) do
    mqtt_params = mqtt_parameters()
    @updates_topic = mqtt_params[:updates_topic]
    @topic_prefix = mqtt_topic_prefix()

    @client.put(
      '/settings',
      mqtt_params.merge(
        mqtt_update_topic_pattern: "#{@topic_prefix}updates/:device_id/:device_type/:group_id"
      )
    )

    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @client.delete_state(@id_params)

    @mqtt_client = create_mqtt_client()

    # Delete any existing transitions
    @client.get('/transitions')['transitions'].each do |t|
      @client.delete("/transitions/#{t['id']}")
    end
  end

  context 'REST routes' do
    it 'should respond with an empty list when there are no transitions' do
      response = @client.transitions
      expect(response).to eq([])
    end

    it 'should respond with an error when missing parameters for POST /transitions' do
      expect { @client.post('/transitions', {}) }.to raise_error(Net::HTTPServerException)
    end

    it 'should create a new transition with a valid POST /transitions request' do
      response = @client.schedule_transition(@id_params, @transition_params)

      expect(response['success']).to eq(true)
    end

    it 'should list active transitions' do
      @client.schedule_transition(@id_params, @transition_params)

      response = @client.transitions

      expect(response.length).to be >= 1
    end

    it 'should support getting an active transition with GET /transitions/:id' do
      @client.schedule_transition(@id_params, @transition_params)

      response = @client.transitions
      detail_response = @client.get("/transitions/#{response.last['id']}")

      expect(detail_response['period']).to_not eq(nil)
    end

    it 'should support deleting active transitions with DELETE /transitions/:id' do
      @client.schedule_transition(@id_params, @transition_params)

      response = @client.transitions

      response.each do |transition|
        @client.delete("/transitions/#{transition['id']}")
      end

      after_delete_response = @client.transitions

      expect(response.length).to eq(1)
      expect(after_delete_response.length).to eq(0)
    end
  end

  context '"transition" key in state update' do
    it 'should create a new transition' do
      @client.patch_state({status: 'ON', level: 0}, @id_params)
      @client.patch_state({level: 100, transition: 2.0}, @id_params)

      response = @client.transitions

      expect(response.length).to be > 0
      expect(response.last['type']).to eq('field')
      expect(response.last['field']).to eq('level')
      expect(response.last['end_value']).to eq(100)

      @client.delete("/transitions/#{response.last['id']}")
    end

    it 'should transition field' do
      seen_updates = 0
      last_value = nil

      @client.patch_state({status: 'ON', level: 0}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, msg|
        if msg.include?('brightness')
          seen_updates += 1
          last_value = msg['brightness']
        end

        last_value == 255
      end

      @client.patch_state({level: 100, transition: 2.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(last_value).to eq(255)
      expect(seen_updates).to eq(8) # duration of 2000ms / 300ms period + 1 for initial packet
    end

    it 'should transition a field downwards' do
      seen_updates = 0
      last_value = nil

      @client.patch_state({status: 'ON'}, @id_params)
      @client.patch_state({level: 100}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, msg|
        if msg.include?('brightness')
          seen_updates += 1
          last_value = msg['brightness']
        end

        last_value == 0
      end

      @client.patch_state({level: 0, transition: 2.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(last_value).to eq(0)
      expect(seen_updates).to eq(8) # duration of 2000ms / 300ms period + 1 for initial packet
    end

    it 'should transition two fields at once if received in the same command' do
      updates = {}

      @client.patch_state({status: 'ON', hue: 0, level: 100}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, msg|
        msg.each do |k, v|
          updates[k] ||= []
          updates[k] << v
        end

        updates['hue'] && updates['brightness'] && updates['hue'].last == 250 && updates['brightness'].last == 0
      end

      @client.patch_state({level: 0, hue: 250, transition: 2.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(updates['hue'].last).to eq(250)
      expect(updates['brightness'].last).to eq(0)
      expect(updates['hue'].length == updates['brightness'].length).to eq(true), "Should have the same number of updates for both fields"
      expect(updates['hue'].length).to eq(8)
    end
  end

  context 'transition packets' do
    it 'should send an initial state packet' do
      seen = false

      @mqtt_client.on_update(@id_params) do |id, message|
        seen = message['brightness'] == 0
      end

      @client.schedule_transition(@id_params, @transition_params)

      @mqtt_client.wait_for_listeners

      expect(seen).to be(true)
    end

    it 'should respect the period parameter' do
      seen_updates = []
      start_time = Time.now

      @mqtt_client.on_update(@id_params) do |id, message|
        seen_updates << message
        message['brightness'] == 255
      end

      @client.schedule_transition(@id_params, @transition_params.merge(duration: 2.0, period: 500))

      @mqtt_client.wait_for_listeners

      expect(seen_updates.map { |x| x['brightness'] }).to eq([0, 64, 128, 191, 255])
      expect((Time.now - start_time)/4).to be >= 0.5 # Don't count the first update
    end

    it 'should support two transitions for different devices at the same time' do
      id1 = @id_params
      id2 = @id_params.merge(type: 'fut089')

      @client.schedule_transition(id1, @transition_params)
      @client.schedule_transition(id2, @transition_params)

      id1_updates = []
      id2_updates = []

      @mqtt_client.on_update do |id, msg|
        if id[:type] == id1[:type]
          id1_updates << msg
        else
          id2_updates << msg
        end
        id1_updates.length == @num_transition_updates && id2_updates.length == @num_transition_updates
      end

      @mqtt_client.wait_for_listeners

      expect(id1_updates.length).to eq(@num_transition_updates)
      expect(id2_updates.length).to eq(@num_transition_updates)
    end

    it 'should assume initial state if one is not provided' do
      @client.patch_state({status: 'ON', level: 0}, @id_params)

      seen_updates = []

      @mqtt_client.on_update(@id_params) do |id, message|
        seen_updates << message
        message['brightness'] == 255
      end

      @client.schedule_transition(@id_params, @transition_params.reject { |x| x == :start_value }.merge(duration: 2, period: 500))

      @mqtt_client.wait_for_listeners

      expect(seen_updates.map { |x| x['brightness'] }).to eq([0, 64, 128, 191, 255])
    end
  end

  context 'status transition' do
    it 'should transition from off -> on' do
      seen_updates = {}
      @client.patch_state({status: 'OFF'}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, message|
        message.each do |k, v|
          seen_updates[k] ||= []
          seen_updates[k] << v
        end
        seen_updates['brightness'] && seen_updates['brightness'].last == 255
      end

      @client.patch_state({status: 'ON', transition: 1.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(seen_updates['state']).to eq(['ON'])
      expect(seen_updates['brightness']).to eq([0, 64, 128, 191, 255])
    end

    it 'should transition from on -> off' do
      seen_updates = {}
      @client.patch_state({status: 'ON', level: 100}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, message|
        message.each do |k, v|
          seen_updates[k] ||= []
          seen_updates[k] << v
        end
        seen_updates['state'] == ['OFF']
      end

      @client.patch_state({status: 'OFF', transition: 1.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(seen_updates['state']).to eq(['OFF'])
      expect(seen_updates['brightness']).to eq([255, 191, 128, 64, 0])
    end

    it 'should transition from off -> on with known last brightness' do
      seen_updates = {}
      @client.patch_state({status: 'ON', brightness: 99}, @id_params)
      @client.patch_state({status: 'OFF'}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, message|
        message.each do |k, v|
          seen_updates[k] ||= []
          seen_updates[k] << v
        end
        seen_updates['brightness'] && seen_updates['brightness'].last == 255
      end

      @client.patch_state({status: 'ON', transition: 1.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(seen_updates['brightness']).to eq([99, 140, 181, 222, 255])
    end

    it 'should transition from on -> off with known last brightness' do
      seen_updates = {}
      @client.patch_state({status: 'ON', brightness: 99}, @id_params)

      @mqtt_client.on_update(@id_params) do |id, message|
        message.each do |k, v|
          seen_updates[k] ||= []
          seen_updates[k] << v
        end
        seen_updates['state'] == ['OFF']
      end

      @client.patch_state({status: 'OFF', transition: 1.0}, @id_params)

      @mqtt_client.wait_for_listeners

      expect(seen_updates['brightness']).to eq([99, 74, 48, 23, 0])
    end
  end

  context 'field support' do
    {
      'level' => {range: [0, 100], update_field: 'brightness', update_max: 255},
      'brightness' => {range: [0, 255]},
      'kelvin' => {range: [0, 100], update_field: 'color_temp', update_min: 153, update_max: 370},
      'color_temp' => {range: [153, 370]},
      'hue' => {range: [0, 359]},
      'saturation' => {range: [0, 100]}
    }.each do |field, params|
      min, max = params[:range]
      update_min = params[:update_min] || min
      update_max = params[:update_max] || max
      update_field = params[:update_field] || field

      it "should support field '#{field}' min --> max" do
        seen_updates = []

        @client.patch_state({'status' => 'ON', field => min}, @id_params)

        @mqtt_client.on_update(@id_params) do |id, message|
          seen_updates << message
          message[update_field] == update_max
        end

        @client.patch_state({field => max, 'transition' => 1.0}, @id_params)

        @mqtt_client.wait_for_listeners

        expect(seen_updates.length).to eq(5)
        expect(seen_updates.last[update_field]).to eq(update_max)
      end

      it "should support field '#{field}' max --> min" do
        seen_updates = []

        @client.patch_state({'status' => 'ON', field => max}, @id_params)

        @mqtt_client.on_update(@id_params) do |id, message|
          seen_updates << message
          message[update_field] == update_min
        end

        @client.patch_state({field => min, 'transition' => 1.0}, @id_params)

        @mqtt_client.wait_for_listeners

        expect(seen_updates.length).to eq(5)
        expect(seen_updates.last[update_field]).to eq(update_min)
      end
    end
  end

  context 'color support' do
    it 'should support color transitions' do
      response = @client.schedule_transition(@id_params, {
        field: 'color',
        start_value: '255,0,0',
        end_value: '0,255,0',
        duration: 1.0,
        period: 500
      })
      expect(response['success']).to eq(true)
    end

    it 'should smoothly transition from one color to another' do
      seen_updates = []

      fields = @client.get('/settings')['group_state_fields']
      @client.put(
        '/settings',
        group_state_fields: fields + %w(oh_color),
        mqtt_state_rate_limit: 1000
      )

      @mqtt_client.on_state(@id_params) do |id, message|
        color = message['color']
        seen_updates << color
        color == '0,255,0'
      end

      response = @client.schedule_transition(@id_params, {
        field: 'color',
        start_value: '255,0,0',
        end_value: '0,255,0',
        duration: 4.0,
        period: 1000
      })

      @mqtt_client.wait_for_listeners

      parts = seen_updates.map { |x| x.split(',').map(&:to_i) }

      # This is less even than you'd expect because RGB -> Hue/Sat is lossy.
      # Raw logs show that the right thing is happening:
      #
      #     >>> stepSizes = (-64,64,0)
      #     >>> start = (255,0,0)
      #     >>> end = (0,255,0)
      #     >>> current color = (191,64,0)
      #     >>> current color = (127,128,0)
      #     >>> current color = (63,192,0)
      #     >>> current color = (0,255,0)
      expect(parts).to eq([
        [255, 0, 0],
        [255, 84, 0],
        [250, 255, 0],
        [84, 255, 0],
        [0, 255, 0]
      ])
    end

    it 'should handle color transitions from known state' do
      seen_updates = []

      fields = @client.get('/settings')['group_state_fields']
      @client.put(
        '/settings',
        group_state_fields: fields + %w(oh_color),
        mqtt_state_rate_limit: 1000
      )
      @client.patch_state({status: 'ON', color: '255,0,0'}, @id_params)

      @mqtt_client.on_state(@id_params) do |id, message|
        color = message['color']
        seen_updates << color if color
        color == '0,0,255'
      end

      @client.patch_state({color: '0,0,255', transition: 2.0}, @id_params)
      @mqtt_client.wait_for_listeners

      parts = seen_updates.map { |x| x.split(',').map(&:to_i) }

      expect(parts).to eq([
        [255,0,0],
        [161,0,255],
        [0,0,255]
      ])
    end
  end
end