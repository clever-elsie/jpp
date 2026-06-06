#pragma once
#include <expected>
#include <optional>
#include <vector>
#include <stdexcept>

namespace json::data_structure{

template<class T>
struct array_t:public std::vector<T>{
    using reference              = std::vector<T>::reference;
    using const_reference        = std::vector<T>::const_reference;
    using iterator               = std::vector<T>::iterator;
    using const_iterator         = std::vector<T>::const_iterator;
    using reverse_iterator       = std::vector<T>::reverse_iterator;
    using const_reverse_iterator = std::vector<T>::const_reverse_iterator;
    using size_type              = std::vector<T>::size_type;
    using difference_type        = std::vector<T>::difference_type;
    using value_type             = std::vector<T>::value_type;
    using allocator_type         = std::vector<T>::allocator_type;
    using pointer                = std::vector<T>::pointer;
    using const_pointer          = std::vector<T>::const_pointer;
    std::expected<reference, std::out_of_range>
    at(size_type n){
        if(n>=this->size())
            return std::unexpected(std::out_of_range("json::data_structure::array_t::at out_of_range"));
        return (*this)[n];
    }
    std::expected<const_reference, std::out_of_range>
    at(size_type n)const{
        if(n>=this->size())
            return std::unexpected(std::out_of_range("json::data_structure::array_t::at out_of_range"));
        return (*this)[n];
    }
    std::optional<reference>
    get(size_t n){
        if(n>=this->size())
            return std::nullopt;
        return (*this)[n];
    }
    std::optional<const_reference>
    get(size_t n)const{
        if(n>=this->size())
            return std::nullopt;
        return (*this)[n];
    }
};

}// namespace json
