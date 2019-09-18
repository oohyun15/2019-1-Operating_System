all:
	gcc -std=c99 ./src/wc.c -o wc -lpthread -O2

debug:
	gcc -std=c99 ./src/wc.c -o wc -lpthread -g

run1: all
	./wc ./data/owl.txt

run2: all
	./wc ./data/therepublic.txt

test:
	./wc ./data/$(INPUT) > $(INPUT).output
	cmp $(INPUT).output ./samples/$(INPUT).sample
	
test1: all
	./wc ./data/owl.txt | diff -Naur samples/owl.txt.sample - ;

test2: all
	./wc ./data/therepublic.txt | diff -Naur samples/therepublic.txt.sample - ;
