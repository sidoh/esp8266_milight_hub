from subprocess import check_output
import os
import os
import platform
import subprocess

dir_path = os.path.dirname(os.path.realpath(__file__))
os.chdir(dir_path)

# http://stackoverflow.com/questions/11210104/check-if-a-program-exists-from-a-python-script
def is_tool(name):
    cmd = "where" if platform.system() == "Windows" else "which"
    try:
        check_output([cmd, "git"])
        return True
    except:
        return False;

version = "UNKNOWN"

if is_tool("git"):
    try:
        version = check_output(["git", "describe", "--always"]).rstrip()
    except:
        try:
            version = check_output(["git", "rev-parse", "--short", "HEAD"]).rstrip()
        except:
            pass
        pass


print("-DMILIGHT_HUB_VERSION=%s" % version)
