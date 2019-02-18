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

  def request(type, path, req_body = nil)
    uri = URI("http://#{@host}#{path}")
    Net::HTTP.start(uri.host, uri.port) do |http|
      req_type = Net::HTTP.const_get(type)

      req = req_type.new(uri)
      if req_body
        req['Content-Type'] = 'application/json'
        req.body = req_body
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
    `curl -s "http://#{@host}#{path}" -X POST -F 'f=@settings.json'`
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

  def get_state(params = {})
    get("/gateways/#{params[:id]}/#{params[:type]}/#{params[:group_id]}")
  end

  def patch_state(state, params = {})
    put("/gateways/#{params[:id]}/#{params[:type]}/#{params[:group_id]}", state.to_json)
  end
end