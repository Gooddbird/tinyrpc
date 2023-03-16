set(HEADERS
    tinypb_codec.h             
    tinypb_rpc_closure.h
    tinypb_data.h               
    tinypb_rpc_controller.h
    tinypb_rpc_async_channel.h 
    tinypb_rpc_dispatcher.h
    tinypb_rpc_channel.h
)

install(FILES ${HEADERS} DESTINATION include/tinyrpc/net/tinypb)