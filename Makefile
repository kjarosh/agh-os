DIRS=$(shell find . -name Makefile -printf '%h\n' | sort)
PREQ_ALL=$(addprefix all-,$(DIRS))
PREQ_CLEAN=$(addprefix clean-,$(DIRS))

all: $(PREQ_ALL)

clean: $(PREQ_CLEAN)

all-. clean-.:
	true

all-./%:
	$(MAKE) -C "$(@:all-%=%)" all

clean-./%:
	$(MAKE) -C "$(@:clean-%=%)" all
