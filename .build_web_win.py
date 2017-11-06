from shutil import copyfile
from subprocess import check_output
import sys
import os
import platform
import subprocess

Import("env")

def is_tool(name):
    cmd = "where" if platform.system() == "Windows" else "which"
    try:
        check_output([cmd, name] )
        return True
    except:
        return False;

def build_web():
    if is_tool("npm"):
        os.chdir("web")
        #os.mkdir("build")
        #os.mkdir("node_modules")
        print("Attempting to build webpage...")
        try:
            print check_output(["npm", "install"] , shell=True)
            #print check_output(["npm", "install gulp-dependency-install"] , shell=True)
            print check_output(["node_modules\.bin\gulp"] , shell=True)
            #print check_output(["node_modules/.bin/gulp"] , shell=True)
            copyfile("build/index.html.gz.h", "../dist/index.html.gz.h")

        except Exception as e:
            print "Encountered error building webpage: ", e
            print "WARNING: Failed to build web package. Using pre-built page."
            pass
        finally:
            os.chdir("..");

build_web()
