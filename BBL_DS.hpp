/*
 * A block-based linked list data structure as described in the paper https://arxiv.org/pdf/2504.17033
 */

#ifndef DIJKSTRA_BBL_DS_HPP
#define DIJKSTRA_BBL_DS_HPP

#include <list>
#include <unordered_map>
#include <vector>
#include <sstream>
#define INF 10000000

#include "RBT.hpp"

template <typename Key, typename Value>
struct Block {
    using Item = pair<Key, Value>;
    using LinkedList = list<Item>;
    enum Location {UNK, D0, D1};

    LinkedList items;
    Value upper_bound{};
    Location location = UNK;


    Block() = default;

    explicit Block(Value upper_bound) {
        this->upper_bound = upper_bound;
    }

    typename LinkedList::iterator insert(const Item &p) {
        items.push_back(p);
        return prev(items.end());
    }

    void remove(typename LinkedList::iterator it) {
        items.erase(it);
    }

    bool empty() {
        return items.empty();
    }

    Value min_value() {
        auto sortie = items.front();
        for (auto p : items) {
            if (p.second < sortie.second) {
                sortie = p;
            }
        }
        return sortie.second;
    }
};

template<typename Key, typename Value>
struct RBData {
    Value upper_bound;
    Block<Key, Value> *block_ptr;

    RBData(Value ub, Block<Key, Value> *block) {
        this->upper_bound = ub;
        this->block_ptr = block;
    }

    RBData(): upper_bound(), block_ptr(nullptr){}

    bool operator<(const RBData &other) const {
        return upper_bound < other.upper_bound;
    }

    friend ostream& operator<<(ostream& os, const RBData& data) {
        os << data.upper_bound;
        return os;
    }
};

template <typename Key, typename Value>
class BBL_DS {

    using Item = pair<Key, Value>;
    using BlockT = Block<Key, Value>;
    using BlockSeq = list<BlockT>;

    using BlockIt = typename BlockSeq::iterator;
    using ItemIt = typename BlockT::LinkedList::iterator;


private:
    int M;
    Value B{};
    BlockSeq D0;  // Blocks from batch_prepend
    BlockSeq D1;  // Blocks from regular insertions (TODO: check deque or others)
    RBT<RBData<Key, Value>> rbtree_D1; // Red-Black Tree for D1 upper bounds (TODO: check cpp maps and sets)

    unordered_map<Key, pair<BlockIt, ItemIt>> keymap; // for lookups


    void register_block_in_RBT(BlockIt block_it) {
        RBData<Key, Value> data{block_it->upper_bound, &*block_it};
        rbtree_D1.insert(data);
    }

    void unregister_block_in_RBT(BlockIt block_it) {
        RBData<Key, Value> data{block_it->upper_bound, &*block_it};
        rbtree_D1.remove(data);
    }

    BlockIt which_D1_block_for_value(Value value) {
        RBData<Key, Value> data{value, nullptr};
        auto node = rbtree_D1.lower_bound(data);
        if (node == nullptr) {
            return D1.end();
        }

        //conversion ptr to it
        Block<Key, Value> *block_ptr = node->data.block_ptr;
        for (auto it = D1.begin(); it != D1.end(); ++it) {
            if (&*it == block_ptr) {
                return it;
            }
        }
        return D1.end();
    }

    void block_batch_insert(vector<Item> &L, BlockIt block_it, bool update_ub=false) {
        Value ub = Value(-1*INF);
        for (auto &p : L) {
            ItemIt item_it = block_it->insert(p);
            keymap[p.first] = {block_it, item_it};
            ub = max(ub, p.second);
        }
        if (update_ub) {
            block_it->upper_bound = ub;
        }
    }

