/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "os/socket/ip_address.h"

namespace baba::os {

// Declare for friends
class int_option;
class address_option;
class group_option;

namespace socket_impl {

error_code get_option(io_handle h, int_option &opt) noexcept;
error_code get_option(io_handle h, address_option &opt) noexcept;
error_code get_option(io_handle h, group_option &opt) noexcept;

}  // namespace socket_impl

// Only meant for related option to derive
class option {
 protected:
  // Used in getting option
  option() noexcept;
  // Used in setting option
  option(int level, int optname) noexcept;

 public:
  int get_level() const noexcept;
  int get_name() const noexcept;

 protected:
  int level_;
  int optname_;
};

// Only meant for related option to derive
// Represents options with value int
class int_option : public option {
 protected:
  // Used in getting option
  int_option() noexcept;
  // Used in setting option
  int_option(int level, int optname, int val) noexcept;

 private:
  // Setters are only used by sock_impl
  void set_value(int val) noexcept;
  friend error_code os::socket_impl::get_option(io_handle h, int_option &opt) noexcept;

 public:
  int get_value() const noexcept;

 protected:
  int optval_;
};

// Only meant for related option to derive
// Represents options with value of ip address
class address_option : public option {
 protected:
  // Used in getting option
  address_option() noexcept;
  // Used in setting option
  address_option(int level, int optname, const ip_address &addr) noexcept;

 private:
  // Setters are only used by sock_impl
  void set_address(const ip_address &addr) noexcept;
  friend error_code os::socket_impl::get_option(io_handle h, address_option &opt) noexcept;

 public:
  const ip_address &get_address() const noexcept;

 protected:
  ip_address addr_;
};

// Only meant for related option to derive
// Represents options with value ip_mreq/ip_mreqn(new) or ipv6_mreq
class group_option : public option {
 protected:
  // Used in getting option
  group_option() noexcept;
  // Used in setting option
  group_option(int level, int optname, const ip_address &iface, uint32_t iface_index,
               const ip_address &mcast) noexcept;

 private:
  // Setters are only used by sock_impl
  void set_iface_address(const ip_address &addr) noexcept;
  void set_iface_index(uint32_t iface_index) noexcept;
  void set_mcast_address(const ip_address &addr) noexcept;
  friend error_code os::socket_impl::get_option(io_handle h, group_option &opt) noexcept;

 public:
  const ip_address &get_iface_address() const noexcept;
  uint32_t get_iface_index() const noexcept;
  const ip_address &get_mcast_address() const noexcept;

 protected:
  ip_address iface_;
  uint32_t iface_index_;
  ip_address mcast_;
};

class reuse_address : public int_option {
 public:
  reuse_address() = default;
  reuse_address(bool val) noexcept;
};

class set_outgoing_interface : public address_option {
 public:
  set_outgoing_interface() = default;
  set_outgoing_interface(const ip_address &addr) noexcept;
};

class join_group : public group_option {
 public:
  join_group() = default;
  join_group(const ip_address &iface, uint32_t iface_index, const ip_address &mcast) noexcept;
};

namespace socket {

error_code set_option(io_handle fd, const reuse_address &opt) noexcept;
error_code set_option(io_handle fd, const set_outgoing_interface &opt) noexcept;
error_code set_option(io_handle fd, const join_group &opt) noexcept;
error_code get_option(io_handle fd, reuse_address &opt) noexcept;
error_code get_option(io_handle fd, set_outgoing_interface &opt) noexcept;
error_code get_option(io_handle fd, join_group &opt) noexcept;

}  // namespace socket

}  // namespace baba::os