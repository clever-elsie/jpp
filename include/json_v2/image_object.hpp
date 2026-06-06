#pragma once
#include <cstddef>

#include <limits>

#include <optional>
#include <expected>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <ranges>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <ext/pb_ds/tag_and_trait.hpp>

#include "./string_like.hpp"
#include "./image_KV.hpp"
#include "./image_iter.hpp"

namespace json::data_structure{

using namespace __gnu_pbds;

template<class T>
class object_map_t{
    using Self = object_map_t;
    public:
    using mapped_type     = T;
    using key_type        = std::string;
    using value_type      = std::pair<const key_type, mapped_type>;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;
    using hasher          = help::hash;
    using key_equal       = help::equal;
    private:
    using umap_t = std::unordered_map<key_type, KV_t<mapped_type>, hasher, key_equal>;
    using imap_t = tree<size_t, typename umap_t::iterator, std::less<size_t>, rb_tree_tag, tree_order_statistics_node_update>;
    public:
    using allocator_type  = typename umap_t::allocator_type;
    // 以下のデータ構造への参照方法は書き換えが必要
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename umap_t::pointer;
    using const_pointer   = typename umap_t::const_pointer;
    using iterator               = iter<Self, false, false>;
    using const_iterator         = iter<Self, true, false>;
    using reverse_iterator       = iter<Self, false, true>;
    using const_reverse_iterator = iter<Self, true, true>;

    private:
    size_t top_id; // 現在未挿入の最小のid
    umap_t umap; // 実際にデータを保持する
    imap_t imap; // idのマッピングを保持する

    public:
    // constructor
    object_map_t() = default;

    object_map_t(const Self& other)
    : top_id(other.top_id), umap(other.umap), imap(other.imap)
    {}

    object_map_t(Self&& other) noexcept
    : top_id(other.top_id), umap(std::move(other.umap)), imap(std::move(other.imap))
    {}

    template<class InputIt>
    object_map_t(InputIt first, InputIt last) {
        for(; first != last; ++first)
          insert(*first);
    }

    object_map_t(std::initializer_list<value_type> init):object_map_t(){
        for(auto&[key,value] : init)
            (*this)[key] = std::move(value);
    }

    //destructor
    ~object_map_t() = default;

    // Assignment operators
    object_map_t& operator=(const Self& other) {
        if(this == &other)
            return *this;
        top_id = other.top_id;
        umap = other.umap;
        imap = other.imap;
        return *this;
    }

    object_map_t& operator=(Self&& other) noexcept {
        if(this == &other)
            return *this;
        top_id = other.top_id;
        umap = std::move(other.umap);
        imap = std::move(other.imap);
        other.top_id = 0;
        return *this;
    }

    object_map_t& operator=(std::initializer_list<value_type> init){
        return *this = object_map_t(init);
    }

    // Capacity
    bool empty() const noexcept {
        return umap.empty();
    }

    size_type size() const noexcept {
        return umap.size();
    }

    size_type max_size() const noexcept {
        return umap.max_size();
    }

    // Element access
    template<help::string_like S>
    mapped_type& operator[](S&& key){
        auto itr = umap.find(key);
        if(itr == umap.end()){
            auto [jtr, inserted] = umap.emplace(std::forward<S>(key), KV_t(top_id, T()));
            imap.emplace(top_id++, jtr);
            itr = jtr;
        }
        return itr->second.value;
    }

    template<help::string_like S>
    auto at(S&& key) -> std::expected<mapped_type&, std::out_of_range>
    {
        auto itr = umap.find(key);
        if(itr == umap.end())
            return std::unexpected(std::out_of_range("out of range at json::data_structure::object_map_t::at."));
        return itr->second.value;
    }

    template<help::string_like S>
    std::optional<mapped_type&> get(S&& key){
        auto itr = umap.find(key);
        if(itr==umap.end())
            return std::nullopt;
        return itr->second.value;
    }


    // Modifiers
    void clear() noexcept {
        top_id = 0;
        umap.clear();
        imap.clear();
    }

    template<class Value_t>
        requires std::same_as<std::decay_t<Value_t>, value_type>
    std::pair<iterator, bool> insert(Value_t&& value){
        auto [itr, inserted] = emplace(value.first, value.seconds);
        return {iterator(this, imap.order_of_key(itr->second.key)), inserted};
    }

    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        for(; first != last; ++first) {
            insert(*first);
        }
    }

    void insert(std::initializer_list<value_type> init){
        for(auto&x:init){
            insert(std::move(x));
        }
    }

