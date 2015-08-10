
CLEAN := clean
TOPDIR := $(shell pwd)
export TOPDIR

all:
	$(MAKE) -C $(TOPDIR)/RTSPServer/
	$(MAKE) -C $(TOPDIR)/RTSPClient/
	$(MAKE) -C $(TOPDIR)/Server/
	$(MAKE) -C $(TOPDIR)/MediaServerC/

clean:
	$(MAKE) $(CLEAN) -C $(TOPDIR)/RTSPServer/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/RTSPClient/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/Server/
	$(MAKE) $(CLEAN) -C $(TOPDIR)/MediaServerC/

