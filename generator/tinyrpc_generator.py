#!/usr/bin/python3.7

import string
import sys
import os
import getopt
import traceback

from string import Template
from datetime import datetime


class Generator:
    def __init__(self):
        self.out_project_path = './'
        self.project_name = ''
        self.proto_file = ''


        self.proj_path = ''
        self.log_path = ''
        self.lib_path = ''
        self.obj_path = ''

        self.src_path = ''
        self.src_interface_path = ''
        self.src_service_path = ''
        self.src_pb_path = ''
        self.src_comm_path = ''

        self.conf_path = ''
        self.bin_path = ''
        self.test_client_path = ''
        self.test_client_tool_path = ''

        self.generator_path = sys.path[0]

        self.service_name = ''
        self.method_list = []
        self.interface_list = []

        self.INCLUDE_BUSINESS_EXCEPTION_HEADER = ''
        self.INCLUDE_PB_HEADER = ''
        self.INCLUDE_INTERFACEBASE_HEADER = ''
        self.class_name = ''
    
    
    def generate_dir(self):
        print('=' * 100)
        print('Begin to generate project dir')

        if self.out_project_path == "":
            self.proj_path = './' + self.project_name.strip()
        if self.out_project_path[-1] == '/':
            self.proj_path = self.out_project_path + self.project_name.strip()
        else:
            self.proj_path = self.out_project_path + './' + self.project_name.strip()
        
        self.bin_path = self.proj_path + '/bin'
        self.conf_path = self.proj_path + '/conf'
        self.test_client_path = self.proj_path + '/test_client'
        self.test_client_tool_path = self.test_client_path + '/test_tool'

        self.log_path = self.proj_path + '/log'
        self.lib_path = self.proj_path + '/lib'
        self.obj_path = self.proj_path + '/obj'

        self.src_path = self.proj_path + '/' + self.project_name
        self.src_interface_path = self.src_path + '/interface'
        self.src_service_path = self.src_path + '/service'
        self.src_pb_path = self.src_path + '/pb'
        self.src_comm_path = self.src_path + '/comm'

        dir_list = []
        dir_list.append(self.proj_path) 
        dir_list.append(self.bin_path) 
        dir_list.append(self.conf_path) 
        dir_list.append(self.log_path) 
        dir_list.append(self.lib_path) 
        dir_list.append(self.obj_path) 
        dir_list.append(self.test_client_path) 
        dir_list.append(self.test_client_tool_path) 
        dir_list.append(self.src_path) 
        dir_list.append(self.src_interface_path) 
        dir_list.append(self.src_service_path) 
        dir_list.append(self.src_pb_path) 
        dir_list.append(self.src_comm_path) 

        for each in dir_list:
            if not os.path.exists(each):
                os.mkdir(each)
                print("succ make dir in " + each)

        print('End generate project dir')
        print('=' * 100)
        


    def to_camel(self, input_s):
        if input_s.find('_') == -1:
            return input_s
        re = ''
        for s in input_s.split('_'):
            re += s.capitalize()
        return re

    def to_underline(self, input_s):
        tmp = self.to_camel(input_s)
        re = ''
        for s in tmp:
            re += s if s.islower() else '_' + s.lower()
        re = re[1:]
        return re

    def gen_include_name(self):
        self.INCLUDE_PB_HEADER = '#include "' + self.project_name + '/pb/' + self.project_name + '.pb.h"'
        self.INCLUDE_SERVER_HEADER = '#include "' + self.project_name + '/service/' + self.project_name + '.h"'
        self.INCLUDE_INTERFACEBASE_HEADER = '#include "' + self.project_name + '/interface/interface.h"'
        self.INCLUDE_BUSINESS_EXCEPTION_HEADER = '#include "' + self.project_name + '/comm/business_exception.h"'
        

    def parse_service_name(self):
        pb_head_file = self.src_pb_path + '/' + self.project_name + '.pb.h'
        file = open(pb_head_file, 'r')
        origin_text = file.read()
        
        # parse all rpc interface method from pb.h file
        begin = origin_text.find('virtual ~')
        i1 = origin_text[begin:].find('~') 
        i2 = origin_text[begin:].find('(') 
        self.service_name = origin_text[begin + i1 + 1 : begin + i2]
        print("service name is " + self.service_name)

        origin_text = origin_text[begin + i2: ] 

        i1 = 0
        while 1:
            i1 = origin_text.find('virtual void')
            if (i1 == -1):
                break
            i2 = origin_text[i1:].find(');')
            self.method_list.append(origin_text[i1: i1 + i2 + 2])
            # print(origin_text[i1: i1 + i2 + 2])
            origin_text = origin_text[i1 + i2 + 3: ]
    
    def gen_exception(self):
        print('=' * 100)
        print('Begin generate business_exception.h')
        exception_file = self.src_comm_path + '/business_exception.h'
        project_name = self.project_name
        if not os.path.exists(exception_file):
            # generate business_exception.h
            exception_template = Template(open(self.generator_path + '/template/business_exception.h.template', 'r').read())
            exception_content = exception_template.safe_substitute(
                PROJECT_NAME = project_name,
                FILE_NAME = 'business_exception.h',
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
    
    def gen_server(self):
        print('=' * 100)
        print('Begin generate server.h')
        # genneator server.h file
        self.class_name = self.to_camel(self.service_name) + 'Impl'
        project_name = self.project_name
        service_name = self.service_name
        head_file_temlate = Template(open(self.generator_path + '/template/server.h.template','r').read())
        head_file_content = head_file_temlate.safe_substitute(
            HEADER_DEFINE = project_name.upper() + '_SERVICE_' + project_name.upper() + '_H',
            FILE_NAME = project_name + '.h',
            PROJECT_NAME = project_name,
            CLASS_NAME = self.class_name,
            SERVICE_NAME = service_name,
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            INCLUDE_PB_HEADER = self.INCLUDE_PB_HEADER, 
        )

        i1 = head_file_content.find('${METHOD}') 
        pre_content = head_file_content[0: i1]
        next_content = head_file_content[i1 + 9: ]
        for each in self.method_list:
            each = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
            pre_content += '// override from ' + service_name + '\n  '
            pre_content += each
            pre_content += '\n\n  '
        content = pre_content + next_content
        out_head_file = open(self.src_path + '/service/' + project_name + '.h', 'w')
        out_head_file.write(content)
        out_head_file.close()

        print('End generate server.h')
        print('=' * 100)

        print('=' * 100)
        print('Begin generate server.cc')
        # genneator server.cc file
        cc_file_temlate = Template(open(self.generator_path + '/template/server.cc.template','r').read())
        cc_file_content = cc_file_temlate.safe_substitute(
            FILE_NAME = project_name + '.cc',
            PROJECT_NAME = project_name,
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            INCLUDE_PB_HEADER = self.INCLUDE_PB_HEADER,
            INCLUDE_BUSINESS_EXCEPTION_HEADER = self.INCLUDE_BUSINESS_EXCEPTION_HEADER,
            INCLUDE_SERVER_HEADER = self.INCLUDE_SERVER_HEADER,
            INCLUDE_INTERFACEBASE_HEADER = self.INCLUDE_INTERFACEBASE_HEADER
        )

        method_i = cc_file_content.find('${METHOD}')
        pre_content = cc_file_content[0: method_i]
        next_content = cc_file_content[method_i + 9: ]
        self.interface_list = []

        for each in self.method_list:
            tmp = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
            i1 = tmp.find('void')
            tmp = tmp[i1:]

            i2 = tmp.find('(')
            method_name = tmp[5: i2]
            # print(method_name)
            interface_class_name = self.to_camel(method_name) + 'Interface'
            interface_file_name = self.to_underline(method_name)
            l = tmp.split(',')
            y = l[1].find('request')
            request_type = l[1][0: y - 1].replace('*', '').replace('const ', '').replace('\n', '').replace(' ', '')
            # print(request_type)

            y = l[2].find('response')
            response_type = l[2][0: y - 1].replace('*', '').replace('\n', '').replace(' ', '')
            # print(response_type)


            self.interface_list.append({
                'interface_name': interface_file_name,
                'method_name': method_name,
                'interface_class_name': interface_class_name,
                'request_type': request_type,
                'response_type': response_type
            })

            tmp = tmp[0: 5] + self.class_name + '::' + tmp[5:]
            tmp = tmp[0: -1] + '{\n\n  ' + 'CALL_RPC_INTERFACE(' + interface_class_name + ');\n}'
            # print(tmp)
            pre_content += tmp
            pre_content += '\n\n'

        include_str = ''
        for each in self.interface_list:
            include_str += '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"\n'
        pre_content = pre_content.replace('${INCLUDE_SERVICE_HEADER}', include_str)
        
        out_cc_file = open(self.src_path + '/service/' + project_name + '.cc', 'w')
        out_cc_file.write(pre_content + next_content)
        out_cc_file.close()

        print('End generate server.cc')
        print('=' * 100)


    
    def gen_main(self):
        print('=' * 100)
        print('Begin generate main.cc')
        # genneator main.cc file
        main_file = self.src_path + '/main.cc'
        project_name = self.project_name

        if not os.path.exists(main_file):
            main_file_temlate = Template(open(self.generator_path + '/template/main.cc.template','r').read())
            main_file_content = main_file_temlate.safe_substitute(
                FILE_NAME = project_name + '.cc',
                PROJECT_NAME = project_name,
                CLASS_NAME = self.class_name,
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                INCLUDE_SERVER_HEADER = self.INCLUDE_SERVER_HEADER,
            )
            main_file_handler = open(main_file, 'w')
            main_file_handler.write(main_file_content)
            main_file_handler.close()
        else:
            print("file: [" + main_file + "] exist, skip")

        print('End generate main.cc')
        print('=' * 100)
    
    def gen_interface_base(self):
        print('=' * 100)
        print('Begin generate interface.h')
        header_file = self.src_path + '/interface/interface.h'
        if not os.path.exists(header_file):
            file_temlate = Template(open(self.generator_path + '/template/interface_base.h.template','r').read())
            file_content = file_temlate.safe_substitute(
                FILE_NAME = self.project_name + '.h',
                PROJECT_NAME = self.project_name,
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                HEADER_DEFINE = self.project_name.upper() + '_INTERFACE_INTERFACE_H',
            )
            file_handler = open(header_file, 'w')
            file_handler.write(file_content)
            file_handler.close()
        else:
            print("file: [" + header_file + "] exist, skip")

        print('End generate interface.h')
        print('=' * 100)
    
        print('=' * 100)
        print('Begin generate interface.cc')
        cc_file = self.src_path + '/interface/interface.cc'
        if not os.path.exists(cc_file):
            file_temlate = Template(open(self.generator_path + '/template/interface_base.cc.template','r').read())
            file_content = file_temlate.safe_substitute(
                FILE_NAME = self.project_name + '.h',
                PROJECT_NAME = self.project_name,
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                INCLUDE_INTERFACEBASE_HEADER = self.INCLUDE_INTERFACEBASE_HEADER
            )
            file_handler = open(cc_file, 'w')
            file_handler.write(file_content)
            file_handler.close()
        else:
            print("file: [" + cc_file + "] exist, skip")

        print('End generate interface.cc')
        print('=' * 100)
        
    
    def gen_each_interface(self):
        print('=' * 100)
        print('Begin generate each interface.cc & interface.h')
        generator_path = self.generator_path
        project_name = self.project_name

        # genneator each interface.cc and .h file
        interface_head_file_temlate = Template(open(generator_path + '/template/interface.h.template','r').read())
        interface_cc_file_temlate = Template(open(generator_path + '/template/interface.cc.template','r').read())
        interface_test_client_file_template = Template(open(generator_path + '/template/test_tinyrpc_client.cc.template','r').read())

        stub_name = self.service_name + "_Stub"
        for each in self.interface_list:

            # interface.h 
            file = self.src_path + '/interface/' + each['interface_name'] + '.h'
            if not os.path.exists(file):
                header_content = interface_head_file_temlate.safe_substitute(
                    PROJECT_NAME = project_name,
                    INCLUDE_PB_HEADER = self.INCLUDE_PB_HEADER, 
                    HEADER_DEFINE = project_name.upper() + '_INTERFACE_' + each['interface_name'].upper() + '_H',
                    CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                    CLASS_NAME = each['interface_class_name'],
                    REQUEST_TYPE = each['request_type'],
                    RESPONSE_TYPE = each['response_type'],
                    FILE_NAME = each['interface_name'] + '.h',
                    INCLUDE_INTERFACEBASE_HEADER = self.INCLUDE_INTERFACEBASE_HEADER
                )
                out_interface_header_file = open(file, 'w')
                out_interface_header_file.write(header_content)
                out_interface_header_file.close()
            else:
                print("file: [" + file + "] exist, skip")

            # interface.cc 
            file = self.src_path + '/interface/' + each['interface_name'] + '.cc'
            if not os.path.exists(file):
                cc_file_content = interface_cc_file_temlate.safe_substitute(
                    PROJECT_NAME = project_name,
                    INCLUDE_PB_HEADER = self.INCLUDE_PB_HEADER, 
                    INCLUDE_INTERFACE_HEADER = '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"',
                    CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                    CLASS_NAME = each['interface_class_name'],
                    REQUEST_TYPE = each['request_type'],
                    RESPONSE_TYPE = each['response_type'],
                    FILE_NAME = each['interface_name'] + '.cc',
                    INCLUDE_INTERFACEBASE_HEADER = self.INCLUDE_INTERFACEBASE_HEADER
                )
                out_interface_cc_file = open(file, 'w')
                out_interface_cc_file.write(cc_file_content)
                out_interface_cc_file.close()
            else:
                print("file: [" + file + "] exist, skip")
        
            # test_interface_client.cc 
            file = self.test_client_path + '/test_' + each['interface_name'] + '_client.cc'
            if not os.path.exists(file):
                cc_file_content = interface_test_client_file_template.safe_substitute(
                    INCLUDE_PB_HEADER = self.INCLUDE_PB_HEADER, 
                    CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                    REQUEST_TYPE = each['request_type'],
                    RESPONSE_TYPE = each['response_type'],
                    STUBCLASS = stub_name,
                    METHOD_NAME = each['method_name'],
                    FILE_NAME = 'test_' + each['interface_name'] + '_client.cc',
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
        
          

    def generate_framework_code(self): 
        print('=' * 100)
        print('Begin to generate tinyrpc framework code')

        self.parse_service_name()

        self.gen_exception()

        self.gen_include_name()

        self.gen_server()

        self.gen_main()

        self.gen_interface_base()

        self.gen_each_interface()
            
            

    def gen_pb_files(self):
        print('=' * 100)
        print('Begin generate protobuf file')
        target_path = self.src_pb_path + '/'
        cmd = 'cp -r ' + self.proto_file + ' ' + target_path
        cmd += ' && cd ' + target_path + ' && protoc --cpp_out=./ ' + self.proto_file 
        print('excute cmd: ' + cmd)

        if os.system(cmd) is not 0:
            raise Exception("execute cmd failed [" + cmd + "]")
        
        print('End generate protobuf file')
        print('=' * 100)

    def gen_makefile(self):
        print('=' * 100)
        print('Begin to generate makefile')
        out_file = self.src_path + '/makefile'
        if os.path.exists(out_file):
            print('makefile exist, skip generate')
            print('End generate makefile')
            print('=' * 100)
            return 
        
        template_file = open(self.generator_path + '/template/makefile.template','r')
        # print(template_file.read())
        tmpl = Template(template_file.read())

        content = tmpl.safe_substitute(
            PROJECT_NAME = self.project_name,
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

        file = open(out_file, 'w')
        file.write(content)
        file.close()
        print('succ write to ' + out_file)
        print('End generate makefile')
        print('=' * 100)

    def gen_conf_file(self):
        print('=' * 100)
        print('Begin to generate tinyrpc conf file')
        out_file = self.conf_path + '/' + self.project_name + '.xml'
        if os.path.exists(out_file):
            print('makefile exist, skip generate')
            print('End generate makefile')
            print('=' * 100)
            return 
        
        template_file = open(self.generator_path + '/template/conf.xml.template','r')
        # print(template_file.read())
        tmpl = Template(template_file.read())

        content = tmpl.safe_substitute(
            PROJECT_NAME = self.project_name,
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

        file = open(out_file, 'w')
        file.write(content)
        file.close()
        print('succ write to ' + out_file)
        print('End generate tinyrpc conf file')
        print('=' * 100)
        
        
    def gen_run_script(self):
        print('=' * 100)
        print('Begin to generate run script')
        script = open(self.generator_path + '/template/conf.xml.template','r')
        dir_src = self.generator_path + '/template/'
        cmd = 'cp -r ' + dir_src + '*.sh ' + self.bin_path + "/" 
        print('excute cmd: ' + cmd)
        os.system(cmd)

        print('End generate run script')
        print('=' * 100)
        
        
    def generate_tinyrpc_project(self):
        try:

            self.parseInput()

            print('=' * 150)
            print('Begin generate tinyrpc project')

            self.generate_dir()

            self.gen_pb_files()

            self.gen_makefile()

            self.gen_run_script()

            self.gen_conf_file()

            self.generate_framework_code()

            print('Succ generate tinyrpc project')
            print('=' * 150)

        except Exception as e:
            print('Falied generate tinyrpc project, err:')
            print(e)
            traceback.print_exc()
            print('=' * 150)
        finally:
            pass


    def printHelp(self):
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

        print('')
        print('For example:')
        print('tinyrpc_generator.py -i order_server.proto -o ./')

        print('')


        print('=' * 100)
    

    def parseInput(self):
        opts,args=getopt.getopt(sys.argv[1:],"hi:o:p:h:",["help","input=","output="])
        for opts,arg in opts:
            if opts=="-h" or opts=="--help":
                self.printHelp()
                sys.exit(0)
            if opts=="-i" or opts=="--input":
                self.proto_file = arg 
            elif opts=="-o" or opts=="--output_path":
                self.out_project_path = arg
                if self.out_project_path[-1] != '/':
                    self.out_project_path = self.out_project_path + '/'
            else:
                raise Exception("invalid options: [" + opts + ": " + arg + "]")

        if not os.path.exists(self.proto_file):
            raise Exception("Generate error, not exist protobuf file: " + self.proto_file)

        if ".proto" not in self.proto_file:
            raise Exception("Generate error, input file is't standard protobuf file:[" + self.proto_file + "]")

        self.project_name = self.proto_file[0: -6]
        print("project name is " + self.project_name)




 

if __name__ == '__main__':
    instanse = Generator()
    instanse.generate_tinyrpc_project()