    template<help::string_like S, class... Args>
    std::pair<iterator, bool> emplace(S&&s, Args&&... args){
        auto itr=umap.find(s);
        if(itr==umap.end()){
            auto [itr, inserted] = umap.emplace(std::forward<S>(s), KV_t(top_id, mapped_type(std::forward<Args>(args)...)));
            imap.emplace(top_id++, itr);
            return {iterator(this, imap.order_of_key(itr->second.key)), true};
        }
        itr->second.value = mapped_type(std::forward<Args>(args)...);
        return {iterator(this,imap.order_of_key(itr->second.key)),false};
    }

    template<help::string_like S, class... Args>
    std::pair<iterator, bool> try_emplace(S&&s, Args&&... args){
        auto itr=umap.find(s);
        if(itr==umap.end()){
            auto [itr, inserted] = umap.emplace(std::forward<S>(s), KV_t(top_id, mapped_type(std::forward<Args>(args)...)));
            imap.emplace(top_id++, itr);
            return {iterator(this, imap.order_of_key(itr->second.key)), true};
        }
        return {iterator(this,imap.order_of_key(itr->second.key)),false};
    }

    template<help::string_like KEY_t, class M>
    std::pair<iterator, bool> insert_or_assign(KEY_t&& key, M&& obj){
        return this->emplace(std::forward<KEY_t>(key), std::forward<M>(obj));
    }

    iterator erase(const_iterator pos){
        return this->erase(iterator(this, pos.idx));
    }
    iterator erase(iterator pos){
        auto itr = imap.find(pos);
        if(itr == imap.end()) return this->end();
        umap.erase(itr->second);
        imap.erase(itr);
        if(pos.idx>=umap.size()) return this->end();
        return pos; // 前の要素が消えたのだから次の要素は同じidxを差す自分自身
    }
    template<help::string_like key_t>
    size_type erase(const key_t& key){
        auto itr = umap.find(key);
        if(itr==umap.end()) return 0;
        imap.erase(itr->second.key);
        umap.erase(itr);
        return 1;
    }
    auto erase(const_iterator first, const_iterator last)
    -> std::expected<iterator,
        std::variant<
            std::invalid_argument,
            std::out_of_range
        >
    >
    {
        if(first.idx>last.idx)
            return std::unexpected(std::invalid_argument(
                "json::data_structure::object_map_t::erase(const_iterator,const_iterator) requires first<=last"
            ));
        if(first.idx >= this->size() || last.idx >= this->size())
            return std::unexpected(std::out_of_range(
                "json::data_structure::object_map_t::erase(const_iterator,const_iterator): out of range argument"
            ));
        size_t count = last.idx - first.idx;
        iterator ret;
        for(size_t i:std::ranges::views::iota(size_t{0}, count))
            ret = this->erase(first);
        return ret;
    }

    void swap(Self& other) noexcept {
        std::swap(top_id, other.top_id);
        umap.swap(other.umap);
        imap.swap(other.imap);
    }

    // Lookup
    template<help::string_like S>
    size_type count(const S& key) const {
        return umap.count(key);
    }

    template<help::string_like S>
    iterator find(const S& key) {
        auto itr = umap.find(key);
        if(itr==umap.end()) return this->end();
        return iterator(this, imap.order_of_key(itr->second.key));
    }

    template<help::string_like S>
    const_iterator find(const S& key) const {
        auto itr = umap.find(key);
        if(itr==umap.end()) return this->end();
        return const_iterator(this, imap.order_of_key(itr->second.key));
    }

    template<help::string_like S>
    bool contains(const S& key) const{
        return umap.contains(key);
    }

    template<help::string_like key_t>
    std::pair<iterator, iterator> equal_range(const key_t& key) {
        auto itr = this->find(key);
        if(itr==umap.end()) return this->end();
        iterator it(this, imap.order_of_key(itr->second.key));
        iterator et(this, it.idx>=this->size()?this->end().idx:it.idx+1);
        return {it,et};
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
        auto itr = this->find(key);
        if(itr==umap.end()) return this->end();
        const_iterator it(this, imap.order_of_key(itr->second.key));
        const_iterator et(this, it.idx>=this->size()?this->end().idx:it.idx+1);
        return {it,et};
    }

    // Iterators
    iterator begin() noexcept { return iterator(this,0); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator cbegin() const noexcept { return const_iterator(this,0); }

    iterator end() noexcept { return iterator(this, std::numeric_limits<size_t>::max()); }
    const_iterator end() const noexcept { return cend(); }
    const_iterator cend() const noexcept { return const_iterator(this, std::numeric_limits<size_t>::max()); }

    // Helper function to get keys in insertion order
    std::vector<std::string> keys_in_order() const {
        std::vector<std::string> keys;
        keys.reserve(imap.size());
        for(const auto&[id,itr]:imap)
            keys.push_back(itr->first);
        return keys;
    }
};


} // namespace json::data_structure
