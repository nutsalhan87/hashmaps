%.o: %.c hashmap_lp.h
	gcc -c $< -o $@

hashmap_lp_test: hashmap_lp.o hashmap_lp_test.o
	gcc $^ -o $@

test: hashmap_lp_test
	./hashmap_lp_test

clean:
	rm *.o hashmap_lp_test 