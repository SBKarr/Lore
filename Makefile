# Copyright (c) 2020 Roman Katuntsev <sbkarr@stappler.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

STAPPLER_ROOT ?= ../stappler
CONFIG_INCLUDE ?= config

CONF ?= local
CONF_DIR:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

LOCAL_OUTDIR := lib
LOCAL_LIBRARY := Lore

LOCAL_TOOLKIT := serenity
LOCAL_INSTALL_DIR := $(LOCAL_OUTDIR)

LOCAL_ROOT = .

LOCAL_SRCS_DIRS := $(DOCUMENT_SOURCE_DIR_COMMON) src users
LOCAL_SRCS_OBJS :=

LOCAL_INCLUDES_DIRS := src users
LOCAL_INCLUDES_OBJS := $(DOCUMENT_INCLUDE_COMMON) $(CONFIG_INCLUDE)

LOCAL_CFLAGS = 
LOCAL_LIBS = $(STAPPLER_ROOT)/libs/linux/x86_64/lib/libhyphen.a

LOCAL_FORCE_INSTALL := 1
LOCAL_OPTIMIZATION := -g -O2

$(LOCAL_OUTDIR)/httpd.conf: Makefile
	@mkdir -p $(LOCAL_OUTDIR)
	@mkdir -p $(CONF_DIR)logs
	@echo '# Autogenerated by makefile\n' > $@
	@echo 'ServerRoot "$(CONF_DIR)"' >> $@
	@echo 'LoadModule serenity_module $(abspath $(SERENITY_OUTPUT))' >> $@
	@echo 'ErrorLog "$(CONF_DIR)logs/error_log"' >> $@
	@echo 'CustomLog "$(CONF_DIR)logs/access_log" common' >> $@
	@echo 'SerenitySourceRoot "$(CONF_DIR)lib"' >> $@
	@echo 'Include $(CONF_DIR)conf/httpd-default.conf' >> $@
	@echo 'Include $(CONF_DIR)conf/serenity-$(CONF).conf' >> $@
	@echo 'Include $(CONF_DIR)conf/sites-$(CONF)/*.conf' >> $@

include $(STAPPLER_ROOT)/make/local.mk

all: $(LOCAL_OUTDIR)/httpd.conf
