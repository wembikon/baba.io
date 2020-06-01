/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::MockFunction;
using ::testing::Test;

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/lifetime.h"
#include "os/common/event_registrar.h"
#include "os/reactor/epoll/epoll_reactor_event.h"
#include "os/reactor/epoll/events/epoll_timer.h"

using namespace baba;
using namespace baba::os;

#include <deque>
#include <functional>
#include <iostream>
#include <memory>

using namespace std::placeholders;

class fixture : public Test {
 protected:
  fixture() : _strand(_service) {
    _strand.impl()->on_enqueue = std::bind(&fixture::enqueue_for_completion, this, _1);
    _enqueue_for_deletion = std::bind(&fixture::enqueue_for_deletion, this, _1);
  }

  void enqueue_for_completion(reactor_event *evt) {
    EXPECT_NE(evt, nullptr);
    _event_for_completion.emplace_back(evt);
  }

  void enqueue_for_deletion(reactor_event *evt) {
    EXPECT_NE(evt, nullptr);
    _event_for_deletion.emplace_back(evt);
  }

  error_code init_command(reactor_event *evt, uint32_t) {
    EXPECT_NE(evt, nullptr);
    _dummy_reactor.emplace_back(evt);
    return init_command_error;
  }

  void run_reactor_step() {
    ASSERT_TRUE(!_dummy_reactor.empty());
    _dummy_reactor.front()->has_active_event = true;
    _dummy_reactor.front()->react(ec::OK);
    _dummy_reactor.pop_front();
  }

  void run_completion_step() {
    ASSERT_TRUE(!_event_for_completion.empty());
    _event_for_completion.front()->complete();
    _event_for_completion.pop_front();
  }

  void run_deletion_step() {
    ASSERT_TRUE(!_event_for_deletion.empty());
    auto evt = _event_for_deletion.front();
    _event_for_deletion.pop_front();
    enqueue_for_completion(evt);
  }

  auto make_event_object(const reactor_io_descriptor_ptr &io_desc) {
    return std::shared_ptr<epoll_timer>(
        new epoll_timer(io_desc, _strand.impl(), &_registrar, _enqueue_for_deletion,
                        std::bind(&fixture::init_command, this, _1, _2),
                        [](auto, auto, auto, auto) { return ec::OK; }));
  }

  auto make_io_desc(io_handle fd) {
    auto io_desc = std::make_shared<reactor_io_descriptor>();
    io_desc->fd = fd;
    return io_desc;
  }

  void destruct(std::shared_ptr<epoll_timer> &evt) {
    evt->dispose();
    evt.reset();
  }

  bool is_in_reactor(uint64_t evt_id) {
    for (auto &evt : _dummy_reactor) {
      if (evt->id == evt_id) {
        return true;
      }
    }
    return false;
  }

  event_registrar _registrar;

  epoll_service _service;
  completion_strand _strand;

  enqueue_for_deletion_fn _enqueue_for_deletion;

  std::deque<reactor_event *> _event_for_completion;
  std::deque<reactor_event *> _event_for_deletion;
  std::deque<reactor_event *> _dummy_reactor;

  error_code init_command_error = ec::OK;
};

TEST_F(fixture, register_command_to_reactor) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  auto evtobj = make_event_object(io_desc);
  auto evt = evtobj->evt();

  evtobj->timeout(1000, [](auto) {});
  EXPECT_TRUE(is_in_reactor(evt->id));
}

TEST_F(fixture, enqueue_delete_event_if_did_command) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  auto evtobj = make_event_object(io_desc);
  evtobj->timeout(1000, [](auto) {});

  destruct(evtobj);
  EXPECT_EQ(_event_for_deletion.size(), 1);
}

