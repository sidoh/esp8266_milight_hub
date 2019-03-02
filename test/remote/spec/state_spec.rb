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

      expect(patched_state.keys).to include(*desired_state.keys)
      expect(patched_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)

      desired_state = {
        'status' => 'ON',
        'level' => 10,
        'hue' => 49,
        'saturation' => 20
      }
      @client.patch_state(desired_state, @id_params)
      patched_state = @client.get_state(@id_params)

      expect(patched_state.keys).to include(*desired_state.keys)
      expect(patched_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)
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
      expect(patched_state.keys).to include(*individual_state.keys)
      expect(patched_state.select { |x| individual_state.include?(x) } ).to eq(individual_state)

      group_4_state = @client.get_state(group_0_params.merge(group_id: 4))

      expect(group_4_state.keys).to include(*desired_state.keys)
      expect(group_4_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)

      @client.patch_state(desired_state, group_0_params)
      group_1_state = @client.get_state(group_0_params.merge(group_id: 1))

      expect(group_1_state.keys).to include(*desired_state.keys)
      expect(group_1_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)
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

      expect(patched_state.keys).to include(*desired_state.keys)
      expect(patched_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)
    end
  end
end