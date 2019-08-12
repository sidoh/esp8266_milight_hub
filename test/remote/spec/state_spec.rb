require 'api_client'

RSpec.describe 'State' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')
  end

  before(:each) do
    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @client.delete_state(@id_params)
  end

  context 'blockOnQueue parameter' do
    it 'should not receive state if we don\'t block on the packet queue' do
      response = @client.patch_state({status: 'ON'}, @id_params.merge(blockOnQueue: false))

      expect(response).to eq({'success' => true})
    end

    it 'should receive state if we do block on the packet queue' do
      response = @client.patch_state({status: 'ON'}, @id_params.merge(blockOnQueue: true))

      expect(response).to eq({'status' => 'ON'})
    end
  end

  context 'initial state' do
    it 'should assume white mode for device types that are white-only' do
      %w(cct fut091).each do |type|
        id = @id_params.merge(type: type)
        @client.delete_state(id)
        state = @client.patch_state({status: 'ON'}, id)
        expect(state['bulb_mode']).to eq('white'), "it should assume white mode for #{type}"
      end
    end

    it 'should assume color mode for device types that are rgb-only' do
      %w(rgb).each do |type|
        id = @id_params.merge(type: type)
        @client.delete_state(id)
        state = @client.patch_state({status: 'ON'}, id)
        expect(state['bulb_mode']).to eq('color'), "it should assume color mode for #{type}"
      end
    end
  end

  context 'toggle command' do
    it 'should toggle ON to OFF' do
      init_state = @client.patch_state({'status' => 'ON'}, @id_params)
      expect(init_state['status']).to eq('ON')

      next_state = @client.patch_state({'command' => 'toggle'}, @id_params)
      expect(next_state['status']).to eq('OFF')
    end

    it 'should toggle OFF to ON' do
      init_state = @client.patch_state({'status' => 'OFF'}, @id_params)
      expect(init_state['status']).to eq('OFF')

      next_state = @client.patch_state({'command' => 'toggle'}, @id_params)
      expect(next_state['status']).to eq('ON')
    end
  end

  context 'night mode command' do
    StateHelpers::ALL_REMOTE_TYPES
      .reject { |x| %w(rgb).include?(x) } # Night mode not supported for these types
      .each do |type|
      it "should affect state when bulb is OFF for #{type}" do
        params = @id_params.merge(type: type)
        @client.delete_state(params)
        state = @client.patch_state({'command' => 'night_mode'}, params)

        expect(state['bulb_mode']).to eq('night')
        expect(state['effect']).to    eq('night_mode')
      end
    end

    StateHelpers::ALL_REMOTE_TYPES
      .reject { |x| %w(rgb).include?(x) } # Night mode not supported for these types
      .each do |type|
      it "should affect state when bulb is ON for #{type}" do
        params = @id_params.merge(type: type)
        @client.delete_state(params)
        @client.patch_state({'status' => 'ON'}, params)
        state = @client.patch_state({'command' => 'night_mode'}, params)

        # RGBW bulbs have to be OFF in order for night mode to take affect
        expect(state['status']).to    eq('ON') if type != 'rgbw'
        expect(state['bulb_mode']).to eq('night')
        expect(state['effect']).to    eq('night_mode')
      end
    end

    it 'should revert to previous mode when status is toggled' do
      @client.patch_state({'status' => 'ON', 'kelvin' => 100}, @id_params)
      state = @client.patch_state({'command' => 'night_mode'}, @id_params)

      expect(state['effect']).to eq('night_mode')

      state = @client.patch_state({'status' => 'OFF'}, @id_params)

      expect(state['bulb_mode']).to eq('white')
      expect(state['kelvin']).to    eq(100)

      @client.patch_state({'status' => 'ON', 'hue' => 0}, @id_params)
      state = @client.patch_state({'command' => 'night_mode'}, @id_params)

      expect(state['effect']).to eq('night_mode')

      state = @client.patch_state({'status' => 'OFF'}, @id_params)

      expect(state['bulb_mode']).to eq('color')
      expect(state['hue']).to       eq(0)
    end
  end

  context 'deleting' do
    it 'should support deleting state' do
      desired_state = {
        'status' => 'ON',
        'level' => 10,
        'hue' => 49,
        'saturation' => 20
      }
      @client.patch_state(desired_state, @id_params)

      resulting_state = @client.get_state(@id_params)
      expect(resulting_state).to_not be_empty

      @client.delete_state(@id_params)
      resulting_state = @client.get_state(@id_params)
      expect(resulting_state).to be_empty
    end
  end

  context 'persistence' do
    it 'should persist parameters' do
      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'hue' => 0,
        'saturation' => 100
      }
      @client.patch_state(desired_state, @id_params)
      patched_state = @client.get_state(@id_params)

      states_are_equal(desired_state, patched_state)

      desired_state = {
        'status' => 'ON',
        'level' => 10,
        'hue' => 49,
        'saturation' => 20
      }
      @client.patch_state(desired_state, @id_params)
      patched_state = @client.get_state(@id_params)

      states_are_equal(desired_state, patched_state)
    end

    it 'should affect member groups when changing group 0' do
      group_0_params = @id_params.merge(group_id: 0)

      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'hue' => 0,
        'saturation' => 100
      }

      @client.patch_state(desired_state, group_0_params)

      individual_state = desired_state.merge('level' => 10)
      patched_state = @client.patch_state(individual_state, @id_params)

      expect(patched_state).to_not eq(desired_state)
      states_are_equal(individual_state, patched_state)

      group_4_state = @client.get_state(group_0_params.merge(group_id: 4))

      states_are_equal(desired_state, group_4_state)

      @client.patch_state(desired_state, group_0_params)
      group_1_state = @client.get_state(group_0_params.merge(group_id: 1))

      states_are_equal(desired_state, group_1_state)
    end

    it 'should keep group 0 state' do
      group_0_params = @id_params.merge(group_id: 0)
      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'hue' => 0,
        'saturation' => 100
      }

      patched_state = @client.patch_state(desired_state, group_0_params)

      states_are_equal(desired_state, patched_state)
    end

    it 'should clear group 0 state after member group state changes' do
      group_0_params = @id_params.merge(group_id: 0)
      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'kelvin' => 100
      }

      @client.patch_state(desired_state, group_0_params)
      @client.patch_state(desired_state.merge('kelvin' => 10), @id_params)

      resulting_state = @client.get_state(group_0_params)

      expect(resulting_state.keys).to_not include('kelvin')
      states_are_equal(desired_state.reject { |x| x == 'kelvin' }, resulting_state)
    end

    it 'should not clear group 0 state when updating member group state if value is the same' do
      group_0_params = @id_params.merge(group_id: 0)
      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'kelvin' => 100
      }

      @client.patch_state(desired_state, group_0_params)
      @client.patch_state(desired_state.merge('kelvin' => 100), @id_params)

      resulting_state = @client.get_state(group_0_params)

      expect(resulting_state).to include('kelvin')
      states_are_equal(desired_state, resulting_state)
    end

    it 'changing member state mode and then changing level should preserve group 0 brightness for original mode' do
      group_0_params = @id_params.merge(group_id: 0)
      desired_state = {
        'status' => 'ON',
        'level' => 100,
        'hue' => 0,
        'saturation' => 100
      }

      @client.delete_state(group_0_params)
      @client.patch_state(desired_state, group_0_params)

      # color -> white mode.  should not have brightness because brightness will
      # have been previously unknown to group 0.
      @client.patch_state(desired_state.merge('color_temp' => 253, 'level' => 11), @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state.keys).to_not include('level')

      # color -> effect mode.  same as above
      @client.patch_state(desired_state, group_0_params)
      @client.patch_state(desired_state.merge('mode' => 0), @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state).to_not include('level')

      # white mode -> color.
      white_mode_desired_state = {'status' => 'ON', 'color_temp' => 253, 'level' => 11}
      @client.patch_state(white_mode_desired_state, group_0_params)
      @client.patch_state({'hue' => 10}, @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state).to_not include('level')

      @client.patch_state({'hue' => 10}, group_0_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state['level']).to eq(100)

      # white mode -> effect mode.  level never set for group 0, so level should
      # level should be present.
      @client.patch_state(white_mode_desired_state, group_0_params)
      @client.patch_state({'mode' => 0}, @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state).to_not include('level')

      # effect mode -> color.  same as white mode -> color
      effect_mode_desired_state = {'status' => 'ON', 'mode' => 0, 'level' => 100}
      @client.patch_state(effect_mode_desired_state, group_0_params)
      @client.patch_state({'hue' => 10}, @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state).to_not include('level')

      # effect mode -> white
      @client.patch_state(effect_mode_desired_state, group_0_params)
      @client.patch_state({'color_temp' => 253}, @id_params)
      resulting_state = @client.get_state(group_0_params)
      expect(resulting_state).to_not include('level')
    end
  end

  context 'fields' do
    it 'should support on/off' do
      @client.patch_state({status: 'on'}, @id_params)
      expect(@client.get_state(@id_params)['status']).to eq('ON')

      # test "state", which is an alias for "status"
      @client.patch_state({state: 'off'}, @id_params)
      expect(@client.get_state(@id_params)['status']).to eq('OFF')
    end

    it 'should support boolean values for status' do
      # test boolean value "true", which should be the same as "ON".
      @client.patch_state({status: true}, @id_params)
      expect(@client.get_state(@id_params)['status']).to eq('ON')

      @client.patch_state({state: false}, @id_params)
      expect(@client.get_state(@id_params)['status']).to eq('OFF')
    end

    it 'should support the color field' do
      desired_state = {
        'hue' => 0,
        'saturation' => 100,
        'status' => 'ON'
      }

      @client.patch_state(
        desired_state.merge(hue: 100),
        @id_params
      )

      @client.patch_state(
        { color: '255,0,0' },
        @id_params
      )

      state = @client.get_state(@id_params)

      expect(state.keys).to include(*desired_state.keys)
      expect(state.select { |x| desired_state.include?(x) } ).to eq(desired_state)

      @client.patch_state(
        { color: {r: 0, g: 255, b: 0} },
        @id_params
      )
      state = @client.get_state(@id_params)

      desired_state.merge!('hue' => 120)

      expect(state.keys).to include(*desired_state.keys)
      expect(state.select { |x| desired_state.include?(x) } ).to eq(desired_state)
    end

    it 'should support hex colors' do
      {
        'FF0000': 0,
        '00FF00': 120,
        '0000FF': 240
      }.each do |hex_color, hue|
        state = @client.patch_state({status: 'ON', color: "##{hex_color}"}, @id_params)
        expect(state['hue']).to eq(hue), "Hex color #{hex_color} should map to hue = #{hue}, but was #{state['hue'].inspect}"
      end
    end

    it 'should support getting color in hex format' do
      fields = @client.get('/settings')['group_state_fields']
      @client.patch_settings({group_state_fields: fields + ['hex_color']})
      state = @client.patch_state({status: 'ON', color: '#FF0000'}, @id_params)
      expect(state['color']).to eq('#FF0000')
    end

    it 'should support getting color in comma-separated format' do
      fields = @client.get('/settings')['group_state_fields']
      @client.patch_settings({group_state_fields: fields+['oh_color']})
      state = @client.patch_state({status: 'ON', color: '#FF0000'}, @id_params)
      expect(state['color']).to eq('255,0,0')
    end

    it 'should support separate brightness fields for different modes' do
      desired_state = {
        'hue' => 0,
        'level' => 50
      }

      @client.patch_state(desired_state, @id_params)
      result = @client.get_state(@id_params)
      expect(result['bulb_mode']).to eq('color')
      expect(result['level']).to eq(50)


      @client.patch_state({'kelvin' => 100}, @id_params)
      @client.patch_state({'level' => 70}, @id_params)
      result = @client.get_state(@id_params)
      expect(result['bulb_mode']).to eq('white')
      expect(result['level']).to eq(70)

      @client.patch_state({'hue' => 0}, @id_params)
      result = @client.get_state(@id_params)
      expect(result['bulb_mode']).to eq('color')
      # Should retain previous brightness
      expect(result['level']).to eq(50)
    end

    it 'should support the mode and effect fields' do
      state = @client.patch_state({status: 'ON', mode: 0}, @id_params)
      expect(state['effect']).to eq("0")

      state = @client.patch_state({effect: 1}, @id_params)
      expect(state['effect']).to eq("1")
    end
  end

  context 'increment/decrement commands' do
    it 'should assume state after sufficiently many down commands' do
      id = @id_params.merge(type: 'cct')
      @client.delete_state(id)

      @client.patch_state({status: 'on'}, id)

      expect(@client.get_state(id)).to_not include('brightness', 'kelvin')

      10.times do
        @client.patch_state(
          { commands: ['level_down', 'temperature_down'] },
          id
        )
      end

      state = @client.get_state(id)
      expect(state).to          include('level', 'kelvin')
      expect(state['level']).to eq(0)
      expect(state['kelvin']).to eq(0)
    end

    it 'should assume state after sufficiently many up commands' do
      id = @id_params.merge(type: 'cct')
      @client.delete_state(id)

      @client.patch_state({status: 'on'}, id)

      expect(@client.get_state(id)).to_not include('level', 'kelvin')

      10.times do
        @client.patch_state(
          { commands: ['level_up', 'temperature_up'] },
          id
        )
      end

      state = @client.get_state(id)
      expect(state).to          include('level', 'kelvin')
      expect(state['level']).to eq(100)
      expect(state['kelvin']).to eq(100)
    end

    it 'should affect known state' do
      id = @id_params.merge(type: 'cct')
      @client.delete_state(id)

      @client.patch_state({status: 'on'}, id)

      expect(@client.get_state(id)).to_not include('level', 'kelvin')

      10.times do
        @client.patch_state(
          { commands: ['level_up', 'temperature_up'] },
          id
        )
      end

      @client.patch_state(
        { commands: ['level_down', 'temperature_down'] },
        id
      )

      state = @client.get_state(id)
      expect(state).to           include('level', 'kelvin')
      expect(state['level']).to  eq(90)
      expect(state['kelvin']).to eq(90)
    end
  end

  context 'state updates while off' do
    it 'should not affect persisted state' do
      @client.patch_state({'status' => 'OFF'}, @id_params)
      state = @client.patch_state({'hue' => 100}, @id_params)

      expect(state.count).to eq(1)
      expect(state).to include('status')
    end

    it 'should not affect persisted state using increment/decrement' do
      @client.patch_state({'status' => 'OFF'}, @id_params)

      10.times do
        @client.patch_state(
          { commands: ['level_down', 'temperature_down'] },
          @id_params
        )
      end

      state = @client.get_state(@id_params)

      expect(state.count).to eq(1)
      expect(state).to include('status')
    end
  end

  context 'fut089' do
    # FUT089 uses the same command ID for both kelvin and saturation command, so
    # interpreting such a command depends on knowledge of the state that the bulb
    # is in.
    it 'should keep enough group 0 state to interpret ambiguous kelvin/saturation commands as saturation commands when in color mode' do
      group0_params = @id_params.merge(type: 'fut089', group_id: 0)

      (0..8).each do |group_id|
        @client.delete_state(group0_params.merge(group_id: group_id))
      end

      # Patch in separate commands so state must be kept
      @client.patch_state({'status' => 'ON', 'hue' => 0}, group0_params)
      @client.patch_state({'saturation' => 100}, group0_params)

      (0..8).each do |group_id|
        state = @client.get_state(group0_params.merge(group_id: group_id))
        expect(state['bulb_mode']).to eq('color')
        expect(state['saturation']).to eq(100)
        expect(state['hue']).to eq(0)
      end
    end
  end

  context 'fut020' do
    it 'should support fut020 commands' do
      id = @id_params.merge(type: 'fut020', group_id: 0)
      @client.delete_state(id)
      state = @client.patch_state({status: 'ON'}, id)

      expect(state['status']).to eq('ON')
    end

    it 'should assume the "off" command sets state to on... commands are the same' do
      id = @id_params.merge(type: 'fut020', group_id: 0)
      @client.delete_state(id)
      state = @client.patch_state({status: 'OFF'}, id)

      expect(state['status']).to eq('ON')
    end
  end
end