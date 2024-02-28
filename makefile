CFLAGS = -Wall

linked_list.o: linked_list.c linked_list.h
	gcc -c linked_list.c

markov_chain.o: markov_chain.c markov_chain.h linked_list.h
	gcc -c markov_chain.c

tweets_generator.o: tweets_generator.c markov_chain.h linked_list.h
	gcc -c tweets_generator.c

snake_and_ladders.o: snakes_and_ladders.c markov_chain.h linked_list.h
	gcc -c snakes_and_ladders.c

tweets: tweets_generator.o markov_chain.o linked_list.o
	gcc $(CFLAGS) tweets_generator.o markov_chain.o linked_list.o -o tweets_generator

snake: snake_and_ladders.o markov_chain.o linked_list.o
	gcc $(CFLAGS) snake_and_ladders.o markov_chain.o linked_list.o -o snakes_and_ladders