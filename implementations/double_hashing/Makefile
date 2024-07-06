%.o: %.c hashmap_dh.h
	gcc -c $< -o $@

hashmap_dh_test: hashmap_dh.o hashmap_dh_test.o
	gcc $^ -o $@

test: hashmap_dh_test
	./hashmap_dh_test

clean:
	rm *.o hashmap_dh_test 