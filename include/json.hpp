#pragma once
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <array>
#include <vector>
#include <map>
#include <variant>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

#include "json/internal.hpp"
#include "json/object_t.hpp"

namespace json{

class value{
  public:
  enum class types{ Null, Boolean, Number, String, Array, Object };
  using Array_t=std::vector<value>;
  using Object_t=object_t;
  using json_t=std::variant<nullptr_t, bool, double, std::string, Array_t, Object_t>;
  // constructors
  constexpr value():data_{nullptr_t{}}{}
  constexpr explicit value(types t):data_{}{
    switch(t){
      case types::Null: data_=nullptr_t{}; break;
      case types::Boolean: data_=bool{}; break;
      case types::Number: data_=double{}; break;
      case types::String: data_=std::string{}; break;
      case types::Array: data_=Array_t{}; break;
      case types::Object: data_=Object_t{}; break;
    }
  }
  constexpr value(const value&)=default;
  constexpr value(value&&)=default;
  constexpr value(const json_t&v):data_{v}{}
  constexpr value(json_t&&v):data_{std::move(v)}{}
    // base type
  constexpr value(nullptr_t):data_{nullptr_t{}}{}
  constexpr value(bool v):data_{v}{}
  template<std::floating_point T>
  constexpr value(T v):data_{double(v)}{}
  template<std::integral T>
  constexpr value(T v):data_{double(v)}{}
    // string
  constexpr value(const std::string&v):data_{v}{}
  constexpr value(std::string&&v):data_{std::move(v)}{}
  constexpr value(const char*v):data_{std::string(v)}{}
  constexpr value(const std::string_view&v):data_{std::string(v.data(),v.size())}{}
  template<size_t N>
  value(const char(&v)[N]):data_{std::string(v,v+N)}{
    static_assert(N>0);
    auto &s = std::get<std::string>(data_);
    if(!s.empty() && s.back()=='\0') s.pop_back();
  }
  // UTF-8 u8 string family
  constexpr value(const char8_t* p):data_{std::string(reinterpret_cast<const char*>(p))}{}
  constexpr value(const std::u8string& s):data_{std::string(reinterpret_cast<const char*>(s.data()), s.size())}{}
  constexpr value(const std::u8string_view& sv):data_{std::string(reinterpret_cast<const char*>(sv.data()), sv.size())}{}
    // array
  constexpr value(const Array_t&v):data_{v}{}
  constexpr value(Array_t&&v):data_{std::move(v)}{}
  template<class T>
  constexpr value(std::initializer_list<T>v):data_{std::vector<value>(v.begin(),v.end())}{}
  template<class T,class Alloc>
  requires (std::is_constructible_v<value,T>)
  constexpr value(const std::vector<T,Alloc>&v):data_{std::vector<value>(v.begin(),v.end())}{}
  template<class T,size_t N>
  requires (std::is_constructible_v<value,T>)
  constexpr value(const std::array<T,N>&v):data_{std::vector<value>(v.begin(),v.end())}{}
    // object
  constexpr value(const Object_t&v):data_{v}{}
  constexpr value(Object_t&&v):data_{std::move(v)}{}
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
    if(!is<Object_t>()) throw std::runtime_error("value is not an object");
    std::vector<std::string> keys;
    for(const auto&[key,value]:std::get<Object_t>(data_)) keys.push_back(key);
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
  constexpr nullptr_t null()const{return get<nullptr_t>();}
  constexpr bool boolean()const{ return get<bool>(); }
  constexpr int64_t integer()const{ return int64_t(get<double>()); }
  constexpr double fp()const{ return get<double>(); }
  constexpr double num()const{ return get<double>(); }
  constexpr std::string& str(){ return get<std::string>(); }
  constexpr const std::string& str()const{ return get<std::string>(); }
  Array_t& array(){ return get<Array_t>(); }
  const Array_t& array()const{ return get<Array_t>(); }
  Object_t& object(){ return get<Object_t>(); }
  const Object_t& object()const{ return get<Object_t>(); }
  private:
  json_t data_;
  public:
  private:
  static json_t json_text(const char*&begin, const char* const end){
    internal::token token(get_token(begin,end));
    switch(token.type){
      case internal::token_type::Null: return nullptr_t{};
      case internal::token_type::Boolean: return token.word=="true";
      case internal::token_type::Number: return get_number(token);
      case internal::token_type::String: return token.word;
      case internal::token_type::begin_array: return get_array(begin,end);
      case internal::token_type::begin_object: return get_object(begin,end);
      case internal::token_type::EOF_token: return nullptr_t{};
      default: throw std::runtime_error("invalid json text");
    }
    throw std::runtime_error("invalid json text");
  }
  static Array_t get_array(const char*&begin, const char* const end){
    Array_t res;
    internal::token token(get_token(begin,end));
    while(true){ // 仕様では末尾のカンマは禁止だが，多分許容した方がいいので許容する.
      if(token.type==internal::token_type::end_array) return res;
      switch(token.type){
        case internal::token_type::Null: res.push_back(nullptr_t{}); break;
        case internal::token_type::Boolean: res.push_back(bool(token.word=="true")); break;
        case internal::token_type::Number: res.push_back(get_number(token)); break;
        case internal::token_type::String: res.push_back(token.word); break;
        case internal::token_type::begin_array: res.push_back(Array_t(get_array(begin,end))); break;
        case internal::token_type::begin_object: res.push_back(Object_t(get_object(begin,end))); break;
        case internal::token_type::value_separator:
          throw std::runtime_error("invalid json text: too many ','");
        default: throw std::runtime_error("invalid json text");
      }
      token=get_token(begin,end); // expect ',' or ']'
      if(token.type==internal::token_type::value_separator)
        token=get_token(begin,end); // expect next value (or ']')
      else if(token.type==internal::token_type::end_array)
        return res;
      else throw std::runtime_error("invalid json text: unexpected token");
    }
    throw std::runtime_error("invalid json text: unclosed array");
  }
  static Object_t get_object(const char*&begin, const char* const end){
    Object_t res;
    internal::token token(get_token(begin,end));
    while(true){ // 末尾カンマはget_arrayと同様．
      if(token.type==internal::token_type::end_object) return res;
      if(token.type!=internal::token_type::String)
        throw std::runtime_error("invalid json text: unexpected token. lost of Object key");
      std::string key(std::move(token.word));
      token=get_token(begin,end); // expect ':'
      if(token.type!=internal::token_type::name_separator)
        throw std::runtime_error("invalid json text: unexpected token. lost of ':'");
      res[key]=json_text(begin,end);
      token=get_token(begin,end); // expect ',' or '}'
      if(token.type==internal::token_type::value_separator)
        token=get_token(begin,end); // expect next key:value pair (or '}')
      else if(token.type==internal::token_type::end_object)
        return res;
      else throw std::runtime_error("invalid json text: unexpected token");
    }
    throw std::runtime_error("invalid json text: unclosed object");
  }
  static double get_number(const internal::token&token){
    return std::stod(token.word); // 多分環境依存
  }
  

