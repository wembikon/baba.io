/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"

#include <memory>
#include <mutex>

namespace baba {

using task_fn = std::function<void()>;

class cancellable_task_;
using cancellable_task_ptr = std::shared_ptr<cancellable_task_>;
class cancellable_task_ final : public std::enable_shared_from_this<cancellable_task_> {
 private:
  struct private_tag {};
  cancellable_task_() = default;

 public:
  NOT_COPYABLE(cancellable_task_)
  NOT_MOVEABLE(cancellable_task_)

  static cancellable_task_ptr create() noexcept;
  cancellable_task_(private_tag) noexcept;
  task_fn operator()(const task_fn &task) noexcept;
  void cancel() noexcept;

 private:
  std::mutex _in_scope_mtx;
  bool _in_scope = true;
};

class cancellable_task final {
 public:
  inline cancellable_task() : _task(cancellable_task_::create()) {}
  inline ~cancellable_task() noexcept { _task->cancel(); }
  inline task_fn operator()(const task_fn &task) noexcept { return (*_task)(task); }

 private:
  cancellable_task_ptr _task;
};

}  // namespace baba