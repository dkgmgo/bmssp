/*
 * A block-based linked list data structure as described in the paper https://arxiv.org/pdf/2504.17033
 */

#ifndef DIJKSTRA_BBL_DS_HPP
#define DIJKSTRA_BBL_DS_HPP

#include <list>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <boost/unordered/unordered_flat_map.hpp>
#include <memory>
#include <boost/sort/spreadsort/spreadsort.hpp>
#define INF 10000000

#include "RBT.hpp"

template <typename Key, typename Value>
struct Block {
    using Item = pair<Key, Value>;
    using LinkedList = vector<Item>; // yeah I know it's not
    enum Location {UNK, D0, D1};

    LinkedList items;
    Value upper_bound{};
    Location location = UNK;

    Block() = default;

    explicit Block(int max_size) {
        items.reserve(max_size);
    }

    Block(Value ub, int max_size) : upper_bound(ub) {
        items.reserve(max_size);
    }

    Block(Value ub, Location loc, int max_size) : upper_bound(ub), location(loc) {
        items.reserve(max_size);
    }

    size_t insert(const Item &p) {
        items.push_back(p);
        return items.size()-1;
    }

    bool empty() {
        return items.empty();
    }

    Value min_value() {
        auto sortie = items.front();
        for (const auto &p : items) {
            if (p.second < sortie.second) {
                sortie = p;
            }
        }
        return sortie.second;
    }
};

template<typename T>
struct is_int_like: integral_constant<bool, is_integral<T>::value>{};

template <typename Key, typename Value>
class BBL_DS {

    using Item = pair<Key, Value>;
    using BlockT = Block<Key, Value>;
    using BlockSeq = list<BlockT>;

    using BlockIt = typename BlockSeq::iterator;

    struct RBData {
        Value upper_bound;
        BlockIt block_it;

        RBData(Value ub, BlockIt it_block): upper_bound(ub), block_it(it_block){}

        RBData() = default;

        RBData(Value ub): upper_bound(ub), block_it(){}

        bool operator<(const RBData &other) const {
            return upper_bound < other.upper_bound;
        }
    };

    struct KeyMap {
        unique_ptr<int[]> sparse;
        vector<Key> dense;
        vector<pair<BlockIt, size_t>> values;

        KeyMap() = default;

        explicit KeyMap(int n) {
            sparse.reset(new int[n]);
            dense.reserve(n/2);
            values.reserve(n/2);
        }

        bool contains(Key key) {
            int idx = sparse[key];
            return idx < dense.size() && dense[idx] == key;
        }

        void update(Key key, pair<BlockIt, size_t> value) {
            int idx = sparse[key];
            values[idx] = value;
        }

        void insert(Key key, pair<BlockIt, size_t> value) {
            sparse[key] = dense.size();
            dense.push_back(key);
            values.push_back(value);
        }

        void erase(Key key) {
            int idx = sparse[key];

            Key &last_key = dense.back();
            sparse[last_key] = idx;
            dense[idx] = last_key;
            values[idx] = values.back();

            dense.pop_back();
            values.pop_back();
        }

        pair<BlockIt, size_t> operator[](Key key) {
            return values[sparse[key]];
        }

        void clear() {
            dense.clear();
            values.clear();
        }
    };

private:
    int M;
    Value B{};
    BlockSeq D0;  // Blocks from batch_prepend
    BlockSeq D1;  // Blocks from regular insertions (TODO: check deque or others)
    RBT<RBData> rbtree_D1; // Red-Black Tree for D1 upper bounds (TODO: check cpp maps and sets)

    KeyMap map;


    void register_block_in_RBT(BlockIt block_it) {
        RBData data{block_it->upper_bound, block_it};
        rbtree_D1.insert(data);
    }

    void unregister_block_in_RBT(BlockIt block_it) {
        RBData data{block_it->upper_bound, block_it};
        rbtree_D1.remove(data);
    }

    BlockIt which_D1_block_for_value(Value value) {
        RBData data{value};
        auto node = rbtree_D1.lower_bound(data);
        if (node == nullptr) {
            return D1.end();
        }

        return node->data.block_it;
    }

    void block_batch_insert(vector<Item> &L, BlockIt block_it, bool update_ub=false) {
        Value ub = Value(-1*INF);
        for (const auto &p : L) {
            size_t idx = block_it->insert(p);
            if (block_it->location == BlockT::Location::D1) {
                // we are batch inserting in D1, so the key is already in the structure
                map.update(p.first, {block_it, idx});
            }else {
                map.insert(p.first, {block_it, idx});
            }
            ub = max(ub, p.second);
        }
        if (update_ub) {
            block_it->upper_bound = ub;
        }
    }

