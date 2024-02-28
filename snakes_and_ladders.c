#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"
#include <ctype.h>

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define SEED 1
#define NUMBEROFPATHS 2
#define ERROR_CLI "Usage: the program gets arguments " \
                  "<seed> <number of paths>"
/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladde
                    // r in case there is one from this square
    int snake_to;  // snake_to represents the jump of the snake
                   // in case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
      free_database (database);
    }
    return EXIT_FAILURE;
}


static int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error(ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to)
        {
            cells[from - 1]->ladder_to = to;
        }
        else
        {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database(MarkovChain *markov_chain)
{
    Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        from_node = get_node_from_database(markov_chain,cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain, cells[index_to])
                    ->data;
          add_node_to_frequencies_list (from_node, to_node, markov_chain);
        }
        else
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;
                }
                to_node = get_node_from_database(markov_chain, cells[index_to])
                        ->data;
              add_node_to_frequencies_list (from_node, to_node, markov_chain);
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

/**
 * get to pointers compere them
 * @param a
 * @param b
 * @return 0 if equal, >0 if the first if bigger, <0 if the second bigger
 */
static int comper_cell(void* a, void* b)
{
  char * a_char = (char*)a;
  char* b_char = (char*)b;
  return strcmp (a_char,b_char);
}

/**
 * get a pointer and create a copy of its data
 * @param origin
 * @return dest
 */
static void* copy_cell(void* a)
{
  char* a_char = (char*) a;
  char* dest = malloc(sizeof (int )*3);
  if(!dest)
  {
    return NULL;
  }
  memcpy (dest,a_char,sizeof (int )*3);
  return (void*)dest;
}

/**
 * get a pointer to data
 * @param elem
 * @return true if data is final, false otherwise
 */
static bool is_last_cell(void* a)
{
  enum Var {LAST_CELL = 100};
  char* a_int = (char*)a;
  int num = (int)(*(a_int));
  if (num == LAST_CELL)
  {
    return true;
  }
  return false;
}

/**
 * get a pointer to data and print it
 * @param elem
 */
static void print_cell(void* a)
{
  enum Var {LADDER = 1, SNAKE = 2};
  char* a_int = (char*)a;
  int data = (int)(*(a_int));
  int ladder = (int)(*(a_int+(LADDER*sizeof (int ))));
  int snake = (int)(*(a_int+(SNAKE*sizeof (int))));
  if(ladder != EMPTY)
  {
    printf ("[%d]-ladder to %d -> ",data, ladder);
  }
  else if(snake != EMPTY)
  {
    printf("[%d]-snake to %d -> ", data, snake);
  }
  else{
    if(data == BOARD_SIZE)
    {
      printf("[%d]", data);
    }
    else
    {
      printf("[%d] -> ",data);
    }
  }
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
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
  //validation
  if(argc != 3)
  {
    printf("%s\n",ERROR_CLI);
    return EXIT_FAILURE;
  }
  int seed = convert_to_int (argv[SEED]);
  srand (seed);
  int paths = convert_to_int (argv[NUMBEROFPATHS]);
  //crate database
  MarkovChain *chain = malloc (sizeof (MarkovChain));
  if(!chain)
  {
    printf("%s\n", ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  chain->free_data = free;
  chain->comp_func = comper_cell;
  chain->copy_func = copy_cell;
  chain->is_last = is_last_cell;
  chain->print_func = print_cell;
  LinkedList* lnk = malloc (sizeof (LinkedList));
  if(!lnk)
  {
    printf("%s\n", ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  lnk->first = NULL;
  lnk->last = NULL;
  lnk->size = 0;
  chain->database = lnk;
  if(fill_database (chain))
  {
    printf("%s\n", ALLOCATION_ERROR_MASSAGE);
    free_database (&chain);
    return EXIT_FAILURE;
  }
  //game play
  for(int i=0; i<paths; i++)
  {
    printf("Random Walk %d: ", i+1);
    generate_tweet (chain, chain->database->first->data,
                    MAX_GENERATION_LENGTH);
    printf("\n");
  }
  free_database (&chain);
  return EXIT_SUCCESS;
}