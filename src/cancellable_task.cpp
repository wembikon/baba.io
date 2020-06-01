/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "baba/cancellable_task.h"

namespace baba {

cancellable_task_ptr cancellable_task_::create() noexcept {
  return std::make_shared<cancellable_task_>(private_tag());
}

cancellable_task_::cancellable_task_(private_tag) noexcept : cancellable_task_() {}

task_fn cancellable_task_::operator()(const task_fn &task) noexcept {
  std::weak_ptr<cancellable_task_> w = shared_from_this();
  return [task, w = std::move(w)]() {
    if (const auto o = w.lock()) {
      std::lock_guard l(o->_in_scope_mtx);
      if (o->_in_scope) {
        task();
      }
    }
  };
}

void cancellable_task_::cancel() noexcept {
  std::lock_guard l(_in_scope_mtx);
  _in_scope = false;
}

}  // namespace baba