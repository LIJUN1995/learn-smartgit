INC_DIR = \
-I "/home/orchid/android/ALPS-MP-M0.MP1-V2.84_AEON6737M_65_M0_INHOUSE/alps/kernel-3.18/" 
LIB_DIR = -L.
#LIBS = -lalgorithm_fpCapV_1.02.29_8056_32bit
LIBS = -lalgorithm_fpOpticV_1.01.03_64bit
saveImg:saveImg.c fb_test.c
	aarch64-linux-gnu-gcc -static -o $@ $^ $(LIB_DIR)  $(LIBS)


.PHONY = clean
clean:
	rm ./*.o fb_test
