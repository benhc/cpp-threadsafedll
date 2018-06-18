#include <iostream> 
#include <thread> 
#include <random>
#include <mutex>
#include <unistd.h>
using namespace std;

/*
NOTE:: On my Linux machine I need to add -pthread to the compile instructions. 
       i.e. g++ -Wall --std=c++11 -pthread 4f14_abrh2.cc -o 4f14
*/

const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

struct node{
    string data;
    node* next; 
    node* prev;
    mutex mtx;
};

string random_string(const char alphabet[]);
void init_list(node*& head, node*& tail);
void create_node(node*& tail, string input);
void traverse_list(node* head);
void delete_node(node*& head, node*& tail, int listlength);
void loop_delete(node*& head, node*& tail, int listlength);


int main() { 
    srand(time(0));             //initialise random number generator
    node* head;                 //ptr to head node
    node* tail;                 //ptr to tail node
    int listlength = 100;       //number of nodes to add

    //Intialise the list
    init_list(head, tail); 
    //Add nodes to the list
    for(int i=1; i<=listlength; i++){
        create_node(tail, random_string(alphabet));
    }
    tail->next = NULL;          //specify end of list
    //Now start the threads running
    thread t(traverse_list, head);
    thread d(loop_delete, ref(head), ref(tail), listlength);

    d.join();                   //Join deletion thread until all nodes gone. 
    t.detach();                 
} 

string random_string(const char alphabet[]) {
    size_t length = rand() % 6+2;
    string str(length, 0);
    cout<<str;
    for(unsigned int i=0; i<length; i++){
        int randchar = rand() % 26;
        str.push_back(alphabet[randchar]);
    }
    return str;
}    

// Create a head node which contains no data, serves to enable traversal of list. 
void init_list(node*& head, node*& tail) {
    node* n = new node;
    n->data = "";
    n->prev = NULL;
    head = n;
    tail = n;
}

void create_node(node*& tail, string input) {
    node* n = new node;
    n->data = input;
    n->prev = tail;
    tail->next = n;
    tail = n;
}

//When traversing, hold mutex to the node being read.
void traverse_list(node* head) { 
    string strstate;
    while(true){
        string str;
        node* n = head;
        while(n->next!=NULL){
            lock_guard<mutex> guard(n->mtx);
            str.append(n->data);
            n = n->next;
        }
        //sleep(0); 
        if(str!= strstate && str!="") {     //Output only if state changes and not deleting the empty node
            strstate = str;
            cout<<str<<endl;}
    }
}

//Hold mutex to node being deleted and nodes before and after
void delete_node(node*& head, node*& tail, int listlength){   

    int selected_node = rand()%listlength+1;
    node* n = head;
    for(int i=0; i<selected_node; i++){    
        n = n->next;                    //This is a read only traversal - no data change happens anywhere so no mutex needed. 
    }

    // Clean up the redundant head node at the end.
    if(listlength==1){
        lock_guard<mutex> nguard(n->mtx);
        delete n;
    }

    node* next = n->next;
    node* prev = n->prev;
    string node_state;
    if(next==NULL){                                //If its the tail node 
        node_state="tail";
        //cout<<node_state<<endl;
        lock_guard<mutex> prevguard(n->prev->mtx); 
        lock_guard<mutex> nguard(n->mtx);
        tail=prev;
        tail->next = NULL;        
        delete n;
    }
    else{                                               //If its a middle node
        node_state="middle";
        //cout<<node_state<<endl;
        lock_guard<mutex> prevguard(n->prev->mtx);
        lock_guard<mutex> nguard(n->mtx);
        lock_guard<mutex> nextguard(n->next->mtx);
        prev->next = next; 
        next->prev = prev;
        delete n;
    }
}

void loop_delete(node*& head, node*& tail, int listlength){
    while(listlength>=2){
        delete_node(head, tail, listlength);
        listlength--;      
        sleep(1);
    }
}
