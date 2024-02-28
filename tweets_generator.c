#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "markov_chain.h"

#define SEED 1
#define TWEET_AMOUNT 2
#define PATH 3
#define WORD_AMOUNT 4
#define ERROR_CLI "Usage: the program gets arguments " \
                  "<seed> <number of tweets> <path> <amount of words to "\
                  "read- optional>"
#define SENTENCE_LEN 1000
#define MAX_TWEET_LEN 20
#define WITH 5
#define WITHOUT 4
#define MAX_SIZE 99999999

/**
 * Check if the mount of args by the user is valid
 * @param argc number of args
 * @return true upon success, false otherwise
 */
static bool argc_check (int argc)
{
  if(argc != WITHOUT && argc != WITH)
  {
    printf("%s\n", ERROR_CLI);
    return false;
  }
  return true;
}

/**
 * Check if the file given by the input is valid
 * @param input_file
 * @return true upon success, false otherwise
 */
static bool path_check (FILE* input_file)
{
  if(input_file == NULL)
  {
    fclose (input_file);
    return false;
  }
  return true;
}


/**
 * Get string and check if there is . in the end
 * @param ptr pointer to char represent a word
 * @return true if their is a dot, false otherwise
 */
static int convert_to_int(char const* num)
{
  enum Var {ASCII_0 = 48, TEN = 10};
  int i = 0, int_num=0;
  while(num[i] != '\n' && num[i] != '\0')
  {
    if(isdigit (num[i]))
    {
      int_num += num[i] - ASCII_0;
      int_num *= TEN;
      i++;
    }
    else
    {
      return 0;
    }
  }
  int_num /= TEN;
  return int_num;
}

/**
 * Check if the word is ending with '.'
 * @param ptr word
 * @return true upon success, false otherwise
 */
static bool check_word_is_final(char const* ptr)
{
  enum Var {ASCII_DOT = 46};
  int i =0;
  while(ptr[i] != '\n' && ptr[i] != '\0')
  {
    if((ptr[i] == ASCII_DOT) && (ptr[i+1] == '\n' || ptr[i+1] == '\0'))
    {
      return true;
    }
    i++;
  }
  return false;
}

/**
 * Get a file and create a markov_chain of it
 * @param fp file
 * @param words_to_read amount of words to read from the file
 * @param markov_chain markov chain
 * @return 0 upon success, 1 otherwise
 */
static int fill_database(FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  char sentence[SENTENCE_LEN];
  int count = 0;
  while(fgets (sentence, SENTENCE_LEN, fp) != NULL && count < words_to_read)
  {
    char* word;
    word = strtok(sentence, " \n");
    count ++;
    Node* first = add_to_database (markov_chain, word);
    if(!first)
    {
      return EXIT_FAILURE;
    }
    Node *second;
    while(word != NULL)
    {
      word = strtok (NULL, " \n");
      if (word == NULL)
      {
        break;
      }
      count++;
      second = add_to_database (markov_chain, word);
      if (!second)
      {
        return EXIT_FAILURE;
      }
      if (!check_word_is_final (first->data->data))
      {
        if (!add_node_to_frequencies_list (first->data, second->data,
                                           markov_chain))
        {
          return EXIT_FAILURE;
        }
      }
      first = second;
      if (count >= words_to_read)
      {
        break;
      }
    }
  }
  return EXIT_SUCCESS;
}

/**
 * get a pointer and create a copy of its data
 * @param origin
 * @return copy
 */
static void* copy_char(void* origin)
{
  char* c_origin = (char*)origin;
  char* copy = malloc(strlen(c_origin)+1);
  if(!copy)
  {
    return NULL;
  }
  memcpy(copy,c_origin,strlen(c_origin)+1);
  return (void*)copy;
}

/**
 * get to pointers compere them
 * @param a
 * @param b
 * @return 0 if equal, >0 if the first if bigger, <0 if the second bigger
 */
static int comp_char(void* a,void*b)
{
  char* ac = (char*)a;
  char* bc = (char*)b;
  return strcmp (ac,bc);
}

/**
 * get a pointer to data
 * @param elem
 * @return true if data is final, false otherwise
 */
static bool check_end_char(void* elem)
{
  char* word = (char*)elem;
  return check_word_is_final (word);
}

/**
 * get a pointer to data and print it
 * @param elem
 */
static void print_char(void* elem)
{
  char* word = (char*)elem;
  printf(" %s", word);
}


static void initial_chain(MarkovChain** chain)
{
  (*chain)->free_data = free;
  (*chain)->comp_func = comp_char;
  (*chain)->copy_func = copy_char;
  (*chain)->is_last = check_end_char;
  (*chain)->print_func = print_char;
}
/**
 * Get input from user and print tweets acording to the given args
 * @param argc
 * @param argv
 * @return 0 upon success, 1 otherwise
 */
int main(int argc, char *argv[])
{
  //valid checks
  if(!argc_check (argc))
  {
    return EXIT_FAILURE;
  }
  FILE* input_file = fopen(argv[PATH], "r");
  if(!path_check (input_file))
  {
    return EXIT_FAILURE;
  }
  //create database
  int seed = convert_to_int (argv[SEED]);
  int tweet_amount = convert_to_int (argv[TWEET_AMOUNT]);
  int words_to_read = MAX_SIZE;
  if(argc == WITH)
  {
    words_to_read = convert_to_int (argv[WORD_AMOUNT]);
  }
  srand (seed);
  MarkovChain *chain = malloc (sizeof (MarkovChain));
  LinkedList* lnk = malloc (sizeof (LinkedList));
  if(!chain || !lnk)
  {
    printf("%s\n", ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  initial_chain (&chain);
  lnk->first = NULL;
  lnk->last = NULL;
  lnk->size = 0;
  chain->database = lnk;
  if(fill_database (input_file,words_to_read, chain))
  {
    printf("%s\n", ALLOCATION_ERROR_MASSAGE);
    free_database (&chain);
    fclose (input_file);
    return EXIT_FAILURE;
  }
  fclose (input_file);
  //create the tweets
  for(int i=0; i<tweet_amount;i++)
  {
    printf("Tweet %d:", i+1);
    generate_tweet (chain, get_first_random_node (chain) ,MAX_TWEET_LEN);
    printf("\n");
  }
  free_database (&chain);
  return EXIT_SUCCESS;
}