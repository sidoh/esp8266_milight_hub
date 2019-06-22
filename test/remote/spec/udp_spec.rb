require 'api_client'
require 'milight'

RSpec.describe 'UDP servers' do
  before(:all) do
    @host = ENV.fetch('ESPMH_HOSTNAME')
    @client = ApiClient.new(@host, ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE'))
    @client.upload_json('/settings', 'settings.json')

    @client.patch_settings( mqtt_parameters() )
    @client.patch_settings( mqtt_update_topic_pattern: '' )
  end

  before(:each) do
    @id_params = {
      id: @client.generate_id,
      type: 'rgbw',
      group_id: 1
    }
    @v6_id_params = {
      id: @client.generate_id,
      type: 'rgbw',
      group_id: 1
    }
    @client.delete_state(@id_params)

    @v5_udp_port = ENV.fetch('ESPMH_V5_UDP_PORT')
    @v6_udp_port = ENV.fetch('ESPMH_V6_UDP_PORT')
    @discovery_port = ENV.fetch('ESPMH_DISCOVERY_PORT')

    @client.patch_settings(
      gateway_configs: [
        [
          @id_params[:id], # device ID
          @v5_udp_port,
          5                # protocol version (gem uses v5)
        ],
        [
          @v6_id_params[:id], # device ID
          @v6_udp_port,
          6                # protocol version
        ]
      ]
    )
    @udp_client = Milight::Controller.new(ENV.fetch('ESPMH_HOSTNAME'), @v5_udp_port)
    @mqtt_client = create_mqtt_client()
  end

  context 'on/off commands' do
    it 'should result in state changes' do
      @udp_client.group(@id_params[:group_id]).on

      # Wait for packet to be processed
      sleep 1

      state = @client.get_state(@id_params)
      expect(state['status']).to eq('ON')

      @udp_client.group(@id_params[:group_id]).off

      # Wait for packet to be processed
      sleep 1

      state = @client.get_state(@id_params)
      expect(state['status']).to eq('OFF')
    end

    it 'should result in an MQTT update' do
      desired_state = {
        'status' => 'ON',
        'level' => 48
      }
      seen_state = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen_state = desired_state.all? { |k,v| v == message[k] }
      end
      @udp_client.group(@id_params[:group_id]).on.brightness(48)
      @mqtt_client.wait_for_listeners

      expect(seen_state).to eq(true)
    end
  end

  context 'color and brightness commands' do
    it 'should result in state changes' do
      desired_state = {
        'status' => 'ON',
        'level' => 48,
        'hue' => 357
      }
      seen_state = false

      @mqtt_client.on_state(@id_params) do |id, message|
        seen_state = desired_state.all? { |k,v| v == message[k] }
      end

      @udp_client.group(@id_params[:group_id])
        .on
        .colour('#ff0000')
        .brightness(48)

      @mqtt_client.wait_for_listeners

      expect(seen_state).to eq(true)
    end
  end

  context 'discovery' do
    before(:all) do
      @client.patch_settings(
        discovery_port: ENV.fetch('ESPMH_DISCOVERY_PORT')
      )

      @discovery_host = '<broadcast>'

      @discovery_socket = UDPSocket.new
      @discovery_socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_BROADCAST, true)
      @discovery_socket.bind('0.0.0.0', 0)
    end

    it 'should respond to v5 discovery' do
      @discovery_socket.send('Link_Wi-Fi', 0, @discovery_host, @discovery_port)

      # wait for response
      sleep 1

      response, _ = @discovery_socket.recvfrom_nonblock(1024)
      response = response.split(',')

      expect(response.length).to      eq(2), "Should be a comma-separated list with two elements"
      expect(response[0]).to          eq(@host)
      expect(response[1].to_i(16)).to eq(@id_params[:id])
    end

    it 'should respond to v6 discovery' do
      @discovery_socket.send('HF-A11ASSISTHREAD', 0, @host, @discovery_port)

      # wait for response
      sleep 1

      response, _ = @discovery_socket.recvfrom_nonblock(1024)
      response = response.split(',')

      expect(response.length).to      eq(3), "Should be a comma-separated list with three elements"
      expect(response[0]).to          eq(@host)
      expect(response[1].to_i(16)).to eq(@v6_id_params[:id])
      expect(response[2]).to          eq('HF-LPB100')
    end
  end
end