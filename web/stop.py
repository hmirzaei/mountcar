#!/usr/bin/env python
import subprocess
from subprocess import Popen, PIPE

p = Popen('ssh pi@169.234.11.41 " screen -d -m bash runstop.sh"', stdout=PIPE, stderr=PIPE, shell=True)
stdout, stderr = p.communicate()
print(stdout)
print(stderr)


