#!/usr/bin/python3.7

import string
import sys
import os
import getopt
import traceback

from string import Template
from datetime import datetime

out_project_path = './'
project_name = ''
proto_file = ''
src_path = ''
conf_path = ''
bin_path = ''
test_client_path = ''
test_client_tool_path = ''

IP = '0.0.0.0'
PORT = 39999

generator_path = sys.path[0]



def to_camel(input_s):
    if input_s.find('_') == -1:
        return input_s
    re = ''
    for s in input_s.split('_'):
        re += s.capitalize()
    return re

def to_underline(input_s):
    tmp = to_camel(input_s)
    re = ''
    for s in tmp:
        re += s if s.islower() else '_' + s.lower()
    re = re[1:]
    return re

def generate_framework_code(): 
    print('=' * 100)
    print('Begin to generate tinyrpc framework code')
    pb_head_file = src_path + '/pb/' + project_name + '.pb.h'
    file = open(pb_head_file, 'r')
    origin_text = file.read()
    
    # parse all rpc interface method from pb.h file
    begin = origin_text.find('virtual ~')
    i1 = origin_text[begin:].find('~') 
    i2 = origin_text[begin:].find('(') 
    service_name = origin_text[begin + i1 + 1 : begin + i2]
    print("service name is " + service_name)

    origin_text = origin_text[begin + i2: ] 
    method_list = []

    i1 = 0
    while 1:
        i1 = origin_text.find('virtual void')
        if (i1 == -1):
            break
        i2 = origin_text[i1:].find(');')
        method_list.append(origin_text[i1: i1 + i2 + 2])
        # print(origin_text[i1: i1 + i2 + 2])
        origin_text = origin_text[i1 + i2 + 3: ]

    
    print('=' * 100)
    print('Begin generate business_exception.h')
    exception_file = src_path + '/comm/' + 'business_exception.h'
    if not os.path.exists(exception_file):
        # generate business_exception.h
        exception_template = Template(open(generator_path + '/template/business_exception.h.template', 'r').read())
        exception_content = exception_template.safe_substitute(
            PROJECT_NAME = project_name,
            FILE_NAME = 'business_exception.cc',
            HEADER_DEFINE = project_name.upper() + '_COMM_BUSINESSEXCEPTION_H',
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        )
        out_exception_file = open(exception_file, 'w')
        out_exception_file.write(exception_content)
        out_exception_file.close()
    else:
        print("file: [" + exception_file + "] exist, skip")

    print('End generate business_exception.h')
    print('=' * 100)


    print('=' * 100)
    print('Begin generate server.h')
    # genneator server.h file
    class_name = to_camel(service_name) + 'Impl'
    head_file_temlate = Template(open(generator_path + '/template/server.h.template','r').read())
    head_file_content = head_file_temlate.safe_substitute(
        HEADER_DEFINE = project_name.upper() + '_SERVICE_' + project_name.upper() + '_H',
        FILE_NAME = project_name + '.h',
        PROJECT_NAME = project_name,
        CLASS_NAME = class_name,
        SERVICE_NAME = service_name,
        PB_HEAD_FILE = project_name + '/pb/' + project_name + '.pb.h', 
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
    )

    i1 = head_file_content.find('${METHOD}') 
    pre_content = head_file_content[0: i1]
    next_content = head_file_content[i1 + 9: ]
    for each in method_list:
        each = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
        pre_content += '// override from ' + service_name + '\n  '
        pre_content += each
        pre_content += '\n\n  '
    content = pre_content + next_content
    out_head_file = open(src_path + '/service/' + project_name + '.h', 'w')
    out_head_file.write(content)
    out_head_file.close()

    print('End generate server.h')
    print('=' * 100)

    print('=' * 100)
    print('Begin generate server.cc')
    # genneator server.cc file
    cc_file_temlate = Template(open(generator_path + '/template/server.cc.template','r').read())
    cc_file_content = cc_file_temlate.safe_substitute(
        FILE_NAME = project_name + '.cc',
        PROJECT_NAME = project_name,
        INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
        INCLUDE_BUSINESS_EXCEPTION_HEADER = '#include "' + project_name + '/comm/business_exception.h"',
        INCLUDE_SERVER_HEADER = '#include "' + project_name + '/service/' + project_name + '.h"', 
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    )


    method_i = cc_file_content.find('${METHOD}')
    pre_content = cc_file_content[0: method_i]
    next_content = cc_file_content[method_i + 9: ]
    interface_list = []

    for each in method_list:
        tmp = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
        i1 = tmp.find('void')
        tmp = tmp[i1:]

        i2 = tmp.find('(')
        method_name = tmp[5: i2]
        # print(method_name)
        interface_class_name = to_camel(method_name) + 'Interface'
        interface_file_name = to_underline(method_name)
        l = tmp.split(',')
        y = l[1].find('request')
        request_type = l[1][0: y - 1].replace('*', '').replace('const ', '').replace('\n', '').replace(' ', '')
        # print(request_type)

        y = l[2].find('response')
        response_type = l[2][0: y - 1].replace('*', '').replace('\n', '').replace(' ', '')
        # print(response_type)


        interface_list.append({
            'interface_name': interface_file_name,
            'method_name': method_name,
            'interface_class_name': interface_class_name,
            'request_type': request_type,
            'response_type': response_type
        })
        # print(interface_list)

        tmp = tmp[0: 5] + class_name + '::' + tmp[5:]
        tmp = tmp[0: -1] + '{\n\n  ' + 'CALL_RPC_INTERFACE(' + interface_class_name + ');\n}'
        # print(tmp)
        pre_content += tmp
        pre_content += '\n\n'

    include_str = ''
    for each in interface_list:
        include_str += '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"\n'
    pre_content = pre_content.replace('${INCLUDE_SERVICE}', include_str)
    
    out_cc_file = open(src_path + '/service/' + project_name + '.cc', 'w')
    out_cc_file.write(pre_content + next_content)
    out_cc_file.close()

    print('End generate server.cc')
    print('=' * 100)


    print('=' * 100)
    print('Begin generate main.cc')
    # genneator main.cc file
    main_file = src_path + '/main.cc'
    if not os.path.exists(main_file):
        main_file_temlate = Template(open(generator_path + '/template/main.cc.template','r').read())
        main_file_content = main_file_temlate.safe_substitute(
            FILE_NAME = project_name + '.h',
            PROJECT_NAME = project_name,
            CLASS_NAME = class_name,
            INCLUDE_SERVER_HEADER = '#include "' + project_name + '/service/' + project_name + '.h"', 
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        )
        main_file_handler = open(main_file, 'w')
        main_file_handler.write(main_file_content)
        main_file_handler.close()
    else:
        print("file: [" + main_file + "] exist, skip")

    print('End generate main.cc')
    print('=' * 100)

    print('=' * 100)
    print('Begin generate each interface.cc & interface.h')
    # genneator each interface.cc and .h file
    interface_head_file_temlate = Template(open(generator_path + '/template/interface.h.template','r').read())
    interface_cc_file_temlate = Template(open(generator_path + '/template/interface.cc.template','r').read())
    interface_test_client_file_template = Template(open(generator_path + '/template/test_tinyrpc_client.cc.template','r').read())

    stub_name = service_name + "_Stub"
    for each in interface_list:

        # interface.h 
        file = src_path + '/interface/' + each['interface_name'] + '.h'
        if not os.path.exists(file):
            header_content = interface_head_file_temlate.safe_substitute(
                PROJECT_NAME = project_name,
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                HEADER_DEFINE = project_name.upper() + '_INTERFACE_' + each['interface_name'].upper() + '_H',
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                CLASS_NAME = each['interface_class_name'],
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                FILE_NAME = each['interface_name'] + '.h'
            )
            out_interface_header_file = open(file, 'w')
            out_interface_header_file.write(header_content)
            out_interface_header_file.close()
        else:
            print("file: [" + file + "] exist, skip")

        # interface.cc 
        file = src_path + '/interface/' + each['interface_name'] + '.cc'
        if not os.path.exists(file):
            cc_file_content = interface_cc_file_temlate.safe_substitute(
                PROJECT_NAME = project_name,
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                INCLUDE_INTERFACE_HEADER_FILE = '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"',
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                CLASS_NAME = each['interface_class_name'],
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                FILE_NAME = each['interface_name'] + '.cc'
            )
            out_interface_cc_file = open(file, 'w')
            out_interface_cc_file.write(cc_file_content)
            out_interface_cc_file.close()
        else:
            print("file: [" + file + "] exist, skip")
       
        # test_interface_client.cc 
        file = test_client_path + '/test_' + each['interface_name'] + '_client.cc'
        if not os.path.exists(file):
            cc_file_content = interface_test_client_file_template.safe_substitute(
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                STUBCLASS = stub_name,
                METHOD_NAME = each['method_name'],
                FILE_NAME = 'test_' + each['interface_name'] + '_client.cc',
                IP = IP,
                PORT = PORT,
            )
            out_interface_cc_file = open(file, 'w')
            out_interface_cc_file.write(cc_file_content)
            out_interface_cc_file.close()
        else:
            print("file: [" + file + "] exist, skip")
            
        

    print('End generate each interface.cc & interface.h & test_interface_client.h')
    print('=' * 100)
        
    print('End generate tinyrpc framework code')
    print('=' * 100)
        
        

