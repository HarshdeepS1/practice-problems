// Given the head of a singly linked list, determine whether it contains a cycle 
// (some node's next eventually points back to an earlier node instead of NULL).

// Constraints:

// O(n) time.
// O(1) extra space — this rules out the "obvious" approach of storing every visited pointer in a hash set to check for repeats.

typedef struct node {
    int data;
    struct node *next;
} node_t;

bool has_cycle(node_t *head) {
    if (head == NULL) return false; // For empty case
    node_t* slow = head;
    node_t* fast = head;

    while (true) {
        if (fast->next != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
        else return false;


        if (fast->next != NULL){
            fast = fast->next;
        } 
        else return false;
        
        if (fast == slow) return true;

    }

}