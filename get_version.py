from subprocess import check_output

version = check_output(["git", "describe", "--always"]).rstrip()

print("-DMILIGHT_HUB_VERSION=%s" % version)