TEST_F(fixture, properly_delete_event_if_did_command_reactor_in_sequence) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  MockFunction<void()> on_delete_cb;
  auto evtobj = make_event_object(io_desc);
  evtobj->evt()->on_delete = [&on_delete_cb] { on_delete_cb.Call(); };
  evtobj->timeout(1000, [](auto) {});

  destruct(evtobj);

  // Mimicking the correct sequence

  // Completing the event
  run_reactor_step();
  run_completion_step();

  // Completing the deletion process
  run_deletion_step();
  EXPECT_CALL(on_delete_cb, Call());
  run_completion_step();
}

TEST_F(fixture, properly_delete_event_if_did_command_reactor_mal_sequence) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  MockFunction<void()> on_delete_cb;
  auto evtobj = make_event_object(io_desc);
  auto evt = evtobj->evt();
  evtobj->evt()->on_delete = [&on_delete_cb] { on_delete_cb.Call(); };
  evtobj->timeout(1000, [](auto) {});

  destruct(evtobj);

  // Mimicking the mal sequence when running multiple reactor threads

  // Enqueue the final event to the completion queue first
  run_deletion_step();
  EXPECT_CALL(on_delete_cb, Call()).Times(0);

  // Run `react` make sure that the active event will be in the completion queue later
  run_reactor_step();
  EXPECT_EQ(evt->has_active_event, true);

  // This will re-enqueue the deletion event to the completion queue because we still
  // have an active event pending.
  run_completion_step();

  // Run the active event `complete`
  EXPECT_CALL(on_delete_cb, Call()).Times(0);
  run_completion_step();
  EXPECT_EQ(evt->has_active_event, false);

  // Run the final event `complete`
  EXPECT_CALL(on_delete_cb, Call());
  run_completion_step();
}

/**
 * Move
 **/

TEST_F(fixture, move_construct_trivially_delete_moved_event) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  auto evtobj = make_event_object(io_desc);
  evtobj->timeout(1000, [](auto) {});  // Do a command to not trivially delete for now

  epoll_timer other = std::move(*evtobj);
  destruct(evtobj);  // Should be trivially deleted
  EXPECT_EQ(_event_for_deletion.size(), 0);
}

TEST_F(fixture, properly_delete_move_constructed_event) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  MockFunction<void()> on_delete_cb;
  auto evtobj = make_event_object(io_desc);
  evtobj->evt()->on_delete = [&on_delete_cb] { on_delete_cb.Call(); };
  evtobj->timeout(1000, [](auto) {});  // Do a command to not trivially delete for now

  {
    // `other` should undergo the deletion step after it goes out of scope
    epoll_timer other = std::move(*evtobj);
    // Change of API now needs to call dispose explicitly
    other.dispose();
  }

  // Complete any pending event so we can delete it
  run_reactor_step();
  run_completion_step();

  run_deletion_step();
  EXPECT_CALL(on_delete_cb, Call());
  run_completion_step();
}

TEST_F(fixture, move_assign_trivially_delete_moved_event) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  auto evtobj = make_event_object(io_desc);
  evtobj->timeout(1000, [](auto) {});  // Do a command to not trivially delete for now

  epoll_timer other;
  other = std::move(*evtobj);
  destruct(evtobj);  // Should be trivially deleted
  EXPECT_EQ(_event_for_deletion.size(), 0);
}

TEST_F(fixture, properly_delete_move_assigned_event) {
  int handle = 10;
  auto io_desc = make_io_desc((HANDLE)handle);

  MockFunction<void()> on_delete_cb;
  auto evtobj = make_event_object(io_desc);
  evtobj->evt()->on_delete = [&on_delete_cb] { on_delete_cb.Call(); };
  evtobj->timeout(1000, [](auto) {});  // Do a command to not trivially delete for now

  {
    // `other` should undergo the deletion step after it goes out of scope
    epoll_timer other;
    other = std::move(*evtobj);
    other.dispose();
  }

  // Complete any pending event so we can delete it
  run_reactor_step();
  run_completion_step();

  run_deletion_step();
  EXPECT_CALL(on_delete_cb, Call());
  run_completion_step();
}
