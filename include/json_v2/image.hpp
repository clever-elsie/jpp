#pragma once
#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>

#include <utility>
#include <variant>
#include <expected>

#include <concepts>
#include <type_traits>

#include <stdexcept>
#include <cassert>

namespace json {

template<class T>
class ref_t {
    T* ptr_ = nullptr;
public:
    constexpr ref_t() noexcept = default;
    constexpr ref_t(T& val) noexcept : ptr_(&val) {}
    constexpr ref_t(const ref_t&) noexcept = default;
    constexpr ref_t& operator=(const ref_t&) noexcept = default;

    constexpr T& get() const noexcept { return *ptr_; }
    constexpr operator T&() const noexcept { return *ptr_; }
    constexpr T& operator*() const noexcept { return *ptr_; }
    constexpr T* operator->() const noexcept { return ptr_; }
};

} // namespace json


#include "./image_array.hpp"
#include "./image_object.hpp"

namespace json{

enum class types:uint8_t{
    Null,
    Bool,
    Int, Uint, Float,
    Str,
    Array, Object 
};


class value{
    public:
    // 下にjson名前空間に型を取り込むusingがある.
    using null_t   = std::nullptr_t;
    using bool_t   = bool;
    using int_t    = int64_t;
    using uint_t   = uint64_t;
    using float_t  = double;
    using str_t    = std::string;
    using array_t  = data_structure::array_t<value>;
    using object_t = data_structure::object_map_t<value>;
    using json_t   = std::variant<null_t, bool_t, int_t, uint_t, float_t, str_t, array_t, object_t>;

    // constructors
    constexpr value() = default; // null_t{}
    constexpr value(const value&)=default;
    constexpr value(value&&)=default;

    template<class JSON_T>
        requires std::same_as<std::decay_t<JSON_T>, json_t>
    constexpr value(JSON_T&&v):data_(std::forward<JSON_T>(v)){}

