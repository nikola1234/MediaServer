
CLEAN := clean
TOPDIR := $(shell pwd)
export TOPDIR

all:
	$(MAKE) -C $(TOPDIR)/Analysis/
	$(MAKE) -C $(TOPDIR)/RTSPServer/
	$(MAKE) -C $(TOPDIR)/RTSPClient/
	$(MAKE) -C $(TOPDIR)/Server/

clean:
	$(MAKE) $(CLEAN) -C $(TOPDIR)/Analysis/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/RTSPServer/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/RTSPClient/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/Server/
