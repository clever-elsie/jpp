#pragma once
#include <cstddef>
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <type_traits>

namespace json::data_structure{

// T.imap.find_by_order(size_t)->second->second.valueがアクセスの戻り値型となる契約
template<class T, bool is_const, bool is_reverse>
struct iter{
    using difference_type = std::ptrdiff_t;
    iter()=delete;
    iter(const iter&other)=default;
    iter(iter&&other)=default;
    iter(T*ptr, size_t idx)requires(!is_const):ptr(ptr), idx(idx) {}
    iter(T const*ptr, size_t idx)requires(is_const):ptr(ptr), idx(idx) {}
    iter& operator=(const iter&other)=default;
    iter& operator=(iter&&other)=default;
    auto& operator*(){
        return ptr->imap.find_by_order(idx)->second->second.value;
    }
    const auto& operator*()const{
        return ptr->imap.find_by_order(idx)->second->second.value;
    }
    auto operator->(){
        return &(ptr->imap.find_by_order(idx)->second->second.value);
    }
    const auto operator->()const{
        return &(ptr->imap.find_by_order(idx)->second->second.value);
    }
    iter& operator++(){
        ++idx;
        return*this;
    }
    iter  operator++(int){
        iter r(*this);
        ++idx;
        return r;
    }
    iter& operator--(){
        --idx;
        return*this;
    }
    iter  operator--(int){
        iter r(*this);
        --idx;
        return r;
    }
    iter& operator+=(difference_type n){
        idx+=n;
        return*this;
    }
    iter& operator-=(difference_type n){
        idx-=n;
        return*this;
    }
    iter  operator+(difference_type n)const{
        iter r(*this);
        return r+=n;
    }
    iter  operator-(difference_type n)const{
        iter r(*this);
        return r-=n;
    }
    template<bool is_const1>
    difference_type operator-(const iter<T,is_const1,is_reverse>&itr)const{
        return idx-itr.idx;
    }
    template<bool is_const1, bool is_reverse1>
    bool operator==(const iter<T,is_const1,is_reverse1>&itr)const{
        return ptr==itr.ptr && idx==itr.idx;
    }
    template<bool is_const1, bool is_reverse1>
    auto operator<=>(const iter<T,is_const1,is_reverse1>&itr)const{
        assert(ptr==itr.ptr && "iter is not belonging to the same object_map_t");
        const size_t i1 = is_reverse?ptr->size()-idx : idx;
        const size_t i2 = is_reverse1?itr.ptr->size()-itr.idx : itr.idx;
        return i1<=>i2;
    }
    private:
    using pp_t = std::conditional_t<is_const, const T*, T*>;
    size_t idx;
    pp_t ptr;
    template<class U>
    friend class object_map_t;
};

} // namespace json::data_structure
