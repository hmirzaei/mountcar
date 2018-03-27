#!/usr/bin/env python
import subprocess
from subprocess import Popen, PIPE

subprocess.check_output('scp wp-content/uploads/rlalg.c pi@169.234.11.41:mountcar/rp/.'.split(' '))
p = Popen('ssh pi@169.234.11.41 " screen -d -m bash runstart.sh"', stdout=PIPE, stderr=PIPE, shell=True)
stdout, stderr = p.communicate()
print(stdout)
print(stderr)


