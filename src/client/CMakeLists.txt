aux_source_directory(. SRC_LIST)
aux_source_directory(../entity ENTITY_LIST)

add_executable(ChatClient ${SRC_LIST} ${ENTITY_LIST})
target_link_libraries(ChatClient pthread)