  static internal::token get_token(const char*&begin, const char* const end){
    while(begin<end && internal::isws(*begin)) ++begin;
    if(begin==end) return {internal::token_type::EOF_token, ""};
    // number
    if(char c=*begin; c=='-' || ('0'<=c && c<='9'))
      return get_number(begin,end);
    // string
    else if(c=='"') return get_string(begin,end);
    // delimiter
    switch(*(begin++)){
      case ',': return {internal::token_type::value_separator, ""}; break;
      case ':': return {internal::token_type::name_separator, ""}; break;
      case '[': return {internal::token_type::begin_array, ""}; break;
      case ']': return {internal::token_type::end_array, ""}; break;
      case '{': return {internal::token_type::begin_object, ""}; break;
      case '}': return {internal::token_type::end_object, ""}; break;
    }
    // boolean or null
    constexpr static std::string_view words[2] = {"true", "false"};
    std::string_view s(--begin,end);
    if(s.starts_with("null")){
      begin+=4;
      return {internal::token_type::Null, "null"};
    }else for(const auto&word:words){
      if(s.starts_with(word)){
        begin+=word.size();
        return {internal::token_type::Boolean, word.data()};
      }
    }
    throw std::runtime_error("invalid token");
  }
  static internal::token get_number(const char*&begin, const char* const end){
    std::string res;
    // minus
    if(begin<end && *begin=='-') res.push_back(*(begin++));
    // int
    for(;begin<end;++begin)
      if('0'<=*begin && *begin<='9') res.push_back(*begin);
      else break;
    // frac
    if(begin<end && *begin=='.'){
      res.push_back(*(begin++));
      for(;begin<end;++begin)
        if('0'<=*begin && *begin<='9') res.push_back(*begin);
        else break;
    }
    // exp
    if(begin<end && (*begin=='e' || *begin=='E')){
      res.push_back(*(begin++));
      if(begin<end && (*begin=='-' || *begin=='+')) res.push_back(*(begin++));
      for(;begin<end;++begin)
        if('0'<=*begin && *begin<='9') res.push_back(*begin);
        else break;
    }
    return internal::token(internal::token_type::Number, std::move(res));
  }
  constexpr static int hexval(char c)noexcept{
    if('0'<=c && c<='9') return c-'0';
    if('a'<=c && c<='f') return 10+(c-'a');
    if('A'<=c && c<='F') return 10+(c-'A');
    return -1;
  }
  constexpr static uint32_t parse4(const char* p)noexcept{
    uint32_t v=0;
    for(int i=0;i<4;++i){
      int h=hexval(p[i]);
      if(h<0) return ~uint32_t(0);
      v=(v<<4)|(uint32_t)h;
    }
    return v;
  }
  constexpr static void append_utf8(std::string&res, uint32_t cp){
    if(cp<=0x7F){
      res.push_back(char(cp));
    }else if(cp<=0x7FF){
      res.push_back(char(0xC0 | ((cp>>6)&0x1F)));
      res.push_back(char(0x80 | (cp & 0x3F)));
    }else if(cp<=0xFFFF){
      res.push_back(char(0xE0 | ((cp>>12)&0x0F)));
      res.push_back(char(0x80 | ((cp>>6)&0x3F)));
      res.push_back(char(0x80 | (cp & 0x3F)));
    }else if(cp<=0x10FFFF){
      res.push_back(char(0xF0 | ((cp>>18)&0x07)));
      res.push_back(char(0x80 | ((cp>>12)&0x3F)));
      res.push_back(char(0x80 | ((cp>>6)&0x3F)));
      res.push_back(char(0x80 | (cp & 0x3F)));
    }else throw std::runtime_error("codepoint out of range");
  };
  static internal::token get_string(const char*&begin, const char* const end){
    std::string res;
    for(++begin;begin<end;++begin)
      if(*begin=='\\'){
        ++begin;
        switch(*begin){
          case '"': res.push_back('"'); break;
          case '/': res.push_back('/'); break;
          case '\\': res.push_back('\\'); break;
          case 'b': res.push_back('\b'); break;
          case 'f': res.push_back('\f'); break;
          case 'n': res.push_back('\n'); break;
          case 'r': res.push_back('\r'); break;
          case 't': res.push_back('\t'); break;
          case 'u': {
            if(begin+5>end) throw std::runtime_error("truncated unicode escape");
            uint32_t u=parse4(begin+1);
            if(u==~uint32_t(0)) throw std::runtime_error("invalid unicode escape");
            begin+=5; // now at the char after uXXXX
            uint32_t codepoint=u;
            if(0xD800<=u && u<=0xDBFF){
              if(!(begin+1<end && *begin== '\\' && begin[1]=='u' && begin+6<=end))
                throw std::runtime_error("invalid surrogate pair");
              ++begin; // at 'u'
              uint32_t u2=parse4(begin+1);
              if(u2==~uint32_t(0)) throw std::runtime_error("invalid unicode escape");
              if(!(0xDC00<=u2 && u2<=0xDFFF)) throw std::runtime_error("invalid surrogate pair");
              begin+=5; // after second uXXXX
              codepoint = 0x10000 + (((u-0xD800)<<10) | (u2-0xDC00));
            }else if(0xDC00<=u && u<=0xDFFF)
              throw std::runtime_error("invalid lone low surrogate");
            append_utf8(res, codepoint);
            --begin; // neutralize for-loop ++begin so next sees current char
          }break;
          default: throw std::runtime_error("invalid escape");
        }
      }else{
        if(*begin=='"') break;
        res.push_back(*begin);
      }
    ++begin; // skip '"'
    return internal::token(internal::token_type::String, std::move(res));
  }
};

// stringify / write APIs
namespace detail{
  inline void write_escaped_string(std::ostream& os, const std::string_view& s){
    os.put('"');
    for(unsigned char uc : s){
      char c = char(uc);
      switch(c){
        case '"': os.put('\\'); os.put('"'); break;
        case '\\': os.put('\\'); os.put('\\'); break;
        case '\b': os.put('\\'); os.put('b'); break;
        case '\f': os.put('\\'); os.put('f'); break;
        case '\n': os.put('\\'); os.put('n'); break;
        case '\r': os.put('\\'); os.put('r'); break;
        case '\t': os.put('\\'); os.put('t'); break;
        default:
          if(uc < 0x20){
            static const char* hex = "0123456789abcdef";
            os.put('\\'); os.put('u');
            os.put('0'); os.put('0');
            os.put(hex[(uc >> 4) & 0xF]);
            os.put(hex[uc & 0xF]);
          }else{
            os.put(c);
          }
      }
    }
    os.put('"');
  }

