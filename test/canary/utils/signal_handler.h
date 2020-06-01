/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include <csignal>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "baba/logger.h"

namespace utils {

class signal_mgr {
 public:
  using signal_handler_fn = std::function<void()>;

  inline static void add_action(int sig, signal_handler_fn hdlr) {
    auto it = _action_map.find(sig);
    if (it != end(_action_map)) {
      it->second.push_back(std::move(hdlr));
    } else {
      _action_map.emplace(sig, std::initializer_list<signal_handler_fn>{std::move(hdlr)});
    }

    std::signal(sig, sig_handler);
  }

 private:
  inline static void sig_handler(int sig) {
    LOGINF("caught signal {}", sig);
    auto it = _action_map.find(sig);
    if (it != end(_action_map)) {
      const auto &hdlrs = it->second;
      for (const auto &hdlr : hdlrs) {
        hdlr();
      }
    }
  }

  static std::unordered_map<int, std::vector<signal_handler_fn>> _action_map;
};

std::unordered_map<int, std::vector<signal_mgr::signal_handler_fn>> signal_mgr::_action_map;

}  // namespace utils