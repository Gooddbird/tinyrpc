set(HEADERS
    config.h
    error_code.h
    log.h
    msg_req.h
    mysql_instase.h
    run_time.h
    start.h
    string_util.h
    thread_pool.h
)

install(FILES ${HEADERS} DESTINATION include/tinyrpc/comm)