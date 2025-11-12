#pragma once
#include "json/object_t.hpp"

// Implement object_t members that need value's complete definition
namespace json{

inline object_t::object_t(std::initializer_list<value_type> init) {
  for(const auto& pair : init) {
    insert(pair);
  }
}

inline object_t& object_t::operator=(std::initializer_list<value_type> init) {
  clear();
  for(const auto& pair : init) {
    insert(pair);
  }
  return *this;
}

inline void object_t::insert(std::initializer_list<value_type> init) {
  for(const auto& pair : init) {
    insert(pair);
  }
}

// Element access
inline object_t::mapped_type& object_t::operator[](const key_type& key) {
  auto it = obj_.find(key);
  if(it == obj_.end()) {
    id_map_[key] = top_id_++;
    return obj_[key];
  }
  return it->second;
}

inline object_t::mapped_type& object_t::operator[](key_type&& key) {
  auto it = obj_.find(key);
  if(it == obj_.end()) {
    std::string key_copy = key;
    id_map_[key_copy] = top_id_++;
    return obj_[std::move(key)];
  }
  return it->second;
}

template<internal::is_string_like T>
inline object_t::mapped_type& object_t::operator[](T&& key) {
  std::string key_str(key);
  auto it = obj_.find(key_str);
  if(it == obj_.end()) {
    id_map_[key_str] = top_id_++;
    return obj_[std::forward<T>(key)];
  } else {
    return it->second;
  }
}

inline object_t::mapped_type& object_t::at(const key_type& key) {
  return obj_.at(key);
}

inline const object_t::mapped_type& object_t::at(const key_type& key) const {
  return obj_.at(key);
}

// Modifiers
inline std::pair<object_t::iterator, bool> object_t::insert(const value_type& value) {
  auto [it, inserted] = obj_.insert(value);
  if(inserted) {
    id_map_[value.first] = top_id_++;
  }
  return {it, inserted};
}

template<class P>
inline std::pair<object_t::iterator, bool> object_t::insert(P&& value) {
  auto [it, inserted] = obj_.insert(std::forward<P>(value));
  if(inserted) {
    id_map_[it->first] = top_id_++;
  }
  return {it, inserted};
}

template<class... Args>
inline std::pair<object_t::iterator, bool> object_t::emplace(Args&&... args) {
  value_type pair(std::forward<Args>(args)...);
  auto [it, inserted] = obj_.emplace(std::move(pair));
  if(inserted) {
    id_map_[it->first] = top_id_++;
  }
  return {it, inserted};
}

template<class M>
inline std::pair<object_t::iterator, bool> object_t::insert_or_assign(const key_type& key, M&& obj) {
  auto [it, inserted] = obj_.insert_or_assign(key, std::forward<M>(obj));
  if(inserted) {
    id_map_[key] = top_id_++;
  }
  return {it, inserted};
}

template<class M>
inline std::pair<object_t::iterator, bool> object_t::insert_or_assign(key_type&& key, M&& obj) {
  std::string key_copy = key;
  auto [it, inserted] = obj_.insert_or_assign(std::move(key), std::forward<M>(obj));
  if(inserted) {
    id_map_[key_copy] = top_id_++;
  }
  return {it, inserted};
}

template<class... Args>
inline std::pair<object_t::iterator, bool> object_t::try_emplace(const key_type& key, Args&&... args) {
  auto [it, inserted] = obj_.try_emplace(key, std::forward<Args>(args)...);
  if(inserted) {
    id_map_[key] = top_id_++;
  }
  return {it, inserted};
}

template<class... Args>
inline std::pair<object_t::iterator, bool> object_t::try_emplace(key_type&& key, Args&&... args) {
  std::string key_copy = key;
  auto [it, inserted] = obj_.try_emplace(std::move(key), std::forward<Args>(args)...);
  if(inserted) {
    id_map_[key_copy] = top_id_++;
  }
  return {it, inserted};
}

// Erase functions
inline object_t::iterator object_t::erase(const_iterator pos) {
  if(pos != end()) {
    id_map_.erase(pos->first);
  }
  return obj_.erase(pos);
}

inline object_t::iterator object_t::erase(iterator pos) {
  if(pos != end()) {
    id_map_.erase(pos->first);
  }
  return obj_.erase(pos);
}

inline object_t::size_type object_t::erase(const key_type& key) {
  auto it = obj_.find(key);
  if(it != obj_.end()) {
    id_map_.erase(key);
    obj_.erase(it);
    return 1;
  }
  return 0;
}

inline object_t::iterator object_t::erase(const_iterator first, const_iterator last) {
  for(auto it = first; it != last; ++it) {
    id_map_.erase(it->first);
  }
  return obj_.erase(first, last);
}

// Lookup functions
inline bool object_t::contains(const key_type& key) const {
  return obj_.find(key) != obj_.end();
}

template<internal::is_string_like T>
inline bool object_t::contains(const T& key) const {
  return obj_.find(key) != obj_.end();
}

} // namespace json

