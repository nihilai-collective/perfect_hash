#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Opaque wrapper around jsonifier::hash_map to isolate its headers from other
// translation units. A map's keys are compile-time, so the keyset is selected
// by the name passed to run_keyset / verify_keyset.
class JsonifierWrapper {
public:
  using LookupFn = std::optional<int> (*)(std::string_view);
  using BenchFn = void (*)(const std::vector<std::string_view> &, std::vector<int> &);

  explicit JsonifierWrapper(const std::string &keyset_name);

  std::optional<int> lookup(std::string_view key) const { return lookup_(key); }

  // Runs the hot lookup loop internally so jsonifier calls can be inlined
  // within this translation unit.
  void bench_lookup(const std::vector<std::string_view> &input,
                    std::vector<int> &results) const {
    bench_(input, results);
  }

private:
  LookupFn lookup_;
  BenchFn bench_;
};
