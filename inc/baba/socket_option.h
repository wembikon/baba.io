/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/ip_address.h"

#include <memory>

namespace baba {

class reuse_address;
class set_outgoing_interface;
class join_group;
namespace socket {
error_code set_option(io_handle fd, const reuse_address &opt) noexcept;
error_code set_option(io_handle fd, const set_outgoing_interface &opt) noexcept;
error_code set_option(io_handle fd, const join_group &opt) noexcept;
error_code get_option(io_handle fd, reuse_address &opt) noexcept;
error_code get_option(io_handle fd, set_outgoing_interface &opt) noexcept;
error_code get_option(io_handle fd, join_group &opt) noexcept;
}  // namespace socket

class reuse_address final {
 public:
  struct pimpl;

  reuse_address(const reuse_address &other) noexcept;
  reuse_address &operator=(const reuse_address &other) noexcept;
  reuse_address(reuse_address &&tmp) noexcept;
  reuse_address &operator=(reuse_address &&tmp) noexcept;
  reuse_address() noexcept;
  ~reuse_address() noexcept;

  reuse_address(bool yes) noexcept;

 private:
  friend error_code socket::set_option(io_handle fd, const reuse_address &opt) noexcept;
  friend error_code socket::get_option(io_handle fd, reuse_address &opt) noexcept;
  std::unique_ptr<pimpl> _pimpl;
};

class set_outgoing_interface final {
 public:
  struct pimpl;

  set_outgoing_interface(const set_outgoing_interface &other) noexcept;
  set_outgoing_interface &operator=(const set_outgoing_interface &other) noexcept;
  set_outgoing_interface(set_outgoing_interface &&tmp) noexcept;
  set_outgoing_interface &operator=(set_outgoing_interface &&tmp) noexcept;
  set_outgoing_interface() noexcept;
  ~set_outgoing_interface() noexcept;

  set_outgoing_interface(const ip_address &addr) noexcept;

 private:
  friend error_code socket::set_option(io_handle fd, const set_outgoing_interface &opt) noexcept;
  friend error_code socket::get_option(io_handle fd, set_outgoing_interface &opt) noexcept;
  std::unique_ptr<pimpl> _pimpl;
};

class join_group final {
 public:
  struct pimpl;

  join_group(const join_group &other) noexcept;
  join_group &operator=(const join_group &other) noexcept;
  join_group(join_group &&tmp) noexcept;
  join_group &operator=(join_group &&tmp) noexcept;
  join_group() noexcept;
  ~join_group() noexcept;

  join_group(const ip_address &iface, uint32_t iface_index, const ip_address &mcast) noexcept;

 private:
  friend error_code socket::set_option(io_handle fd, const join_group &opt) noexcept;
  friend error_code socket::get_option(io_handle fd, join_group &opt) noexcept;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba