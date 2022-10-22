# /usr/bin/python3.7

import string
import sys
import os
from string import Template
from datetime import datetime

project_name = 'test_tinypb_server'
proto_file_name = 'test_tinypb_server.proto'

def main():
    pass


def test():
    pass


def gen_pb_files(filename, path):
    cmd = 'cp -r ' + filename + ' ' + path
    cmd += ' && cd ' + path + ' && protoc --cpp_out=./ ' + filename
    print('excute cmd: ' + cmd)
    os.system(cmd)

def gen_makefile(path):
    template_file = open(r'template/makefile.template','r')
    tmpl = Template(template_file.read())

    tmpl.substitute(
        PROJECT_NAME = project_name,
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
    

def generate_dir():
    if len(project_name) == 0:
        return
    proj_path = project_name.strip()
    bin_path = proj_path + '/bin'
    conf_path = proj_path + '/conf'
    log_path = proj_path + '/log'
    lib_path = proj_path + '/lib'

    src_path = proj_path + '/' + project_name
    src_interface_path = src_path + '/interface'
    src_service_path = src_path + '/service'
    src_pb_path = src_path + '/pb'

    if not os.path.exists(proj_path):
        os.mkdir(proj_path)
        os.mkdir(bin_path)
        os.mkdir(conf_path)
        os.mkdir(log_path)
        os.mkdir(lib_path)
        os.mkdir(src_path)
        os.mkdir(src_interface_path)
        os.mkdir(src_service_path)
        os.mkdir(src_pb_path)

        gen_pb_files(proto_file_name, src_pb_path)
    else:
        pass



if __name__ == '__main__':
    generate_dir()