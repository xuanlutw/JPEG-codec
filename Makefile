all: jpeg_encoder jpeg_decoder

jpeg_encoder: src/encoder.c
	gcc src/encoder.c -O3 -lm -o jpeg_encoder

jpeg_decoder: src/decoder.c
	gcc src/decoder.c -O3 -lm -o jpeg_decoder

clean:
	rm jpeg_encoder jpeg_decoder
