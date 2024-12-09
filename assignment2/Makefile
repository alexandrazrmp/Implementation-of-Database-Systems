bplus:
	rm data.db
	@echo " Compile bplus_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bp_main.c ./src/record.c ./src/bp_file.c ./src/bp_datanode.c ./src/bp_indexnode.c -lbf -g  -o ./build/bplus_main -O2

run:
	./build/bplus_main
	rm data.db
val:
	valgrind --leak-check=full --track-origins=yes ./build/bplus_main