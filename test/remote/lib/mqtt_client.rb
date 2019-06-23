require 'mqtt'
require 'timeout'
require 'json'

class MqttClient
  BreakListenLoopError = Class.new(StandardError)

  def initialize(server, username, password, topic_prefix)
    @client = MQTT::Client.connect("mqtt://#{username}:#{password}@#{server}")
    @topic_prefix = topic_prefix
    @listen_threads = []
  end

  def disconnect
    @client.disconnect
  end

  def reconnect
    @client.disconnect
    @client.connect
  end

  def wait_for_message(topic, timeout = 10)
    on_message(topic, timeout) { |topic, message| }
    wait_for_listeners
  end

  def id_topic_suffix(params)
    if params
      str_id = if params[:id_format] == 'decimal'
        params[:id].to_s
      else
        sprintf '0x%04X', params[:id]
      end

      "#{str_id}/#{params[:type]}/#{params[:group_id]}"
    else
      "+/+/+"
    end
  end

  def on_update(id_params = nil, timeout = 10, &block)
    on_id_message('updates', id_params, timeout, &block)
  end

  def on_state(id_params = nil, timeout = 10, &block)
    on_id_message('state', id_params, timeout, &block)
  end

  def on_id_message(path, id_params, timeout, &block)
    sub_topic = "#{@topic_prefix}#{path}/#{id_topic_suffix(nil)}"

    on_message(sub_topic, timeout) do |topic, message|
      topic_parts = topic.split('/')
      topic_id_params = {
        id: topic_parts[2].to_i(16),
        type: topic_parts[3],
        group_id: topic_parts[4].to_i,
        unparsed_id: topic_parts[2]
      }

      if !id_params || %w(id type group_id).all? { |k| k=k.to_sym; topic_id_params[k] == id_params[k] }
        begin
          message = JSON.parse(message)
        rescue JSON::ParserError => e
        end

        yield( topic_id_params, message )
      end
    end
  end

  def on_message(topic, timeout = 10, raise_error = true, &block)
    @listen_threads << Thread.new do
      begin
        Timeout.timeout(timeout) do
          @client.get(topic) do |topic, message|
            ret_val = yield(topic, message)
            raise BreakListenLoopError if ret_val
          end
        end
      rescue Timeout::Error => e
        puts "Timed out listening for message on: #{topic}"
        raise e if raise_error
      rescue BreakListenLoopError
      end
    end
  end

  def publish(topic, state = {}, retain = false)
    state = state.to_json unless state.is_a?(String)

    @client.publish(topic, state, retain)
  end

  def patch_state(id_params, state = {})
    @client.publish(
      "#{@topic_prefix}commands/#{id_topic_suffix(id_params)}",
      state.to_json
    )
  end

  def wait_for_listeners
    @listen_threads.each(&:join)
    @listen_threads.clear
  end
end