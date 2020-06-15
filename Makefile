all: jpeg_encoder jpeg_decoder

jpeg_encoder: src/encoder.c
	gcc src/encoder.c -O3 -lm -o jpeg_encoder

jpeg_decoder: src/decoder.c
	gcc src/decoder.c -O3 -lm -o jpeg_decoder

test: jpeg_encoder jpeg_decoder
	rm -rf ./Output
	mkdir ./Output
	./jpeg_decoder ./Image/gig-sn01.jpg ./Output/gig-sn01.bmp
	./jpeg_decoder ./Image/gig-sn08.jpg ./Output/gig-sn08.bmp
	./jpeg_decoder ./Image/monalisa.jpg ./Output/monalisa.bmp
	./jpeg_decoder ./Image/teatime.jpg  ./Output/teatime.bmp
	./jpeg_encoder ./Image/bmp_24.bmp   ./Output/bmp_24.jpg
	./jpeg_encoder ./Image/snail.bmp    ./Output/snail.jpg
	./jpeg_encoder ./Image/lenna.bmp    ./Output/lenna.jpg

clean:
	rm jpeg_encoder jpeg_decoder
