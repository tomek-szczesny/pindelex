default:
	gcc -o pindelex pindelex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./pindelex -w 125 -h 110
	./process.sh pindelex_front.svg
	./process.sh pindelex_back.svg
	./process.sh pindelex_side.svg
	./process.sh pindelex_strap.svg
