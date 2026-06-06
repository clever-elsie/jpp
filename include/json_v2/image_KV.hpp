#pragma once
#include <cstddef>

namespace json::data_structure{

    template<class T>
    struct KV_t{
        size_t key;
        T value;
    };

}// namespace json::data_structure
