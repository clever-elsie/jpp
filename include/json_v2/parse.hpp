#pragma once
#include "./image.hpp"
#include "./string_like.hpp"
#include <string>
#include <utility>
#include <expected>
#include <variant>

namespace json::parse{

enum class token_type{
    EOF_token,
    begin_array, begin_object, end_array, end_object,
    name_separator, value_separator,
    Null, Boolean, Number, String,
};

struct token{
    token_type type;
    std::string word;
    token()=defualt;
    template<help::string_like STR>
    token(token_type type, STR&&word):type(type), word(std::forward<STR>(word)) {}
};

constexpr inline bool isws(char c)noexcept{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
constexpr inline int hexval(char c)noexcept{
    if('0'<=c && c<='9') return c-'0';
    if('a'<=c && c<='f') return 10+(c-'a');
    if('A'<=c && c<='F') return 10+(c-'A');
    return -1;
}
constexpr inline uint32_t parse4(const char* p)noexcept{
    uint32_t v=0;
    for(int i=0;i<4;++i){
        int h=hexval(p[i]);
        if(h<0) return ~uint32_t(0);
        v=(v<<4)|(uint32_t)h;
    }
    return v;
}

std::variant<int_t, uint_t, float_t> parse_number(const token&token){
    // 小数点が含まれるならfloat_t
    // e,Eがあっても整数型に収まるなら整数にしたい
    return std::stod(token.word); // 多分環境依存
}


struct parser{

static token get_number(const char*&begin, const char* const end){
    std::string res;
    // minus
    if(begin<end && *begin=='-') res.push_back(*(begin++));
    // int
    for(;begin<end;++begin)
        if('0'<=*begin && *begin<='9') res.push_back(*begin);
        else break;
    // float
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
    return token(token_type::Number, std::move(res));
}

static bool append_utf8(std::string&res, uint32_t cp){
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
    }else return false; // codepoint is out of domain
    return true;
}

static std::expected<token,std::runtime_error> get_string(const char*&begin, const char* const end){
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
                    if(begin+5>end) return std::unexpected(std::runtime_error("truncated unicode escape"));
                    uint32_t u=parse4(begin+1);
                    if(u==~uint32_t(0)) return std::unexpected(std::runtime_error("invalid unicode escape"));
                    begin+=5; // now at the char after uXXXX
                    uint32_t codepoint=u;
                    if(0xD800<=u && u<=0xDBFF){
                        if(!(begin+1<end && *begin== '\\' && begin[1]=='u' && begin+6<=end))
                            return std::unexpected(std::runtime_error("invalid surrogate pair"));
                        ++begin; // at 'u'
                        uint32_t u2=parse4(begin+1);
                        if(u2==~uint32_t(0)) return std::unexpected(std::runtime_error("invalid unicode escape"));
                        if(!(0xDC00<=u2 && u2<=0xDFFF)) return std::unexpected(std::runtime_error("invalid surrogate pair"));
                        begin+=5; // after second uXXXX
                        codepoint = 0x10000 + (((u-0xD800)<<10) | (u2-0xDC00));
                    }else if(0xDC00<=u && u<=0xDFFF)
                        return std::unexpected(std::runtime_error("invalid lone low surrogate"));
                    append_utf8(res, codepoint);
                    --begin; // neutralize for-loop ++begin so next sees current char
                }break;
                default: return std::unexpected(std::runtime_error("invalid escape"));
            }
        }else{
            if(*begin=='"') break;
            res.push_back(*begin);
        }
    ++begin; // skip '"'
    return token(token_type::String, std::move(res));
}

