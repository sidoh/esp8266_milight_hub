// #if defined(ARDUINO) && defined(UNIT_TEST)

#include <FS.h>
#include <Arduino.h>
#include "unity.h"

#include <GroupState.h>
#include <GroupStateStore.h>
#include <GroupStateCache.h>
#include <GroupStatePersistence.h>

GroupState color() {
  GroupState s;

  s.setState(MiLightStatus::ON);
  s.setBulbMode(BulbMode::BULB_MODE_COLOR);
  s.setBrightness(100);
  s.setHue(1);
  s.setSaturation(10);

  return s;
}

void test_init_state() {
  GroupState s;

  TEST_ASSERT_EQUAL(s.getBulbMode(), BulbMode::BULB_MODE_WHITE);
  TEST_ASSERT_EQUAL(s.isSetBrightness(), false);
}

void test_state_updates() {
  GroupState s = color();

  // Make sure values are packed and unpacked correctly
  TEST_ASSERT_EQUAL(s.getBulbMode(), BulbMode::BULB_MODE_COLOR);
  TEST_ASSERT_EQUAL(s.getBrightness(), 100);
  TEST_ASSERT_EQUAL(s.getHue(), 1);
  TEST_ASSERT_EQUAL(s.getSaturation(), 10);

  // Make sure brightnesses are tied to mode
  s.setBulbMode(BulbMode::BULB_MODE_WHITE);
  s.setBrightness(0);

  TEST_ASSERT_EQUAL(s.getBulbMode(), BulbMode::BULB_MODE_WHITE);
  TEST_ASSERT_EQUAL(s.getBrightness(), 0);

  s.setBulbMode(BulbMode::BULB_MODE_COLOR);

  TEST_ASSERT_EQUAL(s.getBrightness(), 100);
}

void test_cache() {
  BulbId id1(1, 1, REMOTE_TYPE_FUT089);
  BulbId id2(1, 2, REMOTE_TYPE_FUT089);

  GroupState s = color();
  s.clearDirty();
  s.clearMqttDirty();

  GroupStateCache cache(1);
  GroupState* storedState = cache.get(id2);

  TEST_ASSERT_NULL_MESSAGE(storedState, "Should not retrieve value which hasn't been stored");

  cache.set(id1, s);
  storedState = cache.get(id1);

  TEST_ASSERT_NOT_NULL_MESSAGE(storedState, "Should retrieve a value");
  TEST_ASSERT_TRUE_MESSAGE(s == *storedState, "State should be the same when retrieved");

  cache.set(id2, s);
  storedState = cache.get(id2);

  TEST_ASSERT_NOT_NULL_MESSAGE(storedState, "Should retrieve a value");
  TEST_ASSERT_TRUE_MESSAGE(s == *storedState, "State should be the same when retrieved");

  storedState = cache.get(id1);

  TEST_ASSERT_NULL_MESSAGE(storedState, "Should evict old entry from cache");
}

void test_persistence() {
  BulbId id1(1, 1, REMOTE_TYPE_FUT089);
  BulbId id2(1, 2, REMOTE_TYPE_FUT089);

  GroupStatePersistence persistence;

  persistence.clear(id1);
  persistence.clear(id2);

  GroupState storedState;
  GroupState s = color();
  s.clearDirty();
  s.clearMqttDirty();

  GroupState defaultState = GroupState::defaultState(REMOTE_TYPE_FUT089);

  persistence.get(id1, storedState);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(defaultState), "Should start with clean state");
  persistence.get(id2, storedState);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(defaultState), "Should start with clean state");

  persistence.set(id1, s);

  persistence.get(id2, storedState);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(defaultState), "Should return default for state that hasn't been stored");

  persistence.get(id1, storedState);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(s), "Should retrieve state from flash without modification");

  GroupState newState = s;
  newState.setBulbMode(BulbMode::BULB_MODE_WHITE);
  newState.setBrightness(255);
  persistence.set(id2, newState);

  persistence.get(id1, storedState);

  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(s), "Should retrieve unmodified state");

  persistence.get(id2, storedState);

  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(newState), "Should retrieve modified state");
}

