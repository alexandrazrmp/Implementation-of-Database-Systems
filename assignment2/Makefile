bplus:
	@echo " Compile bplus_main_ex ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bp_main_ex.c ./src/record.c ./src/bp_file.c ./src/bp_datanode.c ./src/bp_indexnode.c -lbf -g  -o ./build/bplus_main_ex -O2

run:
	./build/bplus_main_ex 5
	rm data.db