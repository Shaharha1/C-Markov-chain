#include "markov_chain.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


Node* add_to_database(MarkovChain *markov_chain, void *data_ptr)
{
  //part one - search for the word
  if(markov_chain->database != NULL && markov_chain !=  NULL)
  {
    if(markov_chain->database->first != NULL)
    {
      Node * ptr = markov_chain->database->first;
      while(ptr != markov_chain->database->last->next)
      {
        if(!markov_chain->comp_func(data_ptr,(ptr->data->data)))
        {
          return ptr;
        }
        ptr = ptr->next;
      }
    }
  }
  //part two - create new word
  MarkovNode *new_markov_node = malloc(sizeof (MarkovNode));
  new_markov_node->data = markov_chain->copy_func(data_ptr);
  if(!new_markov_node->data)
  {
    return NULL; //alloc problem
  }
  new_markov_node->frequencies_list = malloc(sizeof(MarkovNodeFrequency));
  if(!new_markov_node->frequencies_list)
  {
    return NULL;
  }
  new_markov_node->frequencies_list->markov_node = NULL;
  new_markov_node->frequencies_list->frequency = 0;
  new_markov_node->freq_len = 0;
  //part three add node to link list
  if(add (markov_chain->database,new_markov_node))
  {
    return NULL; //alloc problem
  }
  return markov_chain->database->last;
}

Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr)
{
  if(markov_chain->database != NULL && markov_chain != NULL)
  {
    Node * ptr = markov_chain->database->first;
    while(ptr != markov_chain->database->last->next)
    {
      if (!markov_chain->comp_func (data_ptr, (ptr->data->data)))
      {
        return ptr;
      }
      ptr = ptr->next;
    }
  }
  return NULL;
}

bool add_node_to_frequencies_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain* markov_chain)
{
  if(first_node->frequencies_list == NULL)
  {
    MarkovNodeFrequency *temp = malloc (sizeof (MarkovNodeFrequency));
    if(!temp)
    {
      return false;
    }
    first_node->frequencies_list = temp;
    first_node->frequencies_list->markov_node = NULL;
    first_node->frequencies_list->frequency = 0;
  }
  //part one - search if the second node data in the first node
  MarkovNodeFrequency* ptr = first_node->frequencies_list;
  if(ptr->markov_node != NULL)
  {
    for(int i=0; i<first_node->freq_len; i++)
    {
      if(!markov_chain->comp_func((ptr+i)->markov_node->data,
                                  second_node->data))
      {
        (ptr+i)->frequency++;
        return true;
      }
    }
  }
  //part two - add second node to first node freq list
  MarkovNodeFrequency* temp = realloc (first_node->frequencies_list,
                                       ((first_node->freq_len + 1) *
                                       sizeof(MarkovNodeFrequency)));
  if(!temp)
  {
    return false; // alloc problem
  }
  first_node->frequencies_list = temp;
  ptr = first_node->frequencies_list;
  (ptr+(first_node->freq_len))->markov_node = second_node;
  (ptr+(first_node->freq_len))->frequency = 1;
  first_node->freq_len ++;
  return true;
}

void free_database(MarkovChain ** ptr_chain)
{
  if(*ptr_chain && (*ptr_chain)->database != NULL)
  {
    Node* ptr = (*ptr_chain)->database->first;
    Node* temp;
    while(ptr != NULL)
    {
      free(ptr->data->frequencies_list);
      (*ptr_chain)->free_data(ptr->data->data);
      free(ptr->data);
      temp = ptr->next;
      free(ptr);
      ptr = temp;
    }
    free(*ptr_chain);
  }
}

/**
* Get random number between 0 and max_number [0,max_number).
 * @param max_number maxmial number to return (not including).
 * @return Random number
*/
int get_random_number(int max_number)
{
  return rand() % max_number;
}

MarkovNode* get_first_random_node(MarkovChain *markov_chain)
{
  int random;
  Node* ptr;
  do{
    random = get_random_number (markov_chain->database->size);
    ptr = markov_chain->database->first;
    for(int i=0; i< random; i++)
    {
      ptr = ptr->next;
    }
    if(!markov_chain->is_last (ptr->data->data))
    {
      return ptr->data;
    }
  }
  while(true);
}

MarkovNode* get_next_random_node(MarkovNode *state_struct_ptr)
{
  //create the random number
  int count = 0;
  MarkovNodeFrequency* ptr = state_struct_ptr->frequencies_list;
  for(int i=0; i<(state_struct_ptr->freq_len);i++)
  {
    count += (ptr+i)->frequency;
  }
  int random = get_random_number (count);
  //find the word in the freq list
  if(random == 0)
  {
    return ptr->markov_node;
  }
  int j = 0;
  while(random >= 0)
  {
    random -= (ptr+j)->frequency;
    j++;
  }
  return (ptr+j-1)->markov_node;
}

void generate_tweet(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  while(markov_chain->is_last (first_node->data))
  {
    first_node = get_first_random_node (markov_chain);
  }
  markov_chain->print_func(first_node->data);
  MarkovNode* next = first_node;
  int count = 1;
  while(!markov_chain->is_last(next->data) && count <max_length)
  {
    next = get_next_random_node (next);
    count ++;
    markov_chain->print_func(next->data);
  }
}