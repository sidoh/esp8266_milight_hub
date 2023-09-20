require 'json'
require 'net/http'
require 'net/http/post/multipart'
require 'uri'
require 'tempfile'

class ApiClient
  class LivenessError < StandardError; end

  def initialize(host, base_id)
    @host = host
    @current_id = Integer(base_id)
  end

  def self.from_environment
    ApiClient.new(
      ENV.fetch('ESPMH_HOSTNAME'),
      ENV.fetch('ESPMH_TEST_DEVICE_ID_BASE')
    )
  end

  def reset_settings(settings_file = 'settings.json')
    upload_json('/settings', settings_file)

    # clear device aliases
    clear_aliases
  end

  def clear_aliases
    delete('/aliases.bin')
  end

  def live?(ping_count = 10, timeout = 1, inverted = false)
    print "Waiting for #{@host} to be #{inverted ? 'un' : ''}available..."

    ping_test = Net::Ping::External.new(@host, timeout: timeout)
    check = inverted ? -> { not ping_test.ping? } : -> { ping_test.ping? }
    result = nil
    last_call = Time.now

    ping_count.times do
      result = check.call
      break if result

      this_call = Time.now
      time_since_last_call = this_call - last_call

      if time_since_last_call < timeout then
        sleep (timeout - time_since_last_call)
      end

      last_call = this_call

      print '.'
    end

    puts result ? 'OK' : 'FAIL'
    result
  end

  def wait_for_liveness(ping_count = 10, timeout = 5)
    raise LivenessError unless live?(ping_count, timeout)
  end

  def wait_for_unavailable(ping_count = 500, timeout = 0.1)
    raise LivenessError unless live?(ping_count, timeout, true)
  end

  def generate_id
    id = @current_id
    @current_id += 1
    id
  end

  def set_auth!(username, password)
    @username = username
    @password = password
  end

  def clear_auth!
    @username = nil
    @password = nil
  end

  def reboot
    post('/system', '{"command":"restart"}')
    wait_for_unavailable
  end

  def request(type, path, req_body = nil)
    uri = URI("http://#{@host}#{path}")
    Net::HTTP.start(uri.host, uri.port) do |http|
      req_type = Net::HTTP.const_get(type)

      req = req_type.new(uri)

      if req_body
        if req_body.is_a?(File)
          req['Content-Length'] = req_body.size.to_s
          req.set_form [['file', req_body]], 'multipart/form-data'
        else
          req_body = req_body.to_json if !req_body.is_a?(String)

          req['Content-Type'] = 'application/json'
          req['Content-Length'] = req_body.size.to_s
          req.body = req_body
        end
      end

      http.read_timeout = 3

      if @username && @password
        req.basic_auth(@username, @password)
      end

      res = http.request(req)

      begin
        res.value
      rescue Exception => e
        puts "REST Client Error: #{e}\nBody:\n#{res.body}"
        raise e
      end

      body = res.body

      if res['content-type'].downcase == 'application/json'
        begin
          body = JSON.parse(body)
        rescue JSON::ParserError => e
          puts "JSON Parse Error: #{e}\nBody:\n#{res.body}"
          raise e
        end
      end

      body
    end
  end

  def upload_json(path, file)
    if file.is_a?(String)
      upload_json(path, File.new(file))
    else
      request(:Post, path, file)
    end
  end

  def upload_string_as_file(path, string)
    Tempfile.create('tmp-upload-file') do |f|
      f.write(string)
      f.close
      upload_json(path, File.new(f))
    end
  end

  def patch_settings(settings)
    put('/settings', settings)
  end

  def get(path)
    request(:Get, path)
  end

  def put(path, body)
    request(:Put, path, body)
  end

  def post(path, body)
    request(:Post, path, body)
  end

  def delete(path)
    request(:Delete, path)
  end

  def state_path(params = {})
    query = if params[:blockOnQueue].nil? || params[:blockOnQueue]
      "?blockOnQueue=true"
    else
      ""
    end

    "/gateways/#{params[:id]}/#{params[:type]}/#{params[:group_id]}#{query}"
  end

  def delete_state(params = {})
    delete(state_path(params))
  end

  def get_state(params = {})
    get(state_path(params))
  end

  def patch_state(state, params = {})
    put(state_path(params), state.to_json)
  end

  def schedule_transition(_id_params, transition_params)
    id_params = {
      device_id: _id_params[:id],
      remote_type: _id_params[:type],
      group_id: _id_params[:group_id]
    }

    post("/transitions", id_params.merge(transition_params))
  end

  def transitions
    get('/transitions')['transitions']
  end
end