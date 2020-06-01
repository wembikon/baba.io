/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/windows_timer_impl.h"

#include "baba/semantics.h"

namespace baba::os::windows_timer_impl {

namespace {

void CALLBACK timer_callback(PTP_CALLBACK_INSTANCE, PVOID data, PTP_TIMER) {
  const auto ctx = static_cast<iocp_timer_context *>(data);
  PostQueuedCompletionStatus(ctx->iocp_fd, 0, NULL, (LPOVERLAPPED)ctx->evt);
}

}  // namespace

create_timer_result open(iocp_timer_context *ctx) noexcept {
  RUNTIME_ASSERT(ctx);
  if (const auto fd = CreateThreadpoolTimer(timer_callback, ctx, NULL); fd == NULL) {
    return {errno, INVALID_HANDLE_VALUE};
  } else {
    return {ec::OK, fd};
  }
}

void close(io_handle fd) noexcept { CloseThreadpoolTimer((PTP_TIMER)fd); }

error_code init_timer(io_handle fd, uint32_t timeout_ms) noexcept {
  LONGLONG delay = -((LONGLONG)timeout_ms) * 10000LL;
  FILETIME due_time = {(DWORD)delay, (DWORD)(delay >> 32)};
  SetThreadpoolTimer((PTP_TIMER)fd, &due_time, 0, 0);  // never fails!
  return ec::OK;
}

}  // namespace baba::os::windows_timer_impl