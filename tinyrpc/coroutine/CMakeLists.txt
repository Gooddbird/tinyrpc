set(HEADERS
    coctx.h
    coroutine.h
    coroutine_hook.h
    coroutine_pool.h
    memory.h
)

install(FILES ${HEADERS} DESTINATION include/tinyrpc/coroutine)