static token get_token(const char*&begin, const char* const end){
    while(begin<end && isws(*begin)) ++begin;
    if(begin==end) return {};
    // number
    if(char c=*begin; c=='-' || ('0'<=c && c<='9'))
        return get_number(begin,end);
    // string
    else if(c=='"') return get_string(begin,end);
    // delimiter
    switch(*(begin++)){
        case ',': return {token_type::value_separator, ""}; break;
        case ':': return {token_type::name_separator, ""}; break;
        case '[': return {token_type::begin_array, ""}; break;
        case ']': return {token_type::end_array, ""}; break;
        case '{': return {token_type::begin_object, ""}; break;
        case '}': return {token_type::end_object, ""}; break;
    }
    // boolean or null
    constexpr static std::string_view words[2] = {"true", "false"};
    std::string_view s(--begin,end);
    if(s.starts_with("null")){
        begin+=4;
        return {token_type::Null, "null"};
    }else for(const auto&word:words){
        if(s.starts_with(word)){
            begin+=word.size();
            return {token_type::Boolean, word.data()};
        }
    }
    return std::unexpected(std::runtime_error("invalid token"));
}

static std::expected<array_t,std::runtime_error> get_array(const char*&begin, const char* const end){
    array_t res;
    token token(get_token(begin,end));
    while(true){ // 仕様では末尾のカンマは禁止だが，多分許容した方がいいので許容する.
        if(token.type==token_type::end_array) return res;
        switch(token.type){
            case token_type::Null: res.push_back(null_t{}); break;
            case token_type::Boolean: res.push_back(bool_t(token.word=="true")); break;
            case token_type::Number:{
                const auto v = parse_number(token);
                std::visit([&res](const auto&v){
                    res.push_back(v);
                }, v);
            }break;
            case token_type::String: res.push_back(token.word); break;
            case token_type::begin_array:{
                auto arr = get_array(begin,end);
                if(arr) res.push_back(std::move(*arr));
                else return std::unexpected(arr.error());
            }break;
            case token_type::begin_object:{
                auto obj = get_object(begin,end);
                if(obj) res.push_back(std::move(*obj));
                else return std::unexpected(obj.error());
            }break;
            case token_type::value_separator:
                return std::unexpected(std::runtime_error("invalid json text: too many ','"));
            default: return std::unexpected(std::runtime_error("invalid json text"));
        }
        token=get_token(begin,end); // expect ',' or ']'
        if(token.type==token_type::value_separator)
            token=get_token(begin,end); // expect next value (or ']')
        else if(token.type==token_type::end_array)
            return res;
        else return std::unexpected(std::runtime_error("invalid json text: unexpected token"));
    }
    return std::unexpected(std::runtime_error("invalid json text: unclosed array"));
}

static std::expected<object_t, std::runtime_error> get_object(const char*&begin, const char* const end){
    object_t res;
    token token(get_token(begin,end));
    while(true){ // 末尾カンマはget_arrayと同様．
        if(token.type==token_type::end_object) return res;
        if(token.type!=token_type::String)
            return std::unexpected(std::runtime_error("invalid json text: unexpected token. lost of Object key"));
        std::string key(std::move(token.word));
        token=get_token(begin,end); // expect ':'
        if(token.type!=token_type::name_separator)
            return std::unexpected(std::runtime_error("invalid json text: unexpected token. lost of ':'"));
        res[key]=json_text(begin,end);
        token=get_token(begin,end); // expect ',' or '}'
        if(token.type==token_type::value_separator)
            token=get_token(begin,end); // expect next key:value pair (or '}')
        else if(token.type==token_type::end_object)
            return res;
        else return std::unexpected(std::runtime_error("invalid json text: unexpected token"));
    }
    return std::unexpected(std::runtime_error("invalid json text: unclosed object"));
}


static std::expected<json_t, std::runtime_error> json_text(const char*&begin, const char* const end){
    token token(get_token(begin,end));
    switch(token.type){
        case token_type::Null: return null_t{};
        case token_type::Boolean: return token.word=="true";
        case token_type::Number:{
            auto v = parse_number(token);
            json_t r;
            std::visit([&r](const auto&v){
                r = v;
            }, v);
            return r;
        } break;
        case token_type::String: return token.word;
        case token_type::begin_array: return get_array(begin,end);
        case token_type::begin_object: return get_object(begin,end);
        case token_type::EOF_token: return null_t{};
        default: return std::unexpected(std::runtime_error("invalid json text"));
    }
    return std::unexpected(std::runtime_error("invalid json text"));
}

}

} // namespace json::parse