  inline void write_value(std::ostream& os, const value& v){
    switch(v.type()){
      case value::types::Null:
        os.write("null", 4);
        break;
      case value::types::Boolean:
        if(v.boolean()) os.write("true", 4); else os.write("false", 5);
        break;
      case value::types::Number:
        os << v.num();
        break;
      case value::types::String:
        write_escaped_string(os, v.str());
        break;
      case value::types::Array: {
        os.put('[');
        const auto& arr = v.array();
        for(size_t i=0;i<arr.size();++i){
          if(i) os.put(',');
          write_value(os, arr[i]);
        }
        os.put(']');
      } break;
      case value::types::Object: {
        os.put('{');
        const auto& obj = v.object();
        std::vector<std::string> keys = obj.keys_in_order();
        for(size_t i=0;i<keys.size();++i){
          if(i) os.put(',');
          const std::string& k = keys[i];
          write_escaped_string(os, k);
          os.put(':');
          auto it = obj.find(k);
          write_value(os, it->second);
        }
        os.put('}');
      } break;
    }
  }

  inline void write_indent(std::ostream& os, int indent_level, int indent_size){
    for(int i=0; i<indent_level*indent_size; ++i){
      os.put(' ');
    }
  }

  inline void write_readable_value(std::ostream& os, const value& v, int indent_level, int indent_size){
    switch(v.type()){
      case value::types::Null:
        os.write("null", 4);
        break;
      case value::types::Boolean:
        if(v.boolean()) os.write("true", 4); else os.write("false", 5);
        break;
      case value::types::Number:
        os << v.num();
        break;
      case value::types::String:
        write_escaped_string(os, v.str());
        break;
      case value::types::Array: {
        const auto& arr = v.array();
        if(arr.empty()){
          os.put('[');
          os.put(']');
        }else{
          os.put('[');
          os.put('\n');
          for(size_t i=0;i<arr.size();++i){
            if(i){
              os.put(',');
              os.put('\n');
            }
            write_indent(os, indent_level+1, indent_size);
            write_readable_value(os, arr[i], indent_level+1, indent_size);
          }
          os.put('\n');
          write_indent(os, indent_level, indent_size);
          os.put(']');
        }
      } break;
      case value::types::Object: {
        const auto& obj = v.object();
        if(obj.empty()){
          os.put('{');
          os.put('}');
        }else{
          os.put('{');
          os.put('\n');
          std::vector<std::string> keys = obj.keys_in_order();
          for(size_t i=0;i<keys.size();++i){
            if(i){
              os.put(',');
              os.put('\n');
            }
            write_indent(os, indent_level+1, indent_size);
            const std::string& k = keys[i];
            write_escaped_string(os, k);
            os.put(':');
            os.put(' ');
            auto it = obj.find(k);
            write_readable_value(os, it->second, indent_level+1, indent_size);
          }
          os.put('\n');
          write_indent(os, indent_level, indent_size);
          os.put('}');
        }
      } break;
    }
  }
} // namespace detail

inline void write(std::ostream& os, const value& v){
  detail::write_value(os, v);
}

inline std::string to_string(const value& v){
  std::ostringstream oss;
  write(oss, v);
  return oss.str();
}

inline std::string to_readable(const value& v, int indent_size = 2){
  std::ostringstream oss;
  detail::write_readable_value(oss, v, 0, indent_size);
  return oss.str();
}

inline std::ostream& operator<<(std::ostream& os, const value& v){
  write(os, v);
  return os;
}

} // namespace json

// Include object_t_impl.hpp after value is fully defined
#include "json/object_t_impl.hpp"