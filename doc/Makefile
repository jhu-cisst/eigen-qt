# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 EigenQt contributors
SPHINXBUILD ?= sphinx-build
BUILDDIR ?= _build

.PHONY: html clean
html:
	$(SPHINXBUILD) -W --keep-going -b html . $(BUILDDIR)/html

clean:
	$(SPHINXBUILD) -M clean . $(BUILDDIR)
