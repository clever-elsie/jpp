#pragma once
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "json/internal.hpp"

namespace json{

// Forward declaration - value will be fully defined in json.hpp
class value;

class object_t{
  public:
  using mapped_type = value;
  using key_type = std::string;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = internal::hash;
  using key_equal = internal::equal;
  using allocator_type = typename internal::umap<mapped_type>::allocator_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename internal::umap<mapped_type>::pointer;
  using const_pointer = typename internal::umap<mapped_type>::const_pointer;

  // Iterators
  using iterator = typename internal::umap<mapped_type>::iterator;
  using const_iterator = typename internal::umap<mapped_type>::const_iterator;

  private:
  size_t top_id_ = 0;
  internal::umap<mapped_type> obj_;
  std::unordered_map<std::string, size_t> id_map_;

  public:
  // Constructors
  object_t() = default;
  
  object_t(const object_t& other)
    : top_id_(other.top_id_), obj_(other.obj_), id_map_(other.id_map_) {}
  
  object_t(object_t&& other) noexcept
    : top_id_(other.top_id_), obj_(std::move(other.obj_)), id_map_(std::move(other.id_map_)) {
    other.top_id_ = 0;
  }

  template<class InputIt>
  object_t(InputIt first, InputIt last) {
    for(; first != last; ++first) {
      insert(*first);
    }
  }

  // Note: This constructor is defined inline in json.hpp after value is fully defined
  // to avoid incomplete type issues with std::pair<const std::string, value>
  object_t(std::initializer_list<value_type> init);

  // Destructor
  ~object_t() = default;

  // Assignment operators
  object_t& operator=(const object_t& other) {
    if(this != &other) {
      top_id_ = other.top_id_;
      obj_ = other.obj_;
      id_map_ = other.id_map_;
    }
    return *this;
  }

  object_t& operator=(object_t&& other) noexcept {
    if(this != &other) {
      top_id_ = other.top_id_;
      obj_ = std::move(other.obj_);
      id_map_ = std::move(other.id_map_);
      other.top_id_ = 0;
    }
    return *this;
  }

  // Note: This operator is defined inline in json.hpp after value is fully defined
  object_t& operator=(std::initializer_list<value_type> init);

  // Capacity
  bool empty() const noexcept {
    return obj_.empty();
  }

  size_type size() const noexcept {
    return obj_.size();
  }

  size_type max_size() const noexcept {
    return obj_.max_size();
  }

  // Element access
  // Note: These functions are defined inline in json.hpp after value is fully defined
  mapped_type& operator[](const key_type& key);
  mapped_type& operator[](key_type&& key);
  template<internal::is_string_like T>
  mapped_type& operator[](T&& key);
  mapped_type& at(const key_type& key);
  const mapped_type& at(const key_type& key) const;

  // Modifiers
  void clear() noexcept {
    obj_.clear();
    id_map_.clear();
    top_id_ = 0;
  }

  // Note: These functions are defined inline in json.hpp after value is fully defined
  std::pair<iterator, bool> insert(const value_type& value);
  template<class P>
  std::pair<iterator, bool> insert(P&& value);

  template<class InputIt>
  void insert(InputIt first, InputIt last) {
    for(; first != last; ++first) {
      insert(*first);
    }
  }

  // Note: This function is defined inline in json.hpp after value is fully defined
  void insert(std::initializer_list<value_type> init);

  // Note: These functions are defined inline in json.hpp after value is fully defined
  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args);
  template<class M>
  std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& obj);
  template<class M>
  std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& obj);
  template<class... Args>
  std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args);
  template<class... Args>
  std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args);

  // Note: These functions are defined inline in json.hpp after value is fully defined
  iterator erase(const_iterator pos);
  iterator erase(iterator pos);
  size_type erase(const key_type& key);
  iterator erase(const_iterator first, const_iterator last);

  void swap(object_t& other) noexcept {
    std::swap(top_id_, other.top_id_);
    obj_.swap(other.obj_);
    id_map_.swap(other.id_map_);
  }

  // Lookup
  size_type count(const key_type& key) const {
    return obj_.count(key);
  }

  template<internal::is_string_like T>
  size_type count(const T& key) const {
    return obj_.count(key);
  }

  iterator find(const key_type& key) {
    return obj_.find(key);
  }

  const_iterator find(const key_type& key) const {
    return obj_.find(key);
  }

  template<internal::is_string_like T>
  iterator find(const T& key) {
    return obj_.find(key);
  }

  template<internal::is_string_like T>
  const_iterator find(const T& key) const {
    return obj_.find(key);
  }

  // Note: These functions are defined inline in json.hpp after value is fully defined
  bool contains(const key_type& key) const;
  template<internal::is_string_like T>
  bool contains(const T& key) const;

  std::pair<iterator, iterator> equal_range(const key_type& key) {
    return obj_.equal_range(key);
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
    return obj_.equal_range(key);
  }

  // Iterators
  iterator begin() noexcept {
    return obj_.begin();
  }

  const_iterator begin() const noexcept {
    return obj_.begin();
  }

  const_iterator cbegin() const noexcept {
    return obj_.cbegin();
  }

  iterator end() noexcept {
    return obj_.end();
  }

  const_iterator end() const noexcept {
    return obj_.end();
  }

  const_iterator cend() const noexcept {
    return obj_.cend();
  }

  // Helper function to get keys in insertion order
  std::vector<std::string> keys_in_order() const {
    std::vector<std::pair<size_t, std::string>> ordered;
    ordered.reserve(id_map_.size());
    for(const auto& [key, id] : id_map_) {
      ordered.emplace_back(id, key);
    }
    std::sort(ordered.begin(), ordered.end(), 
              [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<std::string> result;
    result.reserve(ordered.size());
    for(const auto& [id, key] : ordered) {
      result.push_back(key);
    }
    return result;
  }

  // Access to internal umap (for compatibility)
  internal::umap<mapped_type>& get_obj() noexcept {
    return obj_;
  }

  const internal::umap<mapped_type>& get_obj() const noexcept {
    return obj_;
  }
};

} // namespace json

