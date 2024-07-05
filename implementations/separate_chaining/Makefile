%.o: %.c hashmap_sc.h
	gcc -c $< -o $@

hashmap_sc_test: hashmap_sc.o hashmap_sc_test.o
	gcc $^ -o $@

test: hashmap_sc_test
	./hashmap_sc_test

clean:
	rm *.o hashmap_sc_test 