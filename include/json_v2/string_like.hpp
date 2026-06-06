#pragma once
#include <string>
#include <string_view>

#include <concepts>
#include <type_traits>

#include <functional>

namespace json::help{

    template<class T>
    using pure = std::decay_t<T>;

    template<class T>
    concept string_like = std::same_as<pure<T>, std::string>
                        ||std::same_as<pure<T>, std::string_view>
                        ||std::same_as<pure<T>, const char*>;

    struct hash{
      using is_transparent = void;
      template<string_like T>
      constexpr static size_t operator()(T&&v)noexcept{
          return std::hash<std::decay_t<T>>{}(v);
      }
    };

    struct equal{
      using is_transparent = void;
      template<string_like S, string_like T>
      constexpr static bool operator()(S&&v1, T&&v2)noexcept{
          return std::string_view(v1) == std::string_view(v2);
      }
    };
} // namespace json::help
