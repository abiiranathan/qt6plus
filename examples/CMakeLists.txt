add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(treeview ${CMAKE_CURRENT_SOURCE_DIR}/treeview.cpp)
add_executable(mediaPlayer ${CMAKE_CURRENT_SOURCE_DIR}/mediaPlayer.cpp)


target_link_libraries(treeview PRIVATE qt6plus)
target_link_libraries(main PRIVATE qt6plus bcrypt)
target_link_libraries(mediaPlayer PRIVATE qt6plus)
