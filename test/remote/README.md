## Integration Tests

This integration test suite is built using rspec.  It integrates with espMH in a variety of ways, and monitors externally visible behaviors and states to ensure they match expectations.

### Setup

1. Copy `settings.json.example` to `settings.json` and make appropriate modifications for your setup.
1. Copy `espmh.env.example` to `espmh.env` and make appropriate modifications.  For MQTT tests, you will need an external MQTT broker.
1. Install ruby and the bundler gem.
1. Run `bundle install`.

### Running

Run the tests using `bundle exec rspec`.