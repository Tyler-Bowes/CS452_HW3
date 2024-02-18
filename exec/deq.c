#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];		// next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];			// head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

static void put(Rep r, End e, Data d) {
  /*append Data based on End to Rep
  each node has two pointers (one to the next node and one to the previous node)
  and Data as the value for the node*/

  if (!d) ERROR("Invalid Data. Cannot be 0"); //Data d is null or 0

  Node new_node = (Node)malloc(sizeof(*new_node));
  if (!new_node) ERROR("malloc() failed"); //if malloc fails, print error

  new_node->data = d; //set the data of the new node to the data passed in

  if (r->len == 0) { //if the length of the rep is 0, set the head and tail to the new node
    r->ht[Head] = new_node;
    r->ht[Tail] = new_node;
    new_node->np[Head] = 0;
    new_node->np[Tail] = 0;
  }
  else { //add new node according to End
    new_node->np[e] = 0; //set the End pointer of the new node to 0
    new_node->np[!e] = r->ht[e]; //set the opposite End pointer of the new node to the End node
    r->ht[e]->np[e] = new_node; //set the End node's End pointer to the new node
    r->ht[e] = new_node; //set the End node to the new node
  }
  r->len++;
}

static Data ith(Rep r, End e, int i)  { 
  /*returns the ith node i based on End e inside Rep r*/

  if (i > r->len - 1) {
    ERROR("Cannot get ith! i is greater than the length of the Rep!");
  }
  else if (i < 0) { //i is less than 0, return 0 and throw error
    ERROR("Cannot get ith! i is less than 0!");
  }
  else {
    Node temp = r->ht[e]; //set a temp node to the End node
    for (int j = 0; j < i; j++) { //loop through the rep until the ith node
      temp = temp->np[!e]; //set the temp node to the opposite End pointer of the temp node
    }
    return temp->data; //return the data of the ith node
  }
  return 0;
}

static Data get(Rep r, End e)         { 
  /*return Data of End e inside Rep r and remove it from the Rep*/
  if (r->len == 0) { //if the length of the rep is 0, return 0 and fatal error
    ERROR("Cannot get when Rep is empty!");
    return 0;
  }
  else { 
    Node temp = r->ht[e]; //set a temp node to the End node
    Data temp_data = temp->data; 
    if (r->len == 1) { //if the length of the rep is 1, set the head and tail to 0
      r->ht[Head] = 0;
      r->ht[Tail] = 0;
    }
    else { //if the length of the rep is greater than 1, set the head or tail to the next node
      r->ht[e] = temp->np[!e];
      r->ht[e]->np[e] = 0;
    }
    free(temp); //free the temp node b/c it is removed
    r->len--; 
    return temp_data; 
  }
}

static Data rem(Rep r, End e, Data d) { 
  /*return by == comparing, len-- (iff found)
  if trying to remove something that isnt there return 0 and error*/

  if (r->len == 0) { //rep len is 0, return 0 and error
    ERROR("Cannot remove when Rep is empty!");
    return 0;
  }  
  else {
    Node temp = r->ht[e]; 
    while (temp) { //loop through the rep until the temp node is null                         //!!!!!!!! or till temp != 0 !!!!!!!
      if (temp->data == d) { 
        if (r->len == 1) { //rep len is 1, make list empty
          r->ht[Head] = 0;
          r->ht[Tail] = 0;
        }
        else if (temp == r->ht[e]) { //temp node is the End node
          r->ht[e] = temp->np[!e]; //set the End node to the (previous/next node)
          r->ht[e]->np[e] = 0; //set the End node's End pointer to 0
        }
        else if (temp == r->ht[!e]) { //temp node is the opposite End node
          r->ht[!e] = temp->np[e]; 
          r->ht[!e]->np[!e] = 0; 
        }
        else { //temp node is in the middle of the rep
          temp->np[!e]->np[e] = temp->np[e]; //set the opposite End pointer of the temp node to the End pointer of the temp node
          temp->np[e]->np[!e] = temp->np[!e]; 
        }
        Data temp_data = temp->data; //set a temp data to the data of the temp node
        free(temp); //free the temp node b/c it is removed
        r->len--;
        return temp_data;
      }
      temp = temp->np[!e]; //set the temp node to the opposite End pointer of the temp node
    }
    ERROR("Cannot remove! Data is not in the Rep!");
    return 0;
  }
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head); }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail); }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
