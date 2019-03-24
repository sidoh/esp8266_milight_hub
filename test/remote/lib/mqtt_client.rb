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
      "#{sprintf '0x%04X', params[:id]}/#{params[:type]}/#{params[:group_id]}"
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
    topic = "#{@topic_prefix}#{path}/#{id_topic_suffix(id_params)}"

    on_message(topic, timeout) do |topic, message|
      topic_parts = topic.split('/')

      yield(
        {
          id: topic_parts[2].to_i(16),
          type: topic_parts[3],
          group_id: topic_parts[4].to_i
        },
        JSON.parse(message)
      )
    end
  end

  def on_message(topic, timeout = 10, &block)
    @listen_threads << Thread.new do
      begin
        Timeout.timeout(timeout) do
          @client.get(topic) do |topic, message|
            raise BreakListenLoopError if yield(topic, message)
          end
        end
      rescue Timeout::Error => e
        puts "Timed out listening for message on: #{topic}"
        puts e.backtrace.join("\n")
      rescue BreakListenLoopError
      end
    end
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