void test_store() {
  BulbId id1(1, 1, REMOTE_TYPE_FUT089);
  BulbId id2(1, 2, REMOTE_TYPE_FUT089);

  // cache 1 item, flush immediately
  GroupStateStore store(1, 0);
  GroupStatePersistence persistence;

  persistence.clear(id1);
  persistence.clear(id2);

  GroupState initState = color();
  GroupState initState2 = color();
  GroupState defaultState = GroupState::defaultState(REMOTE_TYPE_FUT089);
  initState2.setBrightness(255);

  GroupState* storedState;

  storedState = &store.get(id2);
  TEST_ASSERT_TRUE_MESSAGE(*storedState == defaultState, "Should return default for state that hasn't been stored");

  store.set(id1, initState);
  storedState = &store.get(id1);

  TEST_ASSERT_TRUE_MESSAGE(*storedState == initState, "Should return cached state");

  store.flush();
  storedState = &store.get(id1);
  TEST_ASSERT_FALSE_MESSAGE(storedState->isDirty(), "Should not be dirty after flushing");
  TEST_ASSERT_TRUE_MESSAGE(storedState->isEqualIgnoreDirty(initState), "Should return cached state after flushing");

  store.set(id2, defaultState);
  storedState = &store.get(id2);
  TEST_ASSERT_TRUE_MESSAGE(storedState->isEqualIgnoreDirty(defaultState), "Should return cached state");

  storedState = &store.get(id1);
  TEST_ASSERT_TRUE_MESSAGE(storedState->isEqualIgnoreDirty(initState), "Should return persisted state");
}

void test_group_0() {
  BulbId group0Id(1, 0, REMOTE_TYPE_FUT089);
  BulbId id1(1, 1, REMOTE_TYPE_FUT089);
  BulbId id2(1, 2, REMOTE_TYPE_FUT089);

  // cache 1 item, flush immediately
  GroupStateStore store(10, 0);
  GroupStatePersistence persistence;

  persistence.clear(id1);
  persistence.clear(id2);

  GroupState initState = color();
  GroupState initState2 = color();
  GroupState defaultState = GroupState::defaultState(REMOTE_TYPE_FUT089);
  GroupState storedState;
  GroupState expectedState;
  GroupState group0State;

  initState2.setBrightness(255);
  group0State.setHue(100);

  store.set(id1, initState);
  store.set(id2, initState2);

  TEST_ASSERT_FALSE_MESSAGE(group0State.isEqualIgnoreDirty(initState), "group0 state should be different than initState");
  TEST_ASSERT_FALSE_MESSAGE(group0State.isEqualIgnoreDirty(initState2), "group0 state should be different than initState2");

  storedState = store.get(id1);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(initState), "Should fetch persisted state");

  storedState = store.get(id2);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(initState2), "Should fetch persisted state");

  store.set(group0Id, group0State);

  storedState = store.get(id1);
  expectedState = initState;
  expectedState.setHue(group0State.getHue());

  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(expectedState), "Saving group 0 should only update changed field");

  storedState = store.get(id2);
  expectedState = initState2;
  expectedState.setHue(group0State.getHue());
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(expectedState), "Saving group 0 should only update changed field");

  // Test that state for group 0 is not persisted
  storedState = store.get(group0Id);
  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(defaultState), "Group 0 state should not be stored -- should return default state");

  // Should persist group 0 for device types with 0 groups
  BulbId rgbId(1, 0, REMOTE_TYPE_RGB);
  GroupState rgbState = GroupState::defaultState(REMOTE_TYPE_RGB);
  rgbState.setHue(100);
  rgbState.setBrightness(100);

  store.set(rgbId, rgbState);
  store.flush();

  storedState = store.get(rgbId);

  TEST_ASSERT_TRUE_MESSAGE(storedState.isEqualIgnoreDirty(rgbState), "Should persist group 0 for device type with no groups");
}

// setup connects serial, runs test cases (upcoming)
void setup() {
  delay(2000);
  SPIFFS.begin();
  Serial.begin(9600);

  UNITY_BEGIN();

  RUN_TEST(test_init_state);
  RUN_TEST(test_state_updates);
  RUN_TEST(test_cache);
  RUN_TEST(test_persistence);
  RUN_TEST(test_store);
  RUN_TEST(test_group_0);

  UNITY_END();
}

void loop() {
  // nothing to be done here.
}


// #endif