def gen_pb_files():
    print('=' * 100)
    print('Begin generate protobuf file')
    pb_path = src_path + '/pb/'
    cmd = 'cp -r ' + proto_file + ' ' + pb_path
    cmd += ' && cd ' + pb_path + ' && protoc --cpp_out=./ ' + proto_file 
    print('excute cmd: ' + cmd)

    if os.system(cmd) is not 0:
        raise Exception("execute cmd failed [" + cmd + "]")
    
    print('End generate protobuf file')
    print('=' * 100)

def gen_makefile():
    print('=' * 100)
    print('Begin to generate makefile')
    out_file = src_path + '/makefile'
    if os.path.exists(out_file):
        print('makefile exist, skip generate')
        print('End generate makefile')
        print('=' * 100)
        return 
    
    template_file = open(generator_path + '/template/makefile.template','r')
    # print(template_file.read())
    tmpl = Template(template_file.read())

    content = tmpl.safe_substitute(
        PROJECT_NAME = project_name,
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

    file = open(out_file, 'w')
    file.write(content)
    file.close()
    print('succ write to ' + out_file)
    print('End generate makefile')
    print('=' * 100)

def gen_conf_file():
    print('=' * 100)
    print('Begin to generate tinyrpc conf file')
    out_file = conf_path + '/' + project_name + '.xml'
    if os.path.exists(out_file):
        print('makefile exist, skip generate')
        print('End generate makefile')
        print('=' * 100)
        return 
    
    template_file = open(generator_path + '/template/conf.xml.template','r')
    # print(template_file.read())
    tmpl = Template(template_file.read())

    content = tmpl.safe_substitute(
        PROJECT_NAME = project_name,
        IP = IP,
        PORT = str(PORT),
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

    file = open(out_file, 'w')
    file.write(content)
    file.close()
    print('succ write to ' + out_file)
    print('End generate tinyrpc conf file')
    print('=' * 100)
    
    
def gen_run_script():
    print('=' * 100)
    print('Begin to generate run script')
    script = open(generator_path + '/template/conf.xml.template','r')
    dir_src = generator_path + '/template/'
    cmd = 'cp -r ' + dir_src + '*.sh ' + bin_path + "/" 
    print('excute cmd: ' + cmd)
    os.system(cmd)

    print('End generate run script')
    print('=' * 100)
    
    

def generate_dir():
    print('=' * 100)
    print('Begin to generate project dir')

    if out_project_path == "":
        proj_path = './' + project_name.strip()
    if out_project_path[-1] == '/':
        proj_path = out_project_path + project_name.strip()
    else:
        proj_path = out_project_path + './' + project_name.strip()
    
    global bin_path 
    bin_path = proj_path + '/bin'

    global conf_path
    conf_path = proj_path + '/conf'

    global test_client_path 
    test_client_path = proj_path + '/test_client'

    global test_client_tool_path 
    test_client_tool_path = test_client_path + '/test_tool'

    log_path = proj_path + '/log'
    lib_path = proj_path + '/lib'
    obj_path = proj_path + '/obj'

    global src_path
    src_path = proj_path + '/' + project_name
    src_interface_path = src_path + '/interface'
    src_service_path = src_path + '/service'
    src_pb_path = src_path + '/pb'
    src_comm_path = src_path + '/comm'

    dir_list = []
    dir_list.append(proj_path) 
    dir_list.append(bin_path) 
    dir_list.append(conf_path) 
    dir_list.append(log_path) 
    dir_list.append(lib_path) 
    dir_list.append(obj_path) 
    dir_list.append(test_client_path) 
    dir_list.append(test_client_tool_path) 
    dir_list.append(src_path) 
    dir_list.append(src_interface_path) 
    dir_list.append(src_service_path) 
    dir_list.append(src_pb_path) 
    dir_list.append(src_comm_path) 

    for each in dir_list:
        if not os.path.exists(each):
            os.mkdir(each)
            print("succ make dir in " + each)

    print('End generate project dir')
    print('=' * 100)
    

def generate_tinyrpc_project():
    try:

        parseInput()

        print('=' * 150)
        print('Begin generate tinyrpc project')

        generate_dir()

        gen_pb_files()

        gen_makefile()

        gen_run_script()

        gen_conf_file()

        generate_framework_code()

        print('Succ generate tinyrpc project')
        print('=' * 150)

    except Exception as e:
        print('Falied generate tinyrpc project, err:')
        print(e)
        traceback.print_exc()
        print('=' * 150)
    finally:
        pass


def printHelp():
    print('=' * 100)
    print('Welcome to use TinyRPC Generator, this is help document:\n')
    print('Run Environment: Python(version 3.6 or high version is better).')
    print('Run Platform: Linux Only(kernel version >= 3.9 is better).')
    print('Others: Only protobuf3 support, not support protobuf2.\n')
    print('Usage:')
    print('tinyrpc_generator.py -[options][target]\n')
    print('Options:')
    print('-h, --help')
    print(('    ') + 'Print help document.\n')

    print('-i xxx.proto, --input xxx.proto')
    print(('    ') + 'Input the target proto file, must standard protobuf3 file, not support protobuf2.\n')

    print('-o dir, --output dir')
    print(('    ') + 'Set the path that your want to generate project, please give a dir param.\n')

    print('-p port, --input port')
    print(('    ') + 'Set the port(default 39999) of tinyrpc server, you should input a short integer.\n')

    print('-h x.x.x.x, --host x.x.x.x')
    print(('    ') + 'Set the ip(default 0.0.0.0) of tinyrpc server.\n')
    
    print('')
    print('For example:')
    print('tinyrpc_generator.py -i order_server.proto -o ./ -p 12345 -h 127.0.0.1')

    print('')


    print('=' * 100)
    

def parseInput():

    opts,args=getopt.getopt(sys.argv[1:],"hi:o:p:h:",["help","input=","output="])
  
    for opts,arg in opts:
        if opts=="-h" or opts=="--help":
            printHelp()
            sys.exit(0)
        if opts=="-i" or opts=="--input":
            global proto_file 
            proto_file = arg 
        elif opts=="-o" or opts=="--output_path":
            global out_project_path
            out_project_path = arg
            if out_project_path[-1] != '/':
                out_project_path = out_project_path + '/'
        elif opts=="-p" or opts=="--port":
            global PORT 
            PORT = int(arg)
            if PORT <= 0 or PORT > 65536 :
                raise Exception("invalid port: [" + PORT + "]")
                
        elif opts=="-h" or opts=="--host":
            global IP 
            IP = arg
        else:
            raise Exception("invalid options: [" + opts + ": " + arg + "]")

    if not os.path.exists(proto_file):
        raise Exception("Generate error, not exist protobuf file: " + proto_file)

    if ".proto" not in proto_file:
        raise Exception("Generate error, input file is't standard protobuf file:[" + proto_file + "]")

    global project_name
    project_name = proto_file[0: -6]
    print("project name is " + project_name)




 

if __name__ == '__main__':
    generate_tinyrpc_project()