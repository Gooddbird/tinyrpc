set(HEADERS
    abstract_codec.h
    byte.h
    mutex.h
    abstract_data.h   
    fd_event.h
    net_address.h
    timer.h
    abstract_dispatcher.h
    reactor.h
)

install(FILES ${HEADERS} DESTINATION include/tinyrpc/net)