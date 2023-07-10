# .../impalcdhls/Makefile   -*- makefile -*-
#
# 20230416 mark_labels.pl is not needed for our work and causes errors with
#  some of the benchmark programs.

LOCAL_CONFIG = -legup-config=config.tcl
LLVM=$(LEVEL)/../llvm/Release+Asserts/bin/
CLANG=$(LEVEL)/../../clang/bin/clang
ANALYSIS=$(LEVEL)/../tiger/tool_source/partition_analysis/
# CFLAG=-fno-builtin -I $(LEVEL)/lib/include/ -m32 -I /usr/include/i386-linux-gnu -O0
CFLAG=-fno-builtin -I $(LEVEL)/lib/include/ -m32 -O0
CLANG_FLAG=-fno-vectorize -fno-slp-vectorize
OPT_FLAGS=-load=$(LEVEL)/../llvm/Release+Asserts/lib/LLVMLegUp.so -legup-config=$(LEVEL)/legup.tcl
LLC_FLAGS= -legup-config=$(LEVEL)/legup.tcl

touchSrc: $(NAME).c
	touch $(NAME).c
	rm -f 03-$(NAME).prelto.2.bc

02-$(NAME).prelto.cv.bc: ${NAME}.c
	# $(LEVEL)/mark_labels.pl $< > 01-$(NAME)_labeled.c
	# $(CLANG) 01-$(NAME)_labeled.c -emit-llvm -c $(CFLAG) $(CLANG_FLAG) -o 02-$(NAME).prelto.cv.bc
	$(CLANG) $(NAME).c -emit-llvm -c $(CFLAG) $(CLANG_FLAG) -o 02-$(NAME).prelto.cv.bc

03-$(NAME).prelto.2.bc: 02-$(NAME).prelto.cv.bc
	$(LLVM)opt $(OPT_OLEVEL) $(EXTRA_OPT_FLAGS) < 02-$(NAME).prelto.cv.bc > 03-$(NAME).prelto.2.bc

# Perform the compilation, and then perform a checksum on the result.
# This will print "Checksum Report" followed by the checksum, unless
# there is no prelto.2.bc which would be from a failed compilation
optbcChecksum: 03-$(NAME).prelto.2.bc
	echo "Checksum Report" > 04-$(NAME).cksum.rpt
	echo -n " cksum | " >> 04-$(NAME).cksum.rpt
	/usr/bin/cksum 03-$(NAME).prelto.2.bc >> 04-$(NAME).cksum.rpt
	echo " " >> 04-$(NAME).cksum.rpt
	echo " " >> 04-$(NAME).cksum.rpt
	cat 04-$(NAME).cksum.rpt

# Given the "prelto.2.bc" version, compile into an executable for this machine
# and run it, in order to get its output (the test programs we use will
# try to test themselves and print PASS or FAIL)
optVerifyPASS: 03-$(NAME).prelto.2.bc
	$(LLVM)llc -filetype=obj -march=x86-64 03-$(NAME).prelto.2.bc -o 05-$(NAME).prelto.2.o
	gcc 05-$(NAME).prelto.2.o -o 06-$(NAME).prelto.2
	./06-$(NAME).prelto.2

# Save an assembly listing (in SSA format) to the parent directory (which will
# be "../tmp" relative to where the Python experiment is run). The saved file
# will start with a prefix $SAVE_PREFIX which should be something like
# "O3" or "fastest", etc. This also saves the TeX file (timing diagrams or
# "Gantt charts") and the Verilog (.v file), both with the same prefix.
saveSSA: 03-$(NAME).prelto.2.bc
	$(LLVM)llvm-dis 03-$(NAME).prelto.2.bc -o $(NAME).prelto.2.ll
	cp $(NAME).prelto.2.ll ../$(SAVE_PREFIX)-$(NAME).ll
	cp gantt.main.tex ../$(SAVE_PREFIX)-$(NAME)-gantt.tex
	cp 15-$(NAME).v ../$(SAVE_PREFIX)-$(NAME).v
	echo pass-ordering $(EXTRA_OPT_FLAGS) >../$(SAVE_PREFIX)-$(NAME)-ord.txt
	echo olevel $(OPT_OLEVEL) >>../$(SAVE_PREFIX)-$(NAME)-ord.txt