    void split_D1_block(BlockIt block_it) {
        vector<Item> block_items(block_it->items.begin(), block_it->items.end());
        vector<vector<Item>> blocks = blocks_content_by_median(block_items, M/2 + 1); // 2 blocks

        if (blocks.size() > 2) {
            throw invalid_argument("/!\\ Split D1 block: more than 2 blocks" );
        }

        BlockT new_block;
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

    void blocks_content_by_median_helper(vector<vector<Item>> &sortie, vector<Item> &L, int block_size) {
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
        vector<Item> left_block;
        vector<Item> right_block;
        for (auto &p : L) {
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

        blocks_content_by_median_helper(sortie, left_block, block_size);
        blocks_content_by_median_helper(sortie, right_block, block_size);
    }

    vector<vector<Item>> blocks_content_by_median(vector<Item> &L, int block_size) {
        vector<vector<Item>> sortie;
        if (block_size >= static_cast<int>(L.size())) {
            sortie.push_back(L);
        }else {
            blocks_content_by_median_helper(sortie, L, block_size);
        }
        return sortie;
    }

    void delete_pair_from_keymap_it(typename unordered_map<Key, pair<BlockIt, ItemIt>>::iterator it) {
        BlockIt block_it = it->second.first;
        ItemIt item_it = it->second.second;

        block_it->remove(item_it);
        keymap.erase(it);

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
            for (auto &p : block_it->items) {
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

    BlockIt get_D0_block_position(vector<Item> block_content) {
        Item maxi = block_content.front();
        for (auto &p : block_content) {
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

    typename vector<Item>::iterator find_key_in_seq(Key key, vector<Item> &seq) {
        for (auto it = seq.begin(); it != seq.end(); ++it) {
            if (it->first == key) {
                return it;
            }
        }
        return seq.end();
    }

public:
    BBL_DS() = default;

    void initialize(int M, Value B) {
        this->D0.clear();
        this->D1.clear();
        this->keymap.clear();
        this->rbtree_D1.clear();

        BlockT b(B);
        this->D1.emplace_back(b);
        BlockIt block_it = this->D1.begin();
        block_it->location = BlockT::Location::D1;
        register_block_in_RBT(block_it);

        this->M = M;
        this->B = B;
    }

    void delete_pair(const Item &p){
        auto it = keymap.find(p.first);
        if (it == keymap.end()) {
            //cout << "Delete Pair: Key "<< p.first << " not found." << endl;
            return;
        }

        delete_pair_from_keymap_it(it);
    }

    void insert_pair(const Item &p) {
        auto it = keymap.find(p.first);
        if (it != keymap.end()) {
            Value old_v = it->second.second->second;
            if (p.second < old_v) {
                delete_pair_from_keymap_it(it);
            } else {
                return;
            }
        }

        BlockIt block_it = which_D1_block_for_value(p.second);
        if (block_it == D1.end()) {
            throw invalid_argument("Insert Pair: No existing block has ub >= value "+value_as_string(p.second));
        }

        ItemIt item_it = block_it->insert(p);
        keymap[p.first] = {block_it, item_it};

        if (block_it->items.size() > M) {
            split_D1_block(block_it);
        }
    }

    void batch_prepend(vector<Item> &L) {
        if (L.empty()) {
            return;
        }

        // handle duplicates and existing
        vector<Item> cleaned_L;
        for (auto p: L) {
            auto it_v = find_key_in_seq(p.first, cleaned_L);
            if (it_v != cleaned_L.end()) {
                if (p.second < it_v->second) {
                    cleaned_L.erase(it_v);
                }else {
                    continue;
                }
            }
            auto it = keymap.find(p.first);
            if (it != keymap.end()) {
                Value old_v = it->second.second->second;
                if (p.second < old_v) {
                    //cout << "Batch Prepend: Key " << p.first << " already exists and deleted." << endl;
                    delete_pair_from_keymap_it(it);
                }else {
                    continue;
                }
            }
            cleaned_L.push_back(p);
        }

        bool just_push_all = is_block_sequence_empty(D0);
        int block_size = static_cast<int>(cleaned_L.size()) <= M ? M : (M+1)/2;
        vector<vector<Item>> blocks = blocks_content_by_median(cleaned_L, block_size);

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
        vector<Item> buffer;
        vector<Key> keys;
        buffer.reserve(2*M);

        fill_buffer_for_pull(buffer, D0);
        fill_buffer_for_pull(buffer, D1);

        int n = static_cast<int>(buffer.size());

        if (n <= M) {
            for (auto &p : buffer) {
                delete_pair(p);
                keys.push_back(p.first);
            }
            if (total_pairs() <= 0) { //if we removed all //TODO check this (always true here)
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
            delete_pair(buffer[i]);
            keys.push_back(buffer[i].first);
        }
        Value x0 = !is_block_sequence_empty(D0) ? D0.front().min_value(): Value(INF);
        Value x1 = !is_block_sequence_empty(D1) ? D1.front().min_value() : Value(INF);
        x = min(x0, x1);
        return keys;
    }

    int total_pairs() {
        return keymap.size();
    }

    bool empty() {
        return total_pairs() == 0;
    }

    pair<BlockSeq, BlockSeq> get_sequences() {
        return {D0, D1};
    }

    bool contains(Key key) {
        return keymap.find(key) != keymap.end();
    }
};

#endif