    void split_D1_block(BlockIt block_it) {
        int block_size = M/2 + 1;
        vector<vector<Item>> blocks; blocks.reserve(block_it->items.size()/block_size + 1);
        blocks_content_by_median(blocks, block_it->items, block_size); // 2 blocks

        if (blocks.size() > 2) {
            throw invalid_argument("/!\\ Split D1 block: more than 2 blocks" );
        }

        BlockT new_block(M+1);
        new_block.location = BlockT::Location::D1;
        BlockIt new_block_it = D1.insert(block_it, move(new_block)); // before

        block_it->items.clear();
        block_batch_insert(blocks[0], new_block_it, true);
        block_batch_insert(blocks[1], block_it);

        register_block_in_RBT(new_block_it);

        // the second block with bigger values is already in D1
        unregister_block_in_RBT(block_it);
        register_block_in_RBT(block_it);
    }

    void blocks_content_by_median(vector<vector<Item>> &sortie, vector<Item> &L, int block_size) {
        if (L.empty()) {
            throw invalid_argument("L is not supposed to be empty check comparison or inputs");
        }
        if (static_cast<int>(L.size()) <= block_size) {
            sortie.push_back(L);
            return;
        }

        int mid = static_cast<int>(L.size())/2;
        nth_element(L.begin(), L.begin()+mid, L.end(), [](auto &a, auto &b) {
            return a.second < b.second;
        });
        Value median_val = L[mid].second;
        Key median_key = L[mid].first;
        vector<Item> left_block; left_block.reserve(mid+1);
        vector<Item> right_block; right_block.reserve(mid+2);
        for (const auto &p : L) {
            if (p.second < median_val) {
                left_block.push_back(p);
            } else {
                right_block.push_back(p);
            }
        }

        if (left_block.empty()) {
            left_block.push_back(right_block.front());
            right_block.erase(right_block.begin());
        } else if (right_block.empty()) {
            right_block.push_back(left_block.back());
            left_block.pop_back();
        }

        blocks_content_by_median(sortie, left_block, block_size);
        blocks_content_by_median(sortie, right_block, block_size);
    }

    void delete_pair_from_keymap_by_key(Key key) {
        auto val = map[key];
        BlockIt &block_it = val.first;
        size_t &idx = val.second;

        //delete
        size_t last_idx = block_it->items.size()-1;
        if (idx != last_idx) {
            //swap-pop for O(1), fix keymap after
            swap(block_it->items[idx], block_it->items[last_idx]);
            map.update(block_it->items[idx].first, {block_it, idx});
        }
        block_it->items.pop_back();
        map.erase(key);

        if (block_it->items.empty()) {
            if (block_it->location == BlockT::Location::D0) {
                D0.erase(block_it);
            } else {
                if (block_it->upper_bound != B) {
                    unregister_block_in_RBT(block_it);
                    D1.erase(block_it);
                }
            }
        }
    }

    void fill_buffer_for_pull(vector<Item> &buffer, BlockSeq &sequence) {
        int cpt = 0;
        auto block_it = sequence.begin();
        while (block_it != sequence.end() && cpt < M) {
            for (const auto &p : block_it->items) {
                buffer.push_back(p);
                cpt++;
            }
            ++block_it;
        }
    }

    bool is_block_sequence_empty(BlockSeq &sequence) {
        for (auto &block : sequence) {
            if (!block.empty()) {
                return false;
            }
        }
        return true;
    }

    BlockIt get_D0_block_position(vector<Item> &block_content) {
        Item maxi = block_content.front();
        for (const auto &p : block_content) {
            if (maxi.second < p.second) {
                maxi = p;
            }
        }
        for (auto it = D0.begin(); it != D0.end(); ++it) {
            if (maxi.second < it->upper_bound) {
                return it;
            }
        }
        return D0.end();
    }

    string value_as_string(Value v) {
        ostringstream oss;
        oss << v;
        return oss.str();
    }

public:
    BBL_DS() = default;

    void initialize(int M, Value B, int N) {
        this->D0.clear();
        this->D1.clear();
        this->map = KeyMap(N);
        this->rbtree_D1.clear();

        BlockT b(B, M+1);
        this->D1.emplace_back(b);
        BlockIt block_it = this->D1.begin();
        block_it->location = BlockT::Location::D1;
        register_block_in_RBT(block_it);

        this->M = M;
        this->B = B;
    }

    void delete_pair(const Item &p){
        if (!map.contains(p.first)) {
            //cout << "Delete Pair: Key "<< p.first << " not found." << endl;
            return;
        }

        delete_pair_from_keymap_by_key(p.first);
    }

