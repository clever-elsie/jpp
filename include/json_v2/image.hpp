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
    constexpr static value load(const char*begin, const char* const end){
        return value(json_text(begin,end));
    }
    constexpr static value load(const std::string&s){
        const char*begin=s.data();
        return value(json_text(begin,begin+s.size()));
    }
    constexpr static value load(const std::string_view&s){
        const char*begin=s.data();
        return value(json_text(begin,begin+s.size()));
    }
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
            case types::Array: return std::get<Array_t>(data_).size();
            case types::Object: return std::get<Object_t>(data_).size();
            default: return 1;
        }
    }
    std::vector<std::string> keys()const{
        if(!is<object_t>()) throw std::runtime_error("value is not an object");
        std::vector<std::string> keys;
        for(const auto&[key,value]:std::get<object_t>(data_)) keys.push_back(key);
        return keys;
    }
    // change
    constexpr void clear()noexcept{ data_=nullptr_t{}; }
    constexpr void swap(value&other)noexcept{ std::swap(data_,other.data_); }
    // access
    constexpr value& operator[](size_t index){
        if(!is<Array_t>()) throw std::runtime_error("value is not an array");
        return std::get<Array_t>(data_)[index];
    }
    constexpr const value& operator[](size_t index)const{
        return const_cast<value&>(*this)[index];
    }
    template<internal::is_string_like T>
    constexpr value& operator[](T&&key){
        if(!is<Object_t>()) throw std::runtime_error("value is not an object");
        return std::get<Object_t>(data_)[std::forward<T>(key)];
    }
    template<internal::is_string_like T>
    constexpr const value& operator[](T&&key)const{
        return const_cast<value&>(*this)[std::forward<T>(key)];
    }
    template<class T>
    constexpr T& get(){
        if(!is<T>()) throw std::runtime_error("value is not of type " + std::string(typeid(T).name()));
        return std::get<T>(data_);
    }
    template<class T>
    constexpr const T& get()const{
        return const_cast<value&>(*this).get<T>();
    }
    constexpr nullptr_t null()const{return get<null_t>();}
    constexpr bool boolean()const{ return get<bool_t>(); }
    constexpr int64_t sint()const{ return get<int_t>(); }
    constexpr uint64_t uint()const{ return get<uint_t>(); }
    constexpr double fp()const{ return get<float_t>(); }
    constexpr std::string& str(){ return get<str_t>(); }
    constexpr const std::string& str()const{ return get<str_t>(); }
    Array_t& array(){ return get<array_t>(); }
    const Array_t& array()const{ return get<array_t>(); }
    Object_t& object(){ return get<object_t>(); }
    const Object_t& object()const{ return get<object_t>(); }

    private:
    json_t data_;
};

using null_t   = value::null_t;
using bool_t   = value::bool_t;
using int_t    = value::int_t;
using uint_t   = value::uint_t;
using float_t  = value::float_t;
using str_t    = value::float_t;
using array_t  = value::array_t;
using object_t = value::object_t;
using json_t   = value::json_t;

} // namespace json
