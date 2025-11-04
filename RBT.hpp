/*
 *Red-Black Tree in cpp
 *src: https://fr.wikipedia.org/wiki/Arbre_bicolore, https://github.com/yassiommi/redblacktree/tree/main
 */

#ifndef DIJKSTRA_RBT_HPP
#define DIJKSTRA_RBT_HPP
#include <iostream>


using namespace std;

enum  Color {RED, BLACK};
enum  Direction {LEFT, RIGHT};

template <typename T>
class RBT {
    struct Node {
        T data;
        Color color;
        Node *papa;
        Node *children[2]{};

        explicit Node(T data) {
            this->data = data;
            this->color = RED;
            this->papa = nullptr;
            this->children[LEFT] = nullptr;
            this->children[RIGHT] = nullptr;
        }
    };

private:
    Node *root;

    void rotate(Node *node, Direction dir) {
        Node *papa = node->papa;
        Node *new_root = node->children[1-dir];
        Node *new_child = new_root->children[dir];

        node->children[1-dir] = new_child;
        if (new_child != nullptr) {
            new_child->papa = node;
        }

        new_root->children[dir] = node;
        new_root->papa = papa;
        node->papa = new_root;

        if (papa != nullptr) {
            papa->children[node == papa->children[RIGHT]] = new_root;
        } else {
            root = new_root;
        }
    }

    void insert_balancing(Node *&node) {
        Node *papa = nullptr;
        Node *grandpa = nullptr;

        while (node != root && node->color == RED && node->papa->color == RED) {
            papa = node->papa;
            grandpa = papa->papa;
            if (papa == grandpa->children[LEFT]) {
                Node *uncle = grandpa->children[RIGHT];
                if (uncle != nullptr && uncle->color == RED) {
                    grandpa->color = RED;
                    papa->color = BLACK;
                    uncle->color = BLACK;
                    node = grandpa;
                } else {
                    if (node == papa->children[RIGHT]) {
                        rotate(papa, LEFT);
                        node = papa;
                        papa = node->papa;
                    }
                    rotate(grandpa, RIGHT);
                    swap(papa->color, grandpa->color);
                    node = papa;
                }
            } else {
                Node *uncle = grandpa->children[LEFT];
                if (uncle != nullptr && uncle->color == RED) {
                    grandpa->color = RED;
                    papa->color = BLACK;
                    uncle->color = BLACK;
                    node = grandpa;
                } else {
                    if (node == papa->children[LEFT]) {
                        rotate(papa, RIGHT);
                    }
                    swap(papa->color, grandpa->color);
                    node = papa;
                }
            }
        }
        root->color = BLACK;
    }

    void remove_balacing(Node *node) {
        while (node != nullptr && node != root && node->color == BLACK) {
            if (node == node->papa->children[LEFT]) {
                Node *bro = node->papa->children[RIGHT];
                if (bro == nullptr) {
                    return;
                }
                if (bro->color == RED) {
                    bro->color = BLACK;
                    node->papa->color = RED;
                    rotate(node->papa, LEFT);
                    bro = node->papa->children[RIGHT];
                }
                if (bro != nullptr && (bro->children[LEFT] == nullptr || bro->children[LEFT]->color == BLACK) && (bro->children[RIGHT] == nullptr || bro->children[RIGHT]->color == BLACK)) {
                    bro->color = RED;
                    node = node->papa;
                } else {
                    if (bro == nullptr) {
                        return;
                    }
                    if (bro->children[RIGHT] == nullptr || bro->children[RIGHT]->color == BLACK) {
                        if (bro->children[LEFT] != nullptr) {
                            bro->children[LEFT]->color = BLACK;
                        }
                        bro->color = RED;
                        rotate(bro, RIGHT);
                        bro = node->papa->children[RIGHT];
                    }
                    if (bro == nullptr) {
                        return;
                    }
                    bro->color = node->papa->color;
                    node->papa->color = BLACK;
                    if (bro->children[RIGHT] != nullptr) {
                        bro->children[RIGHT]->color = BLACK;
                    }
                    rotate(node->papa, LEFT);
                    node = root;
                }
            } else {
                Node *bro = node->papa->children[LEFT];
                if (bro == nullptr) {
                    return;
                }
                if (bro->color == RED) {
                    bro->color = BLACK;
                    node->papa->color = RED;
                    rotate(node->papa, RIGHT);
                    bro = node->papa->children[LEFT];
                }
                if (bro != nullptr && (bro->children[RIGHT] == nullptr || bro->children[RIGHT]->color == BLACK) && (bro->children[LEFT] == nullptr || bro->children[LEFT]->color == BLACK)) {
                    bro->color = RED;
                    node = node->papa;
                } else {
                    if (bro == nullptr) {
                        return;
                    }
                    if (bro->children[LEFT] == nullptr || bro->children[LEFT]->color == BLACK) {
                        if (bro->children[RIGHT] != nullptr) {
                            bro->children[RIGHT]->color = BLACK;
                        }
                        bro->color = RED;
                        rotate(bro, LEFT);
                        bro = node->papa->children[LEFT];
                    }
                    if (bro == nullptr) {
                        return;
                    }
                    bro->color = node->papa->color;
                    node->papa->color = BLACK;
                    if (bro->children[LEFT] != nullptr) {
                        bro->children[LEFT]->color = BLACK;
                    }
                    rotate(node->papa, RIGHT);
                    node = root;
                }
            }
        }
        if (node != nullptr) {
            node->color = BLACK;
        }
    }

