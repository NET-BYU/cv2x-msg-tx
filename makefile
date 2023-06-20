# LIBS = -lm -L/usr/local/lib/ -lsrsran_common -lsrsran_gtpu -lsrsran_mac -lsrsran_pdcp -lsrsran_phy -lsrsran_radio -lsrsran_rf -L/usr/lib/x86_64-linux-gnu/ -lfftw3 -lfftw3f
LIBS = -lm -lsrsran_common -lsrsran_gtpu -lsrsran_mac -lsrsran_pdcp -lsrsran_phy -lsrsran_radio -lsrsran_rf -lfftw3 -lfftw3f
INCLUDES = -I/usr/include/srsran/
build: ./src/transmitter.c
# g++ -c ./src/ue_sl.c -o ./build/ue_sl.o
# g++ -c ./src/transmitter.c -o ./build/transmitter.o
# g++ ./build/ue_sl.o ./build/transmitter.o $(LIBS) $(INCLUDES) -o ./build/transmitter
	g++ ./src/ue_sl.c ./src/transmitter.c $(INCLUDES) $(LIBS) -o ./build/transmitter

clean:
	rm -f build/*