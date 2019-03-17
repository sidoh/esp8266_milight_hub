require 'api_client'

RSpec.describe 'Settings' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')
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
end