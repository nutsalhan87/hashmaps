%.o: %.c hashmap_qp.h
	gcc -c $< -o $@

hashmap_qp_test: hashmap_qp.o hashmap_qp_test.o
	gcc $^ -o $@

test: hashmap_qp_test
	./hashmap_qp_test

clean:
	rm *.o hashmap_qp_test 