# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 EigenQt contributors
"""Sphinx configuration; building the manual does not require Qt or CMake."""

from pathlib import Path
import re

project = "EigenQt"
author = "EigenQt contributors"
copyright = "2010–2026, Johns Hopkins University and EigenQt contributors"
release = re.search(
    r"project\(EigenQt VERSION ([0-9.]+)",
    (Path(__file__).resolve().parents[1] / "CMakeLists.txt").read_text(),
).group(1)
version = release
needs_sphinx = "7.2"
extensions = []
source_suffix = {".rst": "restructuredtext"}
root_doc = "index"
exclude_patterns = ["_build", "tools", "requirements.txt"]
default_domain = "cpp"
highlight_language = "cpp"
nitpicky = True
html_theme = "sphinx_rtd_theme"
html_theme_options = {"navigation_depth": 3, "collapse_navigation": False}
html_title = f"{project} {release}"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_show_sourcelink = True
