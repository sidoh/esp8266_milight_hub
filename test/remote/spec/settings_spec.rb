require 'api_client'
require 'tempfile'
require 'net/ping'

RSpec.describe 'Settings' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.reset_settings

    @username = 'a'
    @password = 'a'
  end

  after(:all) do
    @client.set_auth!(@username, @password)
    @client.put('/settings', admin_username: '', admin_password: '')
    @client.clear_auth!
  end

  before(:each) do
    @client.set_auth!(@username, @password)
    @client.put('/settings', admin_username: '', admin_password: '')
    @client.clear_auth!
  end

  after(:each) do
    puts "resetting settings..."
    @client.reset_settings
  end

  context 'keys' do
    it 'should persist known settings keys' do
      {
        'simple_mqtt_client_status' => [true, false],
        'mqtt_retain' => [true, false],
        'packet_repeats_per_loop' => [10],
        'home_assistant_discovery_prefix' => ['', 'abc', 'a/b/c'],
        'wifi_mode' => %w(b g n),
        'default_transition_period' => [200, 500]
      }.each do |key, values|
        values.each do |v|
          @client.patch_settings({key => v})
          expect(@client.get('/settings')[key]).to eq(v), "Should persist #{key} possible value: #{v}"
        end
      end
    end
  end

  context 'POST settings file' do
    it 'should clobber patched settings' do
      file = Tempfile.new('espmh-settings.json')
      file.write({
        mqtt_server: 'test123'
      }.to_json)
      file.close

      @client.upload_json('/settings', file.path)

      settings = @client.get('/settings')
      expect(settings['mqtt_server']).to eq('test123')

      @client.put('/settings', {mqtt_server: 'abc123', mqtt_username: 'foo'})

      settings = @client.get('/settings')
      expect(settings['mqtt_server']).to eq('abc123')
      expect(settings['mqtt_username']).to eq('foo')

      @client.upload_json('/settings', file.path)
      settings = @client.get('/settings')

      expect(settings['mqtt_server']).to eq('test123')
      expect(settings['mqtt_username']).to eq('')

      File.delete(file.path)
    end

    it 'should apply POSTed settings' do
      file = Tempfile.new('espmh-settings.json')
      file.write({
        admin_username: @username,
        admin_password: @password
      }.to_json)
      file.close

      @client.upload_json('/settings', file.path)

      expect { @client.get('/settings') }.to raise_error(Net::HTTPServerException)

      @client.set_auth!(@username, @password)
      @client.reset_settings
    end
  end

  context 'PUT settings file' do
    it 'should accept a fairly large request body' do
      contents = (1..25).reduce({}) { |a, x| a[x] = "test#{x}"*10; a }

      expect { @client.put('/settings', contents) }.to_not raise_error
    end

    it 'should not cause excessive memory leaks' do
      start_mem = @client.get('/about')['free_heap']

      20.times do
        @client.put('/settings', mqtt_username: 'a')
      end

      end_mem = @client.get('/about')['free_heap']

      expect(end_mem).to be_within(1024).of(start_mem)
    end
  end

  context 'radio' do
    it 'should store a set of channels' do
      val = %w(HIGH LOW)
      @client.put('/settings', rf24_channels: val)
      result = @client.get('/settings')
      expect(result['rf24_channels']).to eq(val)

      val = %w(MID LOW)
      @client.put('/settings', rf24_channels: val)
      result = @client.get('/settings')
      expect(result['rf24_channels']).to eq(val)

      val = %w(MID LOW LOW LOW)
      @client.put('/settings', rf24_channels: val)
      result = @client.get('/settings')
      expect(result['rf24_channels']).to eq(Set.new(val).to_a)
    end

    it 'should store a listen channel' do
      @client.put('/settings', rf24_listen_channel: 'MID')
      result = @client.get('/settings')
      expect(result['rf24_listen_channel']).to eq('MID')

      @client.put('/settings', rf24_listen_channel: 'LOW')
      result = @client.get('/settings')
      expect(result['rf24_listen_channel']).to eq('LOW')
    end
  end

  context 'group id labels' do
    it 'should store ID labels' do
      id = 1

      aliases = StateHelpers::ALL_REMOTE_TYPES.each_with_index.map do |remote_type, i|
        [i, "test_#{id += 1}", remote_type, id, 1]
      end
      alias_csv = aliases.map { |x| x.join(",") }.join("\n") + "\n"

      Tempfile.create('aliases.txt') do |file|
        file.write(alias_csv)
        file.close

        @client.upload_json('/aliases.txt', file.path)
      end

      expect @client.get('/aliases.txt') == alias_csv

      response = @client.get('/aliases')
      stored_aliases = response['aliases'].map { |x| x['alias'] }

      expect(Set.new(stored_aliases)).to eq(Set.new(aliases.map { |x| x[1] }))
    end
  end

  context 'static ip' do
    it 'should boot with static IP when applied' do
      static_ip = ENV.fetch('ESPMH_STATIC_IP')

      @client.put(
        '/settings',
        wifi_static_ip: static_ip,
        wifi_static_ip_netmask: ENV.fetch('ESPMH_STATIC_IP_NETMASK'),
        wifi_static_ip_gateway: ENV.fetch('ESPMH_STATIC_IP_GATEWAY')
      )

      # Reboot to apply static ip
      @client.reboot

      # Wait for it to come back up
      static_client = ApiClient.new(static_ip, ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
      static_client.wait_for_liveness
      static_client.put('/settings', wifi_static_ip: '')
      static_client.reboot

      @client.wait_for_liveness
    end
  end

  context 'defaults' do
    before(:all) do
      # Clobber all settings
      file = Tempfile.new('espmh-settings.json')
      file.close

      @client.reset_settings(file.path)
    end

    after(:all) do
      @client.reset_settings
    end

    it 'should have some group state fields defined' do
      settings = @client.get('/settings')

      expect(settings['group_state_fields']).to_not be_empty
    end

    it 'should allow for empty group state fields if set' do
      @client.patch_settings(group_state_fields: [])
      settings = @client.get('/settings')

      expect(settings['group_state_fields']).to eq([])
    end

    it 'for enable_automatic_mode_switching, default should be false' do
      settings = @client.get('/settings')

      expect(settings['enable_automatic_mode_switching']).to eq(false)
    end
  end
end