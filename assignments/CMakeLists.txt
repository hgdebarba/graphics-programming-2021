# obtain the list of subdirectories
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})


# add all subdirectories (it expects a CMakeLists.txt file in all of them)
FOREACH(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
ENDFOREACH()