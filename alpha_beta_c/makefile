.PHONY: all clean

DLLS := ../lib/alpha_beta_bit_6x6_min.dll \
		../lib/alpha_beta_bit_6x6_true_stable.dll \
		../lib/alpha_beta_bit_8x8.dll\
		../lib/scout_mt_bit_6x6.dll \
		../lib/alpha_beta_bit_6x6_ratio.dll \

all: $(DLLS)

../lib/%.dll: %.cpp
	g++ -shared -o $@ -fPIC $<
