require 'api_client'

RSpec.describe 'Environment' do
  before(:each) do
    @host = ENV.fetch('ESPMH_HOSTNAME')
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
  end

  context 'environment' do
    it 'should have a host defined' do
      expect(@host).to_not be_nil
    end

    it 'should respond to /about' do
      response = @client.get('/about')

      expect(response).to_not  be_nil
      expect(response.keys).to include('version')
    end
  end

  context 'client' do
    it 'should return IDs' do
      id = @client.generate_id

      expect(@client.generate_id).to equal(id + 1)
    end
  end

  it 'needs to have a settings.json file' do
    expect(File.exists?('settings.json')).to be(true)
  end
end