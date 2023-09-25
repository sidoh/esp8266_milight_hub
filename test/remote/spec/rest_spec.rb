require 'api_client'

RSpec.describe 'REST Server' do
  before(:all) do
    @client = ApiClient.new(ENV.fetch('ESPMH_HOSTNAME'), ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.reset_settings

    @username = 'a'
    @password = 'a'
  end

  before(:each) do
    @id_params = {
      id: @client.generate_id,
      type: 'rgb_cct',
      group_id: 1
    }
    @client.delete_state(@id_params)
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
      expect(state['status']).to eq('ON')
    end
  end

  context 'device aliases' do
    before(:all) do
      @device_id = {
        id: @client.generate_id,
        type: 'rgb_cct',
        group_id: 1
      }
      @alias = 'test'

      @client.patch_settings(
        group_id_aliases: {
          @alias => [
            @device_id[:type],
            @device_id[:id],
            @device_id[:group_id]
          ]
        }
      )

      @client.delete_state(@device_id)
    end

    it 'should respond with a 404 for an alias that doesn\'t exist' do
      expect {
        @client.put("/gateways/__#{@alias}", status: 'on')
      }.to raise_error(Net::HTTPServerException)
    end

    it 'should update state for known alias' do
      path = "/gateways/#{@alias}?blockOnQueue=true"

      @client.put(path, status: 'ON', hue: 100)
      state = @client.get(path)

      expect(state['status']).to eq('ON')
      expect(state['hue']).to eq(100)

      # ensure state for the non-aliased ID is the same
      state = @client.get_state(@device_id)

      expect(state['status']).to eq('ON')
      expect(state['hue']).to eq(100)
    end

    it 'should handle saving bad input gracefully' do
      values_to_try = [
        'string',
        123,
        [ ],
        { 'test' => [ 'rgb_cct' ] },
        { 'test' => [ 'rgb_cct', 1 ] },
        { 'test' => [ 'rgb_cct', '1', 2 ] },
        { 'test' => [ 'abc' ] }
      ]

      values_to_try.each do |v|
        expect {
          @client.patch_settings(group_id_aliases: v)
        }.to_not raise_error
      end
    end
  end

  context 'async state' do
    it 'should respond with state for GET, regardless of blockOnQueue param value' do
      @client.patch_state({status: 'ON'}, @id_params)
      response = @client.get_state(@id_params.merge(blockOnQueue: false))

      expect(response['status']).to eq('ON')
    end
  end

  context 'alias routes' do
    before(:each) do
      @client.clear_aliases
      @test_alias = {
        alias: 'test',
        device_type: 'rgb_cct',
        device_id: 1,
        group_id: 2
      }
    end

    it 'GET /aliases should work when there are no aliases' do
      result = @client.get('/aliases')

      expect(result).to be_a(Hash)
      expect(result['aliases']).to be_a(Array)
      expect(result['aliases'].length).to eq(0)
      expect(result['page']).to eq(1)
      expect(result['count']).to eq(0)
    end

    it 'POST /aliases should create an alias' do
      result = @client.post('/aliases', @test_alias)

      expect(result['success']).to be_truthy
      expect(result['id']).to be_a(Numeric)

      aliases = @client.get('/aliases')['aliases']

      expect(aliases.length).to eq(1)
      expect(aliases[0]['alias']).to eq(@test_alias[:alias])
      expect(aliases[0]['device_type']).to eq(@test_alias[:device_type])
      expect(aliases[0]['device_id']).to eq(@test_alias[:device_id])
      expect(aliases[0]['group_id']).to eq(@test_alias[:group_id])
    end

    it 'DELETE /aliases/:alias should delete an alias' do
      create_response = @client.post('/aliases', @test_alias)
      delete_response = @client.delete("/aliases/#{create_response['id']}")

      expect(delete_response['success']).to be_truthy

      list_response = @client.get('/aliases')

      expect(list_response['aliases'].length).to eq(0)
    end

    it 'DELETE /aliases.bin should clear all aliases' do
      @client.post('/aliases', @test_alias)
      response = @client.delete('/aliases.bin')
      expect(response['success']).to be_truthy

      list_response = @client.get('/aliases')

      expect(list_response['aliases'].length).to eq(0)
    end

    it 'GET /aliases.bin should return a backup file of aliases which POST /aliases.bin restores' do
      create_response = @client.post('/aliases', @test_alias)
      result = @client.get('/aliases.bin')

      @client.clear_aliases
      expect(@client.get('/aliases')['aliases'].length).to eq(0)

      @client.upload_string_as_file('/aliases.bin', result)
      response = @client.get('/aliases')

      expect(response['aliases'].length).to eq(1)
      expect(response['aliases'].first['alias']).to eq(@test_alias[:alias])
    end

    it 'PUT /aliases/:id should update an alias' do
      create_response = @client.post('/aliases', @test_alias)

      updated_alias = {**@test_alias, device_id: 3, alias: 'updated_alias'}
      update_response = @client.put("/aliases/#{create_response['id']}", updated_alias)

      expect(update_response['success']).to be_truthy

      list_response = @client.get('/aliases')

      expect(list_response['aliases'].length).to eq(1)
      expect(list_response['aliases'][0]['alias']).to eq(updated_alias[:alias])
      expect(list_response['aliases'][0]['device_type']).to eq(updated_alias[:device_type])
      expect(list_response['aliases'][0]['device_id']).to eq(updated_alias[:device_id])
      expect(list_response['aliases'][0]['group_id']).to eq(updated_alias[:group_id])
    end

    it 'should support uploading a large list of aliases' do
      num_aliases = 20
      csv = (1..num_aliases).map do |i|
        [i, "test#{i}", 'rgb_cct', i, 1]
      end.flatten.join("\0")

      Tempfile.create('aliases.bin') do |file|
        file.write(num_aliases)
        file.write("\0")
        file.write(csv)
        file.close

        @client.upload_json('/aliases.bin', file.path)
      end

      result = @client.get('/aliases')

      expect(result['count']).to eq(num_aliases)
    end

    it 'should support paging' do
      csv = (1..20).map do |i|
        [i, "test#{i}", 'rgb_cct', i, 1]
      end.flatten.join("\0")
      # Write length
      csv = [20, csv].join("\0")

      @client.upload_string_as_file('/aliases.bin', csv)
      result = @client.get('/aliases?page_size=10')

      expect(result['num_pages']).to eq(2)
      expect(result['count']).to eq(20)
      expect(result['page']).to eq(1)
      expect(result['aliases'].length).to eq(10)

      all_aliases = []
      page = 1
      num_pages = result['num_pages']

      while true do
        result = @client.get("/aliases?page=#{page}&page_size=10")
        all_aliases += result['aliases']

        break if (page += 1) > num_pages
      end

      expect(all_aliases.length).to eq(20)
    end
  end

  context 'backup routes' do
    before(:each) do
      @client.reset_settings
    end

    it 'should preserve settings when creating a backup' do
      @client.patch_settings({
        mqtt_server: 'abc',
        mqtt_topic_pattern: "123",
      })

      # Create a few aliases
      aliases = (1..10).map do |i|
        {
          alias: "test#{i}",
          device_type: 'rgb_cct',
          device_id: @client.generate_id,
          group_id: 1
        }
      end
      aliases.each do |a|
        @client.post('/aliases', a)
      end

      backup = @client.get('/backup')

      # Reset settings
      @client.reset_settings

      # Ensure settings were reset
      expect(@client.get('/settings')['mqtt_server']).to eq('')
      expect(@client.get('/aliases')['aliases'].length).to eq(0)

      # Upload backup
      @client.upload_string_as_file('/backup', backup)

      # Check settings
      result = @client.get('/settings')

      expect(result['mqtt_server']).to eq('abc')
      expect(result['mqtt_topic_pattern']).to eq("123")

      # Check aliases
      result = @client.get('/aliases')

      expected_aliases = Set.new(aliases.map { |a| a[:alias] })
      actual_aliases = Set.new(result['aliases'].map { |a| a['alias'] })

      expect(actual_aliases).to eq(expected_aliases)
    end

    it 'should override existing settings when restoring a backup' do
      @client.patch_settings({
        mqtt_username: 'abc',
      })
      @client.post('/aliases', {
          alias: "test_alias",
          device_type: 'rgb_cct',
          device_id: @client.generate_id,
          group_id: 1
      })

      result = @client.get('/backup')

      @client.patch_settings({
        mqtt_server: 'def',
      })

      @client.upload_string_as_file('/backup', result)

      settings = @client.get('/settings')
      expect(settings['mqtt_username']).to eq('abc')
    end

    it 'should reject invalid backups' do
      expect { @client.upload_string_as_file('/backup', 'invalid') }.to raise_error(Net::HTTPServerException)
    end
  end
end