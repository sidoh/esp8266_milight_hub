require 'json'
require 'net/http'
require 'net/http/post/multipart'
require 'uri'

class ApiClient
  def initialize(host, base_id)
    @host = host
    @current_id = Integer(base_id)
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
  end

  def request(type, path, req_body = nil)
    uri = URI("http://#{@host}#{path}")
    Net::HTTP.start(uri.host, uri.port) do |http|
      req_type = Net::HTTP.const_get(type)

      req = req_type.new(uri)
      if req_body
        req['Content-Type'] = 'application/json'
        req_body = req_body.to_json if !req_body.is_a?(String)
        req.body = req_body
      end

      if @username && @password
        req.basic_auth(@username, @password)
      end

      res = http.request(req)
      res.value

      body = res.body

      if res['content-type'].downcase == 'application/json'
        body = JSON.parse(body)
      end

      body
    end
  end

  def upload_json(path, file)
    `curl -s "http://#{@host}#{path}" -X POST -F 'f=@#{file}'`
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
    "/gateways/#{params[:id]}/#{params[:type]}/#{params[:group_id]}"
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
end