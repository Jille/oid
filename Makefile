oid: oid.c
	cc -o oid -Wall -g oid.c

test: oid
	for i in tests/*.dat; do echo "$$i"; cp $${i%dat}old $${i%dat}out; ./oid $${i%dat}out < $$i || break; diff -u $${i%dat}dat $${i%dat}out || break; done

clean:
	rm -f oid
