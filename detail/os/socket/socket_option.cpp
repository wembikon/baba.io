/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/socket/socket_option.h"

#if defined(PLATFORM_WINDOWS)
#include "os/impl/wsa_socket_impl.h"
#else
#include "os/impl/bsd_socket_impl.h"
#endif

namespace baba::os {

option::option() noexcept : level_(0), optname_(0) {}
option::option(int level, int optname) noexcept : level_(level), optname_(optname) {}
int option::get_level() const noexcept { return level_; }
int option::get_name() const noexcept { return optname_; }

int_option::int_option() noexcept : optval_(0) {}
int_option::int_option(int level, int optname, int val) noexcept
    : option(level, optname), optval_(val) {}
void int_option::set_value(int val) noexcept { optval_ = val; }
int int_option::get_value() const noexcept { return optval_; }

address_option::address_option() noexcept {}
address_option::address_option(int level, int optname, const ip_address &addr) noexcept
    : option(level, optname), addr_(addr) {}
const ip_address &address_option::get_address() const noexcept { return addr_; }
void address_option::set_address(const ip_address &addr) noexcept { addr_ = addr; }

group_option::group_option() noexcept : iface_index_(0) {}
group_option::group_option(int level, int optname, const ip_address &iface, uint32_t iface_index,
                           const ip_address &mcast) noexcept
    : option(level, optname), iface_(iface), iface_index_(iface_index), mcast_(mcast) {}
void group_option::set_iface_address(const ip_address &addr) noexcept { iface_ = addr; }
void group_option::set_iface_index(uint32_t iface_index) noexcept { iface_index_ = iface_index; }
void group_option::set_mcast_address(const ip_address &addr) noexcept { mcast_ = addr; }
const ip_address &group_option::get_iface_address() const noexcept { return iface_; }
uint32_t group_option::get_iface_index() const noexcept { return iface_index_; }
const ip_address &group_option::get_mcast_address() const noexcept { return mcast_; }

reuse_address::reuse_address(bool val) noexcept : int_option(SOL_SOCKET, SO_REUSEADDR, val) {}

set_outgoing_interface::set_outgoing_interface(const ip_address &addr) noexcept
    : address_option(IPPROTO_IP, IP_MULTICAST_IF, addr) {}

join_group::join_group(const ip_address &iface, uint32_t iface_index,
                       const ip_address &mcast) noexcept
    : group_option(IPPROTO_IP, IP_ADD_MEMBERSHIP, iface, iface_index, mcast) {}

namespace socket {

error_code set_option(io_handle fd, const reuse_address &opt) noexcept {
  return socket_impl::set_option(fd, opt);
}

error_code set_option(io_handle fd, const set_outgoing_interface &opt) noexcept {
  return socket_impl::set_option(fd, opt);
}

error_code set_option(io_handle fd, const join_group &opt) noexcept {
  return socket_impl::set_option(fd, opt);
}

error_code get_option(io_handle fd, reuse_address &opt) noexcept {
  return socket_impl::get_option(fd, opt);
}

error_code get_option(io_handle fd, set_outgoing_interface &opt) noexcept {
  return socket_impl::get_option(fd, opt);
}

error_code get_option(io_handle fd, join_group &opt) noexcept {
  return socket_impl::get_option(fd, opt);
}

}  // namespace socket

}  // namespace baba::os