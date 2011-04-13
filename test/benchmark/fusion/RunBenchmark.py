import sys
import os
from subprocess import Popen
import ConfigParser
import logging
from benchmark_helpers import ParseNodeFile
from config_templates import server_template, client_template
import shutil

# Get configuration parameters
if (len(sys.argv) < 3):
  print "ERROR: No configuration/log file given"
  print "\t./RunBenchmark.py ConfigFile LogFile"
  exit(-1)

config_file = sys.argv[1]
log_file = sys.argv[2]

# Set up the log file
logging.basicConfig(filename=log_file,level=logging.DEBUG)

# Read Configuation File
config = ConfigParser.RawConfigParser()
config.read(config_file)

cores = config.get ( "Benchmark", "NumCores")
server_exe = config.get ( "Benchmark", "ServerExe")
client_exe = config.get ( "Benchmark", "ClientExe")
test_files = config.get ( "Benchmark", "TestDirectory")
compression = config.get ("Benchmark", "Compression")
tmp_dir = config.get ("Benchmark", "TempDirectory")
result_file = config.get ("Benchmark", "ResultFile")
config_dir = config.get ("Benchmark", "ConfigurationDirectory")
logging.info ("Configuration Settings")
logging.info ("\tConfiguration File: " + str(config_file))
logging.info ("\tNumber of Cores: " + str(cores))
logging.info ("\tServer Exe: " + str(server_exe))
logging.info ("\tClient Exe: " + str(client_exe))
logging.info ("\tTest File Directory: " + str(test_files))
logging.info ("\tOutput Result File: " + str(result_file))
logging.info ("\tCompression: " + str(compression))
logging.info ("\tTemp Directory: " + str(tmp_dir))
logging.info ("\tConfiguration Dir: " + str(config_dir))
# Get PBS_NODES availible for use
pbs_nodefile = os.environ["PBS_NODEFILE"]
f = open(pbs_nodefile, "r")
nodelist = f.readlines()
f.close()
nodes = ParseNodeFile (nodelist)
if len(nodes.keys()) < 2:
  logging.error("Not enough nodes to run test, Minimum of two needed")
  logging.error("\tNumber of nodes: " + str(len(nodes.keys())))
  exit(-1)

logging.info ("PBS Node/Proc count")
for x in nodes:
  logging.info ("\tNode " + x + ": " + str(nodes[x]))

# Copy testfiles to test directory 
shutil.copytree (test_files, os.path.join(tmp_dir,"test_files"))

# Generate Server Configuration
hostname = os.uname()[1]
ServerNode = hostname
ServerProcCount = nodes[ServerNode]
del nodes[ServerNode]

server_conf = server_template % str("tcp://" + ServerNode + ":9001")
f = open ( os.path.join(config_dir,"serverconf.conf"), "w")
f.write(server_conf)
f.close()

client_conf = client_template
f = open ( os.path.join(config_dir,"clientconf.conf"), "w")
f.write(client_conf)
f.close()

logging.info ("Server Node Info")
logging.info ("\tNode " + str(ServerNode) + ": " + str(ServerProcCount))
logging.info ("Client Node/Proc count")
for x in nodes:
  logging.info ("\tNode " + x + ": " + str(nodes[x]))

f = open ( os.path.join(tmp_dir, "nodelist"), "w")
for x in nodes:
  for y in range (0, nodes[x]):
    f.write(x + "\n")

f.close()

for root, dirs, files in os.walk(os.path.join(tmp_dir,"test_files")):
  for name in files:
    file_name =  os.path.join ( root, name )
    logging.info ("File Test: " + file_name)

    # Execute Server 
    os.putenv("ZOIDFS_SERVER_RANK","0")
    server = Popen([server_exe,"--config",os.path.join(config_dir,"serverconf.conf")])
    logging.info("\tServer Started")
    logging.info("\t\t" + server_exe + " --config " + os.path.join(config_dir,"serverconf.conf"))
  

    #Execute Client 
    nodefile = os.path.join(tmp_dir, "nodelist")
    #"-f", nodefile,
    client = Popen(["mpiexec", "-n", str(cores), 
                    client_exe, "tcp://" + ServerNode + ":9001", 
                    os.path.join(config_dir,"clientconf.conf"), file_name, 
                    "/dev/null",  str(os.path.getsize(file_name)), result_file])
    logging.info ("\tClient Started")
    logging.info ("\t\t" + "mpiexec" + " -n " + str(cores) + " -f \n\t\t\t" + nodefile +
                  " " + client_exe + " tcp://" + ServerNode + ":9001\n\t\t\t" +
                  os.path.join(config_dir,"clientconf.conf") + " " + file_name +  
                  "\n\t\t\t/dev/null " +  str(os.path.getsize(file_name)) + " " + result_file)

    client.wait()
    server.kill()


