set(HEADERS
    ftp_define.h
    ftp_codec.h
    ftp_handler.h
    ftp_dispatcher.h
    ftp_server.h
)

install(FILES ${HEADERS} DESTINATION include/tinyrpc/net/ftp)
