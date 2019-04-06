require 'api_client'
require './helpers/state_helpers'

RSpec.configure do |c|
  c.include StateHelpers
end

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
end