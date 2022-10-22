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

generator_path = sys.path[0]


def main():
    pass


def test():
    pass


def gen_pb_files():
    pb_path = src_path + '/pb/'
    cmd = 'cp -r ' + proto_file + ' ' + pb_path
    cmd += ' && cd ' + pb_path + ' && protoc --cpp_out=./ ' + proto_file 
    print('excute cmd: ' + cmd)
    os.system(cmd)

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
        print('conf file exist, skip generate')
        print('End generate tinyrpc conf file')
        print('=' * 100)
        return 
    
    template_file = open(generator_path + '/template/conf.xml.template','r')
    # print(template_file.read())
    tmpl = Template(template_file.read())

    content = tmpl.safe_substitute(
        PROJECT_NAME = project_name,
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

    file = open(out_file, 'w')
    file.write(content)
    file.close()
    print('succ write to ' + out_file)
    print('End generate tinyrpc conf file')
    print('=' * 100)
    
    
    

def generate_dir():
    print('=' * 100)
    print('Begin to generate dir')

    if out_project_path == "":
        proj_path = './' + project_name.strip()
    if out_project_path[-1] == '/':
        proj_path = out_project_path + project_name.strip()
    else:
        proj_path = out_project_path + './' + project_name.strip()
    
    bin_path = proj_path + '/bin'

    global conf_path
    conf_path = proj_path + '/conf'

    log_path = proj_path + '/log'
    lib_path = proj_path + '/lib'

    global src_path
    src_path = proj_path + '/' + project_name
    src_interface_path = src_path + '/interface'
    src_service_path = src_path + '/service'
    src_pb_path = src_path + '/pb'

    dir_list = []
    dir_list.append(proj_path) 
    dir_list.append(bin_path) 
    dir_list.append(conf_path) 
    dir_list.append(log_path) 
    dir_list.append(lib_path) 
    dir_list.append(src_path) 
    dir_list.append(src_interface_path) 
    dir_list.append(src_service_path) 
    dir_list.append(src_pb_path) 

    for each in dir_list:
        if not os.path.exists(each):
            os.mkdir(each)
            print("succ make dir in " + each)

    print('End generate dir')
    print('=' * 100)
    

def generate_tinyrpc_project():
    try:
        parseInput()

        generate_dir()

        gen_pb_files()

        gen_makefile()

        gen_conf_file()

    except Exception as e:
        print(e)
        traceback.print_exc()
    finally:
        pass


def parseInput():
    print('=' * 100)
    print('Begin to parse Input paramters')

    opts,args=getopt.getopt(sys.argv[1:],"hi:o:",["help","input=","output="])
  
    for opts,arg in opts:
        if opts=="-h" or opts=="--help":
            print("help")
        if opts=="-i" or opts=="--input":
            global proto_file 
            proto_file = arg 
        elif opts=="-o" or opts=="--output_path":
            global out_project_path
            out_project_path = arg
            if out_project_path[-1] != '/':
                out_project_path = out_project_path + '/'
        else:
            raise Exception("invalid options: [" + opts + ": " + arg + "]")

    if not os.path.exists(proto_file):
        raise Exception("Generate error, not exist protobuf file: " + proto_file)

    if ".proto" not in proto_file:
        raise Exception("Generate error, input file is't standard protobuf file:[" + proto_file + "]")

    global project_name
    project_name = proto_file[0: -6]
    print("project name is " + project_name)

    print('End Input paramters')
    print('=' * 100)



 

if __name__ == '__main__':
    generate_tinyrpc_project()