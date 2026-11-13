#include <bits/stdc++.h>
using namespace std;

class SkipList {
public:
    struct Node {
        Node* next;
        Node* prev;
        Node* down;
        int val;
        
        Node(int val_) : next(nullptr), prev(nullptr), down(nullptr), val(val_) {}
    };

    SkipList(unsigned int seed) : seed_{seed} {
        levels_.resize(max_levels_);
        
        levels_[0] = new Node(-1);
        levels_[0]->next = new Node(1e9);
        levels_[0]->next->prev = levels_[0];

        for (int i = 1; i < max_levels_; i++) {
            levels_[i] = new Node(-1);
            levels_[i]->next = new Node(1e9);

            levels_[i]->next->prev = levels_[i];
            levels_[i]->down = levels_[i - 1];
        }
    }

    ~SkipList() {
        for (int level = 0; level < max_levels_; level++) {
            Node* node = levels_[level];
            
            while (node->next) {
                Node* next = node->next;
                delete node;
                node = next;
            }

            delete node;
        }
    }

    bool insert(int value) {
        if (search(value)) {
            return false;
        }

        next_seed();
        int this_level = get_level();
        highest_level_ = max(this_level, highest_level_);
        int current_level = highest_level_;

        Node* node = levels_[highest_level_];
        Node* curr = new Node(value);
        Node* below = nullptr;

        while (node) {
            while (node->next != nullptr && node->next->val <= value) {
                node = node->next;
            }

            if (current_level <= this_level) {
                curr->next = node->next;
                node->next->prev = curr;

                curr->prev = node;
                node->next = curr;

                if (current_level > 0) {
                    below = new Node(value);
                    curr->down = below;
                    curr = below;
                }
            }

            current_level--;
            node = node->down;
        }
        
        return true;
    }

    bool remove(int value) {
        if (!search(value)) {
            return false;
        }

        // get the top node of node to remove
        Node* node = levels_[highest_level_];
        while (node) {
            while (node->next != nullptr && node->next->val <= value) {
                node = node->next;
            }

            if (node->val == value) {
                break;
            }

            node = node->down;
        }

        // remove down the chain
        while (node) {
            Node* next = node->down;

            node->prev->next = node->next;
            node->next->prev = node->prev;

            delete node;

            node = next;
        }

        return true;
    }

    bool search(int value) {
        Node* node = levels_[highest_level_];
        
        while (node) {
            while (node->next != nullptr && node->next->val <= value) {
                node = node->next;
            }

            if (node->val == value) {
                return true;
            }

            node = node->down;
        }

        return false;
    }

    void display() {
        bool used = false;
        for (int i = highest_level_; i >= 0; i--) {
            if (levels_[i]->next->val == 1e9)
                continue;

            cout << "Level " << i << ": "; 
            
            Node* curr = levels_[i]->next;
            while (curr->next->next) {
                cout << curr->val << " ";
                curr = curr->next;
            }

            cout << curr->val << "\n";
            used = true;
        }

        if (!used) cout << "\n";
    }

private:
    unsigned int seed_;
    const int max_levels_ = 16;
    int highest_level_ = 0;
    vector<Node*> levels_;

    void next_seed() {
        seed_ = seed_ * 1103515245 + 12345;
    }

    int get_level() {
        int level = 0;
        for (int i = 0; i + 1 < max_levels_; i++) {
            if (seed_ & (1 << i)) {
                level++;
            } else {
                break;
            }
        }
        return level;
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string in; cin >> in;
    unsigned int cap; cin >> cap;
    SkipList list(cap);

    string s;
    while (getline(cin, s)) {
        stringstream ss(s);
        string t; ss >> t;

        if (t == "display") {
            list.display();
        } else if (t == "insert") {
            int val; ss >> val;
            bool res = list.insert(val);
            cout << "null\n";
        } else if (t == "search") {
            int val; ss >> val;
            bool res = list.search(val);
            cout << (res ? "true\n" : "false\n");
        } else if (t == "remove") {
            int val; ss >> val;
            bool res = list.remove(val);
            cout << "null\n";
        }
    }

    return 0;
}