.PHONY: all hw1 lr1 test test-sanitize clean
all: hw1 lr1
hw1:
	$(MAKE) -C homeworks/hw1
lr1:
	$(MAKE) -C labs/lr1
test:
	$(MAKE) -C homeworks/hw1 test
	$(MAKE) -C labs/lr1 test
test-sanitize:
	$(MAKE) -C labs/lr1 test-sanitize
clean:
	$(MAKE) -C homeworks/hw1 clean
	$(MAKE) -C labs/lr1 clean
