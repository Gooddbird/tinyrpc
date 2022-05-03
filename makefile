##################################
# makefile
# ikerli
# 2022-05-23
##################################

PATH_BIN = bin
PATH_LIB = lib

PATH_TINYRPC = tinyrpc
PATH_COMM = $(PATH_TINYRPC)/comm
PATH_COROUTINE = $(PATH_TINYRPC)/coroutine
PATH_NET = $(PATH_TINYRPC)/net
PATH_HTTP = $(PATH_TINYRPC)/net/http
PATH_TCP = $(PATH_TINYRPC)/net/tcp
PATH_TINYPB = $(PATH_TINYRPC)/net/tinypb

PATH_TESTCASES = testcases

# PATH_PROTOBUF = /usr/include/google
# PATH_TINYXML = /usr/include/tinyxml

CXX := g++

CXXFLAGS += -g -O -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

CXXFLAGS += -I$(PATH_TINYRPC)	-I$(PATH_COMM) -I$(PATH_COROUTINE) -I$(PATH_NET) -I$(PATH_HTTP) -I$(PATH_TCP) -I$(PATH_TINYPB)

LIBS += /usr/lib/libprotobuf.a	/usr/lib/libtinyxml.a

COMM_OBJ := $(patsubst $(PATH_COMM)/%.cc, $(PATH_COMM)/%.o, $(wildcard $(PATH_COMM)/*.cc))
COROUTINE_OBJ := $(patsubst $(PATH_COROUTINE)/%.cc, $(PATH_COROUTINE)/%.o, $(wildcard $(PATH_COROUTINE)/*.cc))
NET_OBJ := $(patsubst $(PATH_NET)/%.cc, $(PATH_NET)/%.o, $(wildcard $(PATH_NET)/*.cc))
HTTP_OBJ := $(patsubst $(PATH_HTTP)/%.cc, $(PATH_HTTP)/%.o, $(wildcard $(PATH_HTTP)/*.cc))
TCP_OBJ := $(patsubst $(PATH_TCP)/%.cc, $(PATH_TCP)/%.o, $(wildcard $(PATH_TCP)/*.cc))
TINYPB_OBJ := $(patsubst $(PATH_TINYPB)/%.cc, $(PATH_TINYPB)/%.o, $(wildcard $(PATH_TINYPB)/*.cc))

COR_CTX_SWAP := coctx_swap.o

ALL_TESTS : $(PATH_BIN)/test_connect $(PATH_BIN)/test_coroutine_hook $(PATH_BIN)/test_coroutine $(PATH_BIN)/test_iothread\
	$(PATH_BIN)/test_log $(PATH_BIN)/test_reactor $(PATH_BIN)/test_rpc_server1 $(PATH_BIN)/test_rpc_server2\
	$(PATH_BIN)/test_tcpserver $(PATH_BIN)/test_thread_pool

TEST_CASE_OUT := $(PATH_BIN)/test_connect $(PATH_BIN)/test_coroutine_hook $(PATH_BIN)/test_coroutine $(PATH_BIN)/test_iothread\
	$(PATH_BIN)/test_log $(PATH_BIN)/test_reactor $(PATH_BIN)/test_rpc_server1 $(PATH_BIN)/test_rpc_server2\
	$(PATH_BIN)/test_tcpserver $(PATH_BIN)/test_thread_pool

LIB_OUT := $(PATH_LIB)/libtinyrpc.a

$(PATH_BIN)/test_log : $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_connect : $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_connect.cc $(PATH_TESTCASES)/tinypb.pb.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_coroutine_hook: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_coroutine_hook.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_coroutine: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_coroutine.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_iothread: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_iothread.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_reactor: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_reactor.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_tcpserver: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_tcpserver.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_thread_pool: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_thread_pool.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_rpc_server1: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_server1.cc $(PATH_TESTCASES)/tinypb.pb.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_BIN)/test_rpc_server2: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_server2.cc $(PATH_TESTCASES)/tinypb.pb.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -lpthread

$(PATH_LIB)/libtinyrpc.a : $(COMM_OBJ) $(COROUTINE_OBJ) $(PATH_COROUTINE)/coctx_swap.o $(NET_OBJ) $(HTTP_OBJ) $(TCP_OBJ) $(TINYPB_OBJ)
	@ar crsvT $@ $^

$(PATH_COMM)/%.o : $(PATH_COMM)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_COROUTINE)/%.o : $(PATH_COROUTINE)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_COROUTINE)/coctx_swap.o : $(PATH_COROUTINE)/coctx_swap.S
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_NET)/%.o : $(PATH_NET)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_HTTP)/%.o : $(PATH_HTTP)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_TCP)/%.o : $(PATH_TCP)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_TINYPB)/%.o : $(PATH_TINYPB)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@


# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)


# to clean 
clean :
	rm -f $(COMM_OBJ) $(COROUTINE_OBJ) $(NET_OBJ) $(HTTP_OBJ) $(TCP_OBJ) $(TINYPB_OBJ) $(TESTCASES) $(PATH_COROUTINE)/coctx_swap.o $(TEST_CASE_OUT) $(PATH_LIB)/libtinyrpc.a