# This is like saveSSA, but saves only the LL file, and saves it into
# a directory ../tmp/ccmp. This is meant to be used for collecting a variety
# of LL files corresponding to compiled results that are the same in one
# way but different in another, such as having the same cycle count
# but having different checksums.
saveCycInst: 03-$(NAME).prelto.2.bc
	$(LLVM)llvm-dis 03-$(NAME).prelto.2.bc -o $(NAME).prelto.2.ll
	echo $(EXTRA_OPT_FLAGS) > ../ccmp/$(SAVE_PREFIX)-$(NAME).ll
	cat $(NAME).prelto.2.ll >> ../ccmp/$(SAVE_PREFIX)-$(NAME).ll

accelerationCycle: 03-$(NAME).prelto.2.bc
	/usr/bin/printenv | grep LD
	$(LLVM)opt $(OPT_FLAGS) -std-link-opts < 03-$(NAME).prelto.2.bc > 07-$(NAME).prelto.linked.1.bc
	$(LLVM)opt $(OPT_FLAGS) -legup-prelto < 07-$(NAME).prelto.linked.1.bc > 08-$(NAME).opted.bc
	$(LLVM)opt $(OPT_FLAGS) -std-link-opts -o 09-$(NAME).prelto.bc < 08-$(NAME).opted.bc
	$(LLVM)llvm-link 09-$(NAME).prelto.bc $(LEVEL)/lib/llvm/liblegup.bc $(LEVEL)/lib/llvm/libm.bc > 10-$(NAME).linked.bc
	$(LLVM)opt -internalize-public-api-list=main -internalize -globaldce -o 11-$(NAME).postlto.8.bc < 10-$(NAME).linked.bc
	$(LLVM)opt $(OPT_FLAGS) -instcombine -std-link-opts < 11-$(NAME).postlto.8.bc > 12-$(NAME).opted.bc
	# this next cmd also generates pipelining.legup.rpt
	rm -f resources.legup.rpt
	$(LLVM)opt $(OPT_FLAGS) -basicaa -loop-simplify -indvars2 -loop-pipeline < 12-$(NAME).opted.bc > 13-$(NAME).opted.bc
	$(LLVM)opt $(OPT_FLAGS) -instcombine -o 14-$(NAME).bc < 13-$(NAME).opted.bc
	# this next command generates the Verilog (15-<name>.v) and a ton of .rpt files including the Fmax in resources.legup.rpt
	export DEVICE_FAMILY="Virtex 6" && export BOARD_TOPS="$(LEVEL)/../boards/Virtex6/ML605/top.v" && $(LLVM)llc $(LLC_FLAGS) -march=v 14-$(NAME).bc -o 15-$(NAME).v
	$(LLVM)opt $(OPT_FLAGS) -legup-track-bb < 14-$(NAME).bc > 16-$(NAME).track_bb.bc
	$(LLVM)llvm-dis 16-$(NAME).track_bb.bc -o 17-$(NAME).track_bb.ll
	$(LLVM)llc -march=x86-64 16-$(NAME).track_bb.bc -o 18-$(NAME).track_bb.s
	rm -f 19-$(NAME).track_bb
	gcc 18-$(NAME).track_bb.s -o 19-$(NAME).track_bb
	./19-$(NAME).track_bb | grep 'Track@' | sed 's/Track@//' > 20-$(NAME).lli_bb.trace
	/usr/bin/perl $(ANALYSIS)get_hw_cycle.pl 20-$(NAME).lli_bb.trace 22-$(NAME).acel_cycle.rpt
	cat 22-$(NAME).acel_cycle.rpt
	cat resources.legup.rpt

.PHONY: clean
clean:
	-rm *.*.*
	-rm *_*
	-rm *.bc
	-rm *.mif
	-rm *.rpt
	-rm *.sdc
	-rm *.qsf
	-rm *.v
	-rm *.dot
	-rm *.txt
	-rm -r *_*
