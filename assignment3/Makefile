sort:
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2

sort_upload:
	@echo " Compile sort_main_upload ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main_upload.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main_upload -O2

run:
	@echo " Run sort_main ...";
	./build/sort_main

run_upload:
	@echo " Run sort_main_upload ...";
	./build/sort_main_upload

valgrind:
	@echo " Valgrind sort_main ...";
	valgrind ./build/sort_main