    void insert_pair(const Item &p) {
        if (map.contains(p.first)) {
            auto val = map[p.first];
            Value &old_v = val.first->items[val.second].second;
            if (p.second < old_v) {
                delete_pair_from_keymap_by_key(p.first);
            } else {
                return;
            }
        }

        BlockIt block_it = which_D1_block_for_value(p.second);
        if (block_it == D1.end()) {
            throw invalid_argument("Insert Pair: No existing block has ub >= value "+value_as_string(p.second));
        }

        size_t idx = block_it->insert(p);
        map.insert(p.first, {block_it, idx});

        if (block_it->items.size() > M) {
            split_D1_block(block_it);
        }
    }

    void batch_prepend(vector<Item> &L) {
        if (L.empty()) {
            return;
        }

        // handle duplicates and existing
        vector<Item> cleaned_L; cleaned_L.reserve(L.size());

        if(is_int_like<Key>::value) {
            boost::sort::spreadsort::integer_sort(L.begin(), L.end(),[](const Item& x, unsigned shift) {
                return static_cast<unsigned>(x.first) >> shift;
            });
        }else {
            sort(L.begin(), L.end(), [](auto &a, auto &b) {
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second;
            });
        }

        for (size_t i = 0; i < L.size(); ) {
            size_t j = i + 1;
            Value best = L[i].second;

            while (j < L.size() && L[j].first == L[i].first) {
                best = min(best, L[j].second);
                ++j;
            }

            cleaned_L.emplace_back(L[i].first, best);
            i = j;
        }
        L.clear(); L.swap(cleaned_L);

        for (const auto &p: L) {
            if (map.contains(p.first)) {
                auto val = map[p.first];
                Value &old_v = val.first->items[val.second].second;
                if (p.second < old_v) {
                    //cout << "Batch Prepend: Key " << p.first << " already exists and deleted." << endl;
                    delete_pair_from_keymap_by_key(p.first);
                }else {
                    continue;
                }
            }
            cleaned_L.push_back(p);
        }

        bool just_push_all = is_block_sequence_empty(D0);
        int block_size = static_cast<int>(cleaned_L.size()) <= M ? M : (M+1)/2;
        vector<vector<Item>> blocks; blocks.reserve(cleaned_L.size()/block_size + 1);
        blocks_content_by_median(blocks, cleaned_L, block_size);

        for (int i = static_cast<int>(blocks.size())-1; i >= 0; --i) {
            BlockT block{};
            BlockIt block_it;
            if (just_push_all) {
                D0.emplace_front(block);
                block_it = D0.begin();
            }else {
                block_it = get_D0_block_position(blocks[i]);
                block_it = D0.insert(block_it, block);
            }
            block_it->location = BlockT::Location::D0;
            block_batch_insert(blocks[i], block_it);
        }
    }

    // collect the M smallest values from union(D0, D1)
    vector<Key> pull(Value &x) {
        vector<Item> buffer; buffer.reserve(4*M);
        vector<Key> keys; keys.reserve(M);

        fill_buffer_for_pull(buffer, D0);
        fill_buffer_for_pull(buffer, D1);

        int n = static_cast<int>(buffer.size());

        if (n <= M) {
            for (const auto &p : buffer) {
                delete_pair_from_keymap_by_key(p.first);
                keys.push_back(p.first);
            }
            if (total_pairs() <= 0) { //if we removed all
                x = B;
            }else {
                Value x0 = !is_block_sequence_empty(D0) ? D0.front().min_value(): Value(INF);
                Value x1 = !is_block_sequence_empty(D1) ? D1.front().min_value() : Value(INF);
                x = min(x0, x1);
            }
            return  keys;
        }

        nth_element(buffer.begin(), buffer.begin()+M, buffer.end(), [](auto &a, auto &b) {
            return a.second < b.second;
        });

        for (int i = 0; i < M; i++) {
            delete_pair_from_keymap_by_key(buffer[i].first);
            keys.push_back(buffer[i].first);
        }
        Value x0 = !is_block_sequence_empty(D0) ? D0.front().min_value(): Value(INF);
        Value x1 = !is_block_sequence_empty(D1) ? D1.front().min_value() : Value(INF);
        x = min(x0, x1);
        return keys;
    }

    int total_pairs() {
        return map.dense.size();
    }

    bool empty() {
        return total_pairs() == 0;
    }

    pair<BlockSeq, BlockSeq> get_sequences() {
        return {D0, D1};
    }

    bool contains(Key key) {
        return map.contains(key);
    }
};

#endif