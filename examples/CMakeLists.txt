# SPDX-License-Identifier: MIT
add_executable(eigenqtGallery gallery.cpp)
target_link_libraries(eigenqtGallery PRIVATE EigenQt::Geometry)
add_executable(eigenqtScreenshots EXCLUDE_FROM_ALL ../doc/tools/capture_widgets.cpp)
target_link_libraries(eigenqtScreenshots PRIVATE EigenQt::Geometry)
