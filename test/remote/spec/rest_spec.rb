require 'api_client'

RSpec.describe 'REST Server' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    @username = 'a'
    @password = 'a'
  end

  context 'authentication' do
    after(:all) do
      @client.set_auth!(@username, @password)
      @client.put('/settings', admin_username: '', admin_password: '')
    end

    it 'should not require auth unless both username and password are set' do
      @client.put('/settings', admin_username: 'abc', admin_password: '')
      expect { @client.get('/settings') }.not_to raise_error

      @client.put('/settings', admin_username: '', admin_password: 'abc')
      expect { @client.get('/settings') }.not_to raise_error

      @client.put('/settings', admin_username: '', admin_password: '')
      expect { @client.get('/settings') }.not_to raise_error
    end

    it 'should require auth for all routes when password is set' do
      @client.put('/settings', admin_username: @username, admin_password: @password)

      # Try no auth
      expect { @client.get('/settings') }.to raise_error(Net::HTTPServerException)

      # Try wrong username
      @client.set_auth!("#{@username}wronguser", @password)
      expect { @client.get('/settings') }.to raise_error(Net::HTTPServerException)

      # Try wrong password
      @client.set_auth!(@username, "wrong#{@password}")
      expect { @client.get('/settings') }.to raise_error(Net::HTTPServerException)

      # Try right username
      @client.set_auth!(@username, @password)
      expect { @client.get('/settings') }.not_to raise_error

      # Make sure all routes are protected
      @client.clear_auth!
      [
        '/about',
        '/gateways/0/rgb_cct/1',
        '/remote_configs',
        '/'
      ].each do |page|
        expect { @client.get(page) }.to raise_error(Net::HTTPServerException), "No auth required for page: #{page}"
      end

      expect { @client.post('/system', {}) }.to raise_error(Net::HTTPServerException)
      expect { @client.post('/firmware', {}) }.to raise_error(Net::HTTPServerException)

      # Clear auth
      @client.set_auth!(@username, @password)
      @client.put('/settings', admin_username: '', admin_password: '')
      @client.clear_auth!

      expect { @client.get('/settings') }.not_to raise_error
    end
  end

  context 'misc routes' do
    it 'should respond to /about' do
      result = @client.get('/about')

      expect(result['firmware']).to eq('milight-hub')
    end

    it 'should respond to /system' do
      expect { @client.post('/system', {}) }.to raise_error('400 "Bad Request"')
    end

    it 'should respond to /remote_configs' do
      result = @client.get('/remote_configs')

      expect(result).to be_a(Array)
      expect(result).to include('rgb_cct')
    end
  end

  context 'sending raw packets' do
    it 'should support sending a raw packet' do
      id = {
        id: 0x2222,
        type: 'rgb_cct',
        group_id: 1
      }
      @client.delete_state(id)

      # Hard-coded packet which should turn the bulb on
      result = @client.post(
        '/raw_commands/rgb_cct',
        packet: '00 DB BF 01 66 D1 BB 66 F7',
        num_repeats: 1
      )
      expect(result['success']).to be_truthy

      sleep(1)

      state = @client.get_state(id)
      puts state.inspect
      expect(state['status']).to eq('ON')
    end
  end
end