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

      @mqtt_client.on_update(id1) { |id, msg| id1_updates << msg }
      @mqtt_client.on_update(id2) { |id, msg| id2_updates << msg }

      @mqtt_client.wait_for_listeners

      expect(id1_updates.length).to eq(@num_transition_updates)
      expect(id2_updates.length).to eq(@num_transition_updates)
    end
  end
end