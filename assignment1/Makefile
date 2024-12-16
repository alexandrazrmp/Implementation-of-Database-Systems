hp:
	@echo " Compile hp_main_ex ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main_ex.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main_ex -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/record.c -lbf -o ./build/bf_main -O2;
run:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/record.c -lbf -o ./build/bf_main -O2
	./build/bf_main 
	rm block_example.db

main:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main_ex.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main_ex -O2
	./build/hp_main_ex 1000
	rm data.db