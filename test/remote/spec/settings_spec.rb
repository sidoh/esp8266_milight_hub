require 'api_client'
require 'tempfile'
require 'net/ping'

RSpec.describe 'Settings' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

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
      ping_test = Net::Ping::External.new(static_ip)

      10.times do 
        break if ping_test.ping?
        sleep 1
      end

      expect(ping_test.ping?).to be(true)

      static_client = ApiClient.new(static_ip, ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
      static_client.put('/settings', wifi_static_ip: '')
      static_client.reboot

      ping_test = Net::Ping::External.new(ENV.fetch('ESPMH_HOSTNAME'))

      10.times do 
        break if ping_test.ping?
        sleep 1
      end

      expect(ping_test.ping?).to be(true)
    end
  end
end