import os
import signal
from subprocess import Popen, PIPE
import time

def read_current_output(process):
    while True:
        output = process.stdout.readline()
        if output == b'' and process.poll() is not None:
            break
        if output:
            print(output.decode().strip())

def create_process(cmd):
    process = Popen([cmd],
                    stdin=PIPE, stdout=PIPE, stderr=PIPE,
                    shell=True, preexec_fn=os.setsid)
    read_current_output(process)
    return process

def run_inference(audio_path, process):
    process.stdin.write("{}\n".format(audio_path).encode())
    process.stdin.flush()
    read_current_output(process)

inference_process = create_process("./gyro")