    constexpr explicit value(types t):data_{}{
        switch(t){
            case types::Null:   data_=null_t{}; break;
            case types::Bool:   data_=bool_t{}; break;
            case types::Int:    data_=int_t{}; break;
            case types::Uint:   data_=uint_t{}; break;
            case types::Float:  data_=float_t{}; break;
            case types::Str:    data_=str_t{}; break;
            case types::Array:  data_=array_t{}; break;
            case types::Object: data_=object_t{}; break;
        }
    }
    // base type
    constexpr value(null_t):data_{}{}
    constexpr value(bool v):data_{v}{}
    template<std::floating_point T>
    constexpr value(T v):data_{float_t(v)}{}
    template<std::signed_integral T>
    constexpr value(T v):data_{int_t(v)}{}
    template<std::unsigned_integral T>
    constexpr value(T v):data_{uint_t(v)}{}
    // string
    template<class STR_T>
        requires std::same_as<std::decay_t<STR_T>, str_t>
    constexpr value(STR_T&&v):data_(std::forward<STR_T>(v)){}
    constexpr value(const char*v):data_{str_t(v)}{}
    constexpr value(const std::string_view&v):data_{str_t(v.data(),v.size())}{}
    template<size_t N>
        requires (N>0)
    value(const char(&v)[N]):data_{str_t(v,v+N)}{
        auto &s = std::get<std::string>(data_);
        if(!s.empty() && s.back()=='\0') s.pop_back();
    }
    // UTF-8 u8 string family
    constexpr value(const char8_t* p):data_{str_t(reinterpret_cast<const char*>(p))}{}
    constexpr value(const std::u8string& s):data_{str_t(reinterpret_cast<const char*>(s.data()), s.size())}{}
    constexpr value(const std::u8string_view& sv):data_{str_t(reinterpret_cast<const char*>(sv.data()), sv.size())}{}
    // array
    template<class ARRAY_T>
        requires std::same_as<std::decay_t<ARRAY_T>, array_t>
    constexpr value(ARRAY_T&&v):data_(std::forward<ARRAY_T>(v)){}
    template<class T>
    constexpr value(std::initializer_list<T>v):data_{std::vector<value>(v.begin(),v.end())}{}
    template<class T,class Alloc>
        requires (std::is_constructible_v<value,T>)
    constexpr value(const std::vector<T,Alloc>&v):data_{std::vector<value>(v.begin(),v.end())}{}
    template<class T,size_t N>
        requires (std::is_constructible_v<value,T>)
    constexpr value(const std::array<T,N>&v):data_{std::vector<value>(v.begin(),v.end())}{}
    // object
    template<class OBJ_T>
        requires std::same_as<std::decay_t<OBJ_T>, object_t>
    constexpr value(OBJ_T&&v):data_(std::forward<OBJ_T>(v)){}
    template<class T>
        requires (std::is_constructible_v<value,T>)
    constexpr value(std::initializer_list<std::pair<std::string,T>>v):data_{Object_t(v.begin(),v.end())}{}
    template<class T,class Hash,class Pred,class Alloc>
        requires (std::is_constructible_v<value,T>)
    constexpr value(const std::unordered_map<std::string,T,Hash,Pred,Alloc>&v):data_{Object_t(v.begin(),v.end())}{}
    template<class T,class Compare,class Alloc>
        requires (std::is_constructible_v<value,T>)
        constexpr value(const std::map<std::string,T,Compare,Alloc>& v):data_{Object_t(v.begin(),v.end())}{}
    template<class T,class Alloc>
        requires (std::is_constructible_v<value,T>)
        constexpr value(const std::vector<std::pair<std::string,T>,Alloc>& v):data_{Object_t(v.begin(),v.end())}{}
    // load
    static std::expected<value, std::runtime_error> load(const char*begin, const char* const end);
    static std::expected<value, std::runtime_error> load(const std::string&s);
    static std::expected<value, std::runtime_error> load(const std::string_view&s);
    // destructor
    constexpr ~value()=default;
    // assignment operators
    constexpr value& operator=(const value&)noexcept=default;
    constexpr value& operator=(value&&)noexcept=default;
    template<class T> requires (std::is_assignable_v<json_t,T>)
    constexpr value& operator=(T&&v){
        data_=std::forward<T>(v);
        return *this;
    }
    // information
    constexpr types type()const noexcept{ return types(data_.index()); }
    constexpr bool is(types t)const noexcept{ return type() == t; }
    template<types T>
    constexpr bool is()const noexcept{ return type() == T; }
    template<class T>
    constexpr bool is()const noexcept{ return std::holds_alternative<T>(data_); }
    constexpr size_t size()const noexcept{
        switch(type()){
            case types::Array: return std::get<array_t>(data_).size();
            case types::Object: return std::get<object_t>(data_).size();
            default: return 1;
        }
    }
    std::expected<std::vector<std::string>, std::domain_error> keys()const{
        if(!is<object_t>()) return std::unexpected(std::domain_error("value is not an object"));
        return std::get<object_t>(data_).keys_in_order();
    }
    // change
    constexpr void clear()noexcept{ data_=nullptr_t{}; }
    constexpr void swap(value&other)noexcept{ std::swap(data_,other.data_); }
    // access
    constexpr value& operator[](size_t index){
        if(is<null_t>()) {
            data_ = array_t{};
        }
        assert(is<array_t>());
        auto& arr = std::get<array_t>(data_);
        if(index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }
    constexpr const value& operator[](size_t index)const{
        assert(is<array_t>());
        return std::get<array_t>(data_)[index];
    }
    template<help::string_like T>
    constexpr value& operator[](T&&key){
        if(is<null_t>()) {
            data_ = object_t{};
        }
        assert(is<object_t>());
        return std::get<object_t>(data_)[std::forward<T>(key)];
    }
    template<help::string_like T>
    constexpr const value& operator[](T&&key)const{
        assert(is<object_t>());
        return std::get<object_t>(data_)[std::forward<T>(key)];
    }
    
    // at
    constexpr std::expected<ref_t<value>, std::out_of_range> at(size_t index) {
        if(!is<array_t>()) return std::unexpected(std::out_of_range("value is not array"));
        auto& arr = std::get<array_t>(data_);
        if(index >= arr.size()) return std::unexpected(std::out_of_range("index out of range"));
        return ref_t<value>(arr[index]);
    }
    constexpr std::expected<ref_t<const value>, std::out_of_range> at(size_t index) const {
        if(!is<array_t>()) return std::unexpected(std::out_of_range("value is not array"));
        const auto& arr = std::get<array_t>(data_);
        if(index >= arr.size()) return std::unexpected(std::out_of_range("index out of range"));
        return ref_t<const value>(arr[index]);
    }
    template<help::string_like T>
    constexpr std::expected<ref_t<value>, std::out_of_range> at(T&& key) {
        if(!is<object_t>()) return std::unexpected(std::out_of_range("value is not object"));
        auto& obj = std::get<object_t>(data_);
        auto it = obj.find(key);
        if(it == obj.end()) return std::unexpected(std::out_of_range("key not found"));
        return ref_t<value>(*it);
    }
    template<help::string_like T>
    constexpr std::expected<ref_t<const value>, std::out_of_range> at(T&& key) const {
        if(!is<object_t>()) return std::unexpected(std::out_of_range("value is not object"));
        const auto& obj = std::get<object_t>(data_);
        auto it = obj.find(key);
        if(it == obj.end()) return std::unexpected(std::out_of_range("key not found"));
        return ref_t<const value>(*it);
    }

    template<class T>
    constexpr std::expected<ref_t<T>, std::domain_error> get(){
        if(!is<T>()) return std::unexpected(std::domain_error("value is not of type " + std::string(typeid(T).name())));
        return ref_t<T>(std::get<T>(data_));
    }
    template<class T>
    constexpr std::expected<ref_t<const T>, std::domain_error> get()const{
        if(!is<T>()) return std::unexpected(std::domain_error("value is not of type " + std::string(typeid(T).name())));
        return ref_t<const T>(std::get<T>(data_));
    }

    // 安全なアクセス (expected返却)
    constexpr std::expected<std::nullptr_t, std::domain_error> null() const {
        if (!is<null_t>()) return std::unexpected(std::domain_error("value is not null"));
        return nullptr;
    }
    constexpr std::expected<bool, std::domain_error> boolean() const {
        if (!is<bool_t>()) return std::unexpected(std::domain_error("value is not boolean"));
        return std::get<bool_t>(data_);
    }
    constexpr std::expected<int64_t, std::domain_error> sint() const {
        if (!is<int_t>()) return std::unexpected(std::domain_error("value is not integer"));
        return std::get<int_t>(data_);
    }
    constexpr std::expected<uint64_t, std::domain_error> uint() const {
        if (!is<uint_t>()) return std::unexpected(std::domain_error("value is not unsigned integer"));
        return std::get<uint_t>(data_);
    }
    constexpr std::expected<double, std::domain_error> fp() const {
        if (!is<float_t>()) return std::unexpected(std::domain_error("value is not float"));
        return std::get<float_t>(data_);
    }
    constexpr std::expected<ref_t<std::string>, std::domain_error> str() {
        if (!is<str_t>()) return std::unexpected(std::domain_error("value is not string"));
        return ref_t<std::string>(std::get<str_t>(data_));
    }
    constexpr std::expected<ref_t<const std::string>, std::domain_error> str() const {
        if (!is<str_t>()) return std::unexpected(std::domain_error("value is not string"));
        return ref_t<const std::string>(std::get<str_t>(data_));
    }
    constexpr std::expected<ref_t<array_t>, std::domain_error> array() {
        if (!is<array_t>()) return std::unexpected(std::domain_error("value is not array"));
        return ref_t<array_t>(std::get<array_t>(data_));
    }
    constexpr std::expected<ref_t<const array_t>, std::domain_error> array() const {
        if (!is<array_t>()) return std::unexpected(std::domain_error("value is not array"));
        return ref_t<const array_t>(std::get<array_t>(data_));
    }
    constexpr std::expected<ref_t<object_t>, std::domain_error> object() {
        if (!is<object_t>()) return std::unexpected(std::domain_error("value is not object"));
        return ref_t<object_t>(std::get<object_t>(data_));
    }
    constexpr std::expected<ref_t<const object_t>, std::domain_error> object() const {
        if (!is<object_t>()) return std::unexpected(std::domain_error("value is not object"));
        return ref_t<const object_t>(std::get<object_t>(data_));
    }

    // 契約アクセス (大文字、assert/直接返却)
    constexpr std::nullptr_t Null() const { assert(is<null_t>()); return nullptr; }
    constexpr bool Boolean() const { assert(is<bool_t>()); return std::get<bool_t>(data_); }
    constexpr int64_t Sint() const { assert(is<int_t>()); return std::get<int_t>(data_); }
    constexpr uint64_t Uint() const { assert(is<uint_t>()); return std::get<uint_t>(data_); }
    constexpr double Fp() const { assert(is<float_t>()); return std::get<float_t>(data_); }
    constexpr std::string& Str() { assert(is<str_t>()); return std::get<str_t>(data_); }
    constexpr const std::string& Str() const { assert(is<str_t>()); return std::get<str_t>(data_); }
    constexpr array_t& Array() { assert(is<array_t>()); return std::get<array_t>(data_); }
    constexpr const array_t& Array() const { assert(is<array_t>()); return std::get<array_t>(data_); }
    constexpr object_t& Object() { assert(is<object_t>()); return std::get<object_t>(data_); }
    constexpr const object_t& Object() const { assert(is<object_t>()); return std::get<object_t>(data_); }

    private:
    json_t data_;
};

using null_t   = value::null_t;
using bool_t   = value::bool_t;
using int_t    = value::int_t;
using uint_t   = value::uint_t;
using float_t  = value::float_t;
using str_t    = value::str_t;
using array_t  = value::array_t;
using object_t = value::object_t;
using json_t   = value::json_t;

} // namespace json
