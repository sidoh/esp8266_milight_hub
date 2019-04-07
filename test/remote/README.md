## Integration Tests

This integration test suite is built using rspec.  It integrates with espMH in a variety of ways, and monitors externally visible behaviors and states to ensure they match expectations.

### Setup

1. Copy `settings.json.example` to `settings.json` and make appropriate modifications for your setup.
1. Copy `espmh.env.example` to `espmh.env` and make appropriate modifications.  For MQTT tests, you will need an external MQTT broker.
1. Install ruby and the bundler gem.
1. Run `bundle install`.

### Running

Run the tests using `bundle exec rspec`.

### Example output

```
$ bundle exec rspec -f d

Environment
  needs to have a settings.json file
  environment
    should have a host defined
    should respond to /about
  client
    should return IDs

State
  deleting
    should remove retained state
  birth and LWT
    should send birth message when configured
  commands and state
    should affect state
    should publish to state topics
    should publish an update message for each new command
    should respect the state update interval
  :device_id token for command topic
    should support hexadecimal device IDs
    should support decimal device IDs
  :hex_device_id for command topic
    should respond to commands
  :dec_device_id for command topic
    should respond to commands
  :hex_device_id for update/state topics
    should publish updates with hexadecimal device ID
    should publish state with hexadecimal device ID
  :dec_device_id for update/state topics
    should publish updates with hexadecimal device ID
    should publish state with hexadecimal device ID

REST Server
  authentication
    should not require auth unless both username and password are set
    should require auth for all routes when password is set

Settings
  POST settings file
    should clobber patched settings
    should apply POSTed settings
  radio
    should store a set of channels
    should store a listen channel
  static ip
    should boot with static IP when applied

State
  toggle command
    should toggle ON to OFF
    should toggle OFF to ON
  deleting
    should support deleting state
  persistence
    should persist parameters
    should affect member groups when changing group 0
    should keep group 0 state
    should clear group 0 state after member group state changes
    should not clear group 0 state when updating member group state if value is the same
    changing member state mode and then changing level should preserve group 0 brightness for original mode
  fields
    should support the color field
  increment/decrement commands
    should assume state after sufficiently many down commands
    should assume state after sufficiently many up commands
    should affect known state

Finished in 2 minutes 36.9 seconds (files took 0.23476 seconds to load)
38 examples, 0 failures
```