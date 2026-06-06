#include "./image.hpp"
#include <iostream>
#include <sstream>

namespace json{

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
        case types::Null: os.write("null", 4); break;
        case types::Bool:
            if(v.boolean()) os.write("true", 4); else os.write("false", 5);
            break;
        case types::Int: os << v.sint(); break;
        case types::Uint: os << v.uint(); break;
        case types::Float: os << v.fp(); break;
        case types::Str:
            write_escaped_string(os, v.str());
            break;
        case types::Array: {
            os.put('[');
            const auto& arr = v.array();
            for(size_t i=0;i<arr.size();++i){
                if(i) os.put(',');
                write_value(os, arr[i]);
            }
            os.put(']');
        } break;
        case types::Object: {
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
        case types::Null:
            os.write("null", 4);
            break;
        case types::Bool:
            if(v.boolean()) os.write("true", 4); else os.write("false", 5);
            break;
        case types::Int: os << v.sint(); break;
        case types::Uint: os << v.uint(); break;
        case types::Float: os << v.fp(); break;
        case types::Str:
            write_escaped_string(os, v.str());
            break;
        case types::Array: {
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
        case types::Object: {
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
}
