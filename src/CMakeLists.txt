cmake_minimum_required(VERSION 3.2)

set(LOCAL_SOURCE_FILES
    slink.cpp	
    bits.cpp
    socketfun.cpp
    packet.cpp	
    socketfun.cpp   	
    netconn_pack.cpp
    accept_client.cpp
    acceptor.cpp
    synfile.cpp
    netclient.cpp
    winque.cpp  
    netconn.cpp
    main.cpp
) 


add_executable(${EXEC_NAME} ${LOCAL_SOURCE_FILES})
target_link_libraries(${EXEC_NAME} pthread)
install(TARGETS ${EXEC_NAME} DESTINATION ${EXECSVR_INSTALL_BIN_DIR})