    void transplant(Node *node1, Node *node2) {
        if (node1->papa == nullptr) {
            root = node2;
        } else if (node1 == node1->papa->children[LEFT]) {
            node1->papa->children[LEFT] = node2;
        } else {
            node1->papa->children[RIGHT] = node2;
        }
        if (node2 != nullptr) {
            node2->papa = node1->papa;
        }
    }

    Node* minimum_node(Node *node) {
        while (node->children[LEFT] != nullptr) {
            node = node->children[LEFT];
        }
        return node;
    }

    void delete_node(Node *node) {
        Node *y = node;
        Node *x = nullptr;
        Color y_ori_color = y->color;

        if (node->children[LEFT] == nullptr) {
            x = node->children[RIGHT];
            transplant(node, node->children[RIGHT]);
        } else if (node->children[RIGHT] == nullptr) {
            x = node->children[LEFT];
            transplant(node, node->children[LEFT]);
        } else {
            y = minimum_node(node->children[RIGHT]);
            y_ori_color = y->color;
            x = y->children[RIGHT];
            if (y->papa == node) {
                if (x != nullptr) {
                    x->papa = y;
                }
            } else {
                if (x != nullptr) {
                    x->papa = y->papa;
                }
                transplant(y, y->children[RIGHT]);
                y->children[RIGHT] = node->children[RIGHT];
                if (y->children[RIGHT] != nullptr) {
                    y->children[RIGHT]->papa = y;
                }
            }
            transplant(node, y);
            y->children[LEFT] = node->children[LEFT];
            if (y->children[LEFT] != nullptr) {
                y->children[LEFT]->papa = y;
            }
            y->color = node->color;
        }
        delete node;

        if (y_ori_color == BLACK) {
            remove_balacing(x);
        }
    }

    void print_tree_aux(Node *node, int space) {
        constexpr int COUNT = 5;
        if (node == nullptr) {
            return;
        }

        space += COUNT;
        print_tree_aux(node->children[RIGHT], space);
        cout << endl;
        for (int i = COUNT; i < space; i++) {
            cout << " ";
        }
        cout << node->data << "(" << (node->color == RED ? "RED" : "BLACK") << ")" << endl;
        print_tree_aux(node->children[LEFT], space);
    }

    void clear_node(Node *node) {
        if (node == nullptr) {
            return;
        }
        clear_node(node->children[LEFT]);
        clear_node(node->children[RIGHT]);
        delete node;
    }


public:
    RBT() {
        root = nullptr;
    }

    ~RBT() {
        clear_node(root);
    }

    void insert(T data) {
        Node *node = new Node(data);
        Node *papa = nullptr;
        Node *iter = root;

        while (iter != nullptr) {
            papa = iter;
            if (node->data < iter->data) {
                iter = iter->children[LEFT];
            } else {
                iter = iter->children[RIGHT];
            }
        }

        node->papa = papa;
        if (papa == nullptr) {
            root = node;
        } else if (node->data < papa->data) {
            papa->children[LEFT] = node;
        } else {
            papa->children[RIGHT] = node;
        }

        insert_balancing(node);
    }

    void remove(T data) {
        Node *node = root;
        while (node != nullptr) {
            if (data < node->data) {
                node = node->children[LEFT];
            } else if (node->data < data) {
                node = node->children[RIGHT];
            } else {
                delete_node(node);
                return;
            }
        }

        //cout << "Element " << data << " not found" << endl;
    }

    Node* lower_bound(T data) {
        Node *node = root;
        Node *sortie = nullptr;
        while (node != nullptr) {
            if (!(node->data < data)) {
                sortie = node;
                node = node->children[LEFT];
            } else {
                node = node->children[RIGHT];
            }
        }

        return sortie;
    }

    void clear() {
        clear_node(root);
        root = nullptr;
    }

    void print_tree() {
        if (root == nullptr) {
            cout << "Tree is empty." << endl;
        } else {
            print_tree_aux(root, 0);
        }
    }

};

#endif