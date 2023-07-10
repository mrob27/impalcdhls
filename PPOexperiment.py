# .../06-plotting/RLbase.py
#
# This drives a RL (Reinforcement Learning) style of AI training,
# using a variety of models including:
#   A3C - Asynchronous Advantage Actor Critic
#   DQN - Deep Q-Networks
#   PPO - Proximal Policy Optimisation
# with a variety of model hyperparameter choices
#
# 20230404 Add reporting of Fmax
# 20230405 Add MAX_GETHWC option
# 20230406 Add VARIANTs, initially just for episode length
# 20230508 Add AGENT_TYPE option
# 20230524 Add ACT_HISTORY (not yet a parameterised option)
# 20230525 Implement part of action history changes, then complete them
#   in separate script RL-ah.py
# 20230611 Finish ACT_HISTORY conditionals, and take RLBASE_ACT_HISTORY
#   from environment to set it.
# 20230612 Add USE_LSTM option (minor change, from RL-ah-lstm.py)

VAL_SEMANTIC = True
ACT_HISTORY = False
USE_LSTM = True

from datetime import datetime
import glob
import gym
from gym.spaces import Discrete, Box, Tuple
from gym.utils import seeding
import json
import math
import numpy as np
import os
import ray
from ray.rllib import agents
import ray.rllib.agents.a3c as a3c
import ray.rllib.agents.dqn as dqn
import ray.rllib.agents.ppo as ppo
from ray import tune
from ray.tune.registry import register_env
from ray.tune import CLIReporter
from ray.tune.logger import UnifiedLogger
import re
import shutil
import subprocess
import sys
import tempfile

def d14():
  return datetime.today().strftime('%Y%m%d.%H:%M:%S')

print("Starting at " + d14())

EO_NAME = os.getenv('RLBASE_PROG')    # e.g. "adpcm" or "veccrc"
if EO_NAME is None:
  print("RLbase: env RLBASE_PROG not defined")
  sys.exit(-1)
name_c = EO_NAME + ".c"
if not os.path.isfile(name_c):
  print("RLbase: there is no file '%s' in the current directory" % (name_c))
  sys.exit(-2)

AGENT_TYPE = os.getenv('RLBASE_AGENT_TYPE')  # e.g. dqn
if AGENT_TYPE is None:
  AGENT_TYPE = 'ppo'

USE_LSTM = os.getenv('RLBASE_USE_LSTM')  # 0 or 1
if USE_LSTM is None:
  USE_LSTM = False
else:
  if (int(USE_LSTM) == 0):
    USE_LSTM = False
  else:
    USE_LSTM = True

ACT_HISTORY = os.getenv('RLBASE_ACT_HISTORY')  # 0 or 1
if ACT_HISTORY is None:
  ACT_HISTORY = False
else:
  if (int(ACT_HISTORY) == 0):
    ACT_HISTORY = False
  else:
    ACT_HISTORY = True

INIT_SEED =  os.getenv('RLBASE_SEED')  # e.g. 3, 7, 27
if INIT_SEED is None:
  print("RLbase: env RLBASE_SEED not defined")
  sys.exit(-3)
INIT_SEED = int(INIT_SEED)

N_ITER =  os.getenv('RLBASE_N_ITER')  # e.g. 10 or 300
if N_ITER is None:
  print("RLbase: env RLBASE_N_ITER not defined")
  sys.exit(-4)
N_ITER = int(N_ITER)

NUM_WORKERS = os.getenv('RLBASE_WORKERS')  # e.g. 1 or 4
if NUM_WORKERS is None:
  print("RLbase: env RLBASE_WORKERS not defined")
  sys.exit(-5)
NUM_WORKERS = int(NUM_WORKERS)
if ((NUM_WORKERS<1) or (NUM_WORKERS>4)):
  print("RLbase: env RLBASE_WORKERS out of range [1..4]")
  sys.exit(-6)

MAX_GETHWC = os.getenv('RLBASE_MAX_GETHWC')  # e.g. 20000
if MAX_GETHWC is None:
  MAX_GETHWC = N_ITER * 90
  print("RLbase: default EO_MAX_GETHWC 90*N_ITER = %d" % (MAX_GETHWC))
else:
  MAX_GETHWC = int(MAX_GETHWC)
  if (MAX_GETHWC<1):
    MAX_GETHWC = 1

SCALE_FMAX = os.getenv('RLBASE_SCALE_FMAX')  # e.g. 0 or 1
if SCALE_FMAX is None:
  SCALE_FMAX = True
  print("RLbase: default SCALE_FMAX = True")
else:
  SCALE_FMAX = int(SCALE_FMAX)
  SCALE_FMAX = True if (SCALE_FMAX > 0) else False
  print("RLbase: SCALE_FMAX = " + str(SCALE_FMAX))

VARIANT = os.getenv('RLBASE_VARIANT')  # e.g. 30steps
if VARIANT is None:
  VARIANT = 'standard'

JSON_TAG = os.getenv('RLBASE_JSON_TAG')  # e.g. 19701231.235959
if JSON_TAG is None:
  JSON_TAG = datetime.today().strftime('%Y%m%d.%H%M%S')


# "../legup/legup-4.0/examples"
# "/home/Robert/devt/asanaullah-legup/legup/legup-4.0/examples"
EO_LEVEL = os.getenv('LEGUP_PATH') + '/examples'
# 'run_ppo_'+ env_config['NAME'] +'_p'+str(os.getpid())
EO_rundir = "../tmp"

ckmemo = {}
ckmpop = 0
hitrate = 0.0
cycseen = {}
cspop = 0
hwsinc = 0
O0_cyc = -1000
O3_cyc = -500
make_accel_calls = 0
mac_sent = 0
worstcyc = 0
bestcyc = 100000000
firstever = True

eplen = 45

if VARIANT == 'standard':
  pass # no changes needed
elif VARIANT == '45steps':
  eplen = 45
elif VARIANT == '30steps':
  eplen = 30
elif VARIANT == '20steps':
  eplen = 20
elif VARIANT == '14steps':
  eplen = 14
elif VARIANT == '10steps':
  eplen = 10
elif VARIANT == '7steps':
  eplen = 7

def configEnv_variables():   # called by __init__
    global EO_rundir
    print (" " + str(hwsinc) + " configEnv_variables()")
    env_config={}
    env_config["NAME"] = EO_NAME
    env_config["LEVEL"] = EO_LEVEL
    EO_rundir = '../tmp/p'+str(os.getpid())
    env_config["self_run_dir"] = EO_rundir
    self_run_dir= env_config["self_run_dir"]   
    cwd = os.getcwd()
    self_run_dir = os.path.join(cwd, self_run_dir)
    env_config["self_run_dir"]= self_run_dir
    if os.path.isdir(self_run_dir):
        shutil.rmtree(self_run_dir, ignore_errors=True)
    os.makedirs(self_run_dir)
    # Copy the Makefile and program into the temp. dir for this worker
    shutil.copy("Makefile", self_run_dir + '/' + "Makefile")
    shutil.copy(env_config['NAME'] + ".c", self_run_dir)

    env_config["opt_passes"] = "-correlated-propagation -scalarrepl -lowerinvoke -strip -strip-nondebug -sccp -globalopt -gvn -jump-threading -globaldce -loop-unswitch -scalarrepl-ssa -loop-reduce -break-crit-edges -loop-deletion -reassociate -lcssa -codegenprepare -memcpyopt -functionattrs -loop-idiom -lowerswitch -constmerge -loop-rotate -partial-inliner -inline -early-cse -indvars -adce -loop-simplify -instcombine -simplifycfg -dse -loop-unroll -lower-expect -tailcallelim -licm -sink -mem2reg -prune-eh -functionattrs -ipsccp -deadargelim -sroa -loweratomic".split()
    env_config["actions"] = []

    env_config["max_episode_steps"] = eplen
    env_config["prev_cycles"] = 10000000 
    env_config["O0_cycles"] = 10000000 
    env_config["min_cycles"] = 10000000 
    if ACT_HISTORY:
      env_config["state"] = [0] * eplen
    else:
      env_config["state"] = [0] * (len(env_config["opt_passes"]))
    env_config["reward"] = get_reward(is_done=False, init=True,
                             env_config=env_config)
    if ACT_HISTORY:
      env_config["n_acts"] = 0
    return env_config


def get_actionSpace(env_config):  # called by __init__
    action_space = Discrete(len(env_config["opt_passes"]))
    return action_space

def get_observationSpace(env_config):  # called by __init__
    if ACT_HISTORY:
      # Each element of action history is a number in [0..(# of opt passes)]
      observation_space = Box(0.0,len(env_config["opt_passes"]),
                     # It's a 1-D array of size (max # of actions)
                     shape=(env_config["max_episode_steps"],),dtype = np.int32)
    else:
      # Each element of a histogram is a number in [0..(max # of actions)]
      observation_space = Box(0.0,env_config["max_episode_steps"],
                     # It's a 1-D array with 1 entry for each opt pass
                     shape=(len(env_config["opt_passes"]),),dtype = np.int32)
    return observation_space

def get_state(env_config, action):  # called by step()
    env_config["actions"].append(action)
    if ACT_HISTORY:
      # Append to end of history
      env_config["state"][env_config["n_acts"]] = action
      env_config["n_acts"] += 1
    else:
      env_config["state"][action] += 1
    return (np.array(env_config["state"]))


def check_done(env_config):         # called by step()
    # print("cd: " + str(env_config["state"]))
    # print("cd: " + str(len(env_config["actions"]))
    #        + " " + str(env_config["max_episode_steps"]))
    if len(env_config["actions"]) < env_config["max_episode_steps"]:
      is_done = False
    else:
      is_done = True 
    return is_done


def domake(cmd, do_print=False):
  """
  run a make command, and get its output.
  """
  proc = subprocess.Popen(["timeout 10 make "+ cmd],
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          shell=True, cwd=EO_rundir)
  (out, err) = proc.communicate()
  if do_print is True:
    print(out.decode("utf-8"));
    print(err.decode("utf-8"));
  return out.decode("utf-8")


def save_results(makeparams, prefix):
  """
  This invokes the Makefile rule to save the ll and TeX files with
  a name that starts with a particular prefix, like 'fastest' for
  the fastest one we've found so far.
  """
  print (" saving .ll and .tex files with prefix " + prefix)
  global EO_rundir
  makesaveprefix = " SAVE_PREFIX=" + prefix
  out = domake("saveSSA " + makeparams + makesaveprefix, do_print=False)
  #proc = subprocess.Popen([shellcmd],
  #                     stdout=subprocess.PIPE, stderr=subprocess.PIPE,
  #                     shell=True, cwd=EO_rundir)
  #(out, err) = proc.communicate()


def save_cc_instance(makeparams, cyc, instance):
  """
  This saves the ll file with name starting with a cycle count and sequence
  number. This is intended for archiving a sampling of examples that 'should'
  be equivalent but for some reason are not.
  """
  global EO_rundir
  ccf = "{:05d}".format(cyc)
  inf = "{:02d}".format(instance)
  print (" saving .ll for cycles " + ccf + " instance " + inf)
  makesaveprefix = " SAVE_PREFIX=" + ccf + "-" + inf
  out = domake("saveCycInst " + makeparams + makesaveprefix, do_print=False)
  #shellcmd = "make saveCycInst " + makeparams + makesaveprefix
  #proc = subprocess.Popen([shellcmd],
  #                     stdout=subprocess.PIPE, stderr=subprocess.PIPE,
  #                     shell=True, cwd=EO_rundir)
  #(out, err) = proc.communicate()
  #print(out.decode("utf-8"));
  #print(err.decode("utf-8"));


def hwcyc(opt_passes, want_O0=None, extrasave=None):
  global ckmemo
  global ckmpop
  global hitrate
  global cspop
  global cycseen
  global hwsinc
  global worstcyc
  global bestcyc
  global firstever
  global make_accel_calls
  done = False
  hw_cycle = 10000001
  if want_O0 is None:
    opt_Olevel = ""
  elif want_O0 == 0:
    opt_Olevel = ""
  else:
    opt_Olevel = "-O" + str(want_O0)

  # Our first #make# rule will compile the program with the opt flags
  # given to us by the caller (it does the 'touch' target first to
  # make sure the rule doesn't short-circuit the re-generation of the
  # bitcode)
  makeparams = (" NAME=" + EO_NAME
             + " LEVEL=" + EO_LEVEL
             + " EXTRA_OPT_FLAGS='" + opt_passes + "'"
             + " OPT_OLEVEL='" + opt_Olevel + "'")
  out = domake("touchSrc optbcChecksum "+ makeparams, do_print=firstever)
  p = re.compile(r"^.*\scksum\s+\|\s+(\d+)\s", re.DOTALL)
  m = re.match(p, out)
  if m:
    cksum = int(m.group(1))
  else:
    hw_cycle = 10000002
    cksum = -1
    done = True

  # If we know the answer for this checksum, we can skip the next domake()s
  gothit = False
  if str(cksum) in ckmemo:
    hw_cycle = int(ckmemo[str(cksum)])
    hitrate = hitrate * 0.999 + 0.001
    gothit = True
  else:
    hitrate = hitrate * 0.999

  # Find out if the program is still semantically valid
  if VAL_SEMANTIC:
    if gothit is False and done is False:
      out = domake("optVerifyPASS "+ makeparams, do_print=firstever)
      p = re.compile(r"^.*PASS.*", re.DOTALL)
      m = re.match(p, out)
      if m:
        pass # No error; use of 'pass' keyword here is merely a coincidence
      else:
        print("*** Error, program does not print PASS")
        hw_cycle = 10000003
        done = True

  # If still no error, we can go ahead and do all the steps to generate
  # a hardware description and measure clock cycles.
  if gothit is False and done is False:
    print ("make accelerationCycle")
    out = domake("accelerationCycle "+ makeparams, do_print=firstever)
    make_accel_calls = make_accel_calls + 1
    p = re.compile(r"^.*main \|\s+(\d+).*", re.DOTALL)
    m = re.match(p, out)
    if m:
      hw_cycle = m.group(1)
      if int(hw_cycle) == 0:
        hw_cycle = 10000004
        done = True
    else:
      print("*** Error, accelCyc target did not yield a number")
      hw_cycle = 10000005
      done = True

    if done is False:
      m2 = re.findall(r"Fmax: ([.0-9]+) MHz", out, re.M)
      if m2 is None:
        print("*** Error, accelCyc got no Fmax")
        hw_cycle = 10000008
        done = True
      else:
        hw_fmax = m2[0]
        if hw_fmax is None:
          print("*** Error, accelCyc got no Fmax")
          hw_cycle = 10000009
          done = True
        else:
          print ("accelCyc %d cyc at Fmax %s" % (int(hw_cycle), hw_fmax))

  print ("hwc %s " % (str(hw_cycle)))
  if (gothit is False) and (int(hw_cycle) < 10000000) and SCALE_FMAX:
    hw_cycle = int(int(hw_cycle) * 50 / int(hw_fmax))

  # log this result for gleaning pairwise ordering etc. stats
  print("macc: " + opt_passes + " " + opt_Olevel + " : " + str(hw_cycle))

  if str(hw_cycle) in cycseen:
    cycseen[str(hw_cycle)] = cycseen[str(hw_cycle)] + 1
    if cycseen[str(hw_cycle)] <= 27:
      save_cc_instance(makeparams, int(hw_cycle), cycseen[str(hw_cycle)])
  else:
    cspop = cspop + 1
    cycseen[str(hw_cycle)] = 1

  hwsinc = hwsinc + 1
  summary = "cktocycle {:10d} = {:4s} pop {:d}".format(cksum, str(hw_cycle), cycseen[str(hw_cycle)])
  if cksum < 0:
    print("got error cksum " + str(cksum))
  elif gothit:
    print(" " + str(hwsinc) + "rpt " + summary)
    # print("  repeat by " + str(len(env_config["actions"])) + ":(" + opt_passes + ")")
  else:
    print(" " + str(hwsinc) + "    new " + summary)
    ckmemo[str(cksum)] = str(hw_cycle)
    ckmpop = ckmpop + 1
    if ckmpop % 200 == 0:
      print(" " + d14()
          + " " + str(hwsinc) + "  " + str(cspop) + " cyc and " + str(ckmpop)
          + " cs, hitrate " + "{:7.5f}".format(hitrate))

  hw_cycle = int(hw_cycle)

  if extrasave is None:
    if hwsinc>2:
      # Initial stuff is done, we can start checking for and saving
      # .v, .ll, .tex for best and worst.
      if (hw_cycle > worstcyc) and (hw_cycle < 10000000):
        worstcyc = hw_cycle
        print (" new worst cycles: " + str(worstcyc))
        save_results(makeparams, "worst")
      if (hw_cycle < bestcyc) and (hw_cycle > 0) and (hw_cycle < 10000000) :
        bestcyc = hw_cycle
        print (" new best cycles: " + str(bestcyc))
        save_results(makeparams, "best")
  else:
    save_results(makeparams, extrasave)

  firstever = False

  return hw_cycle, done
  # end of def hwcyc


def sendJson():
    global O0_cyc
    global O3_cyc
    global make_accel_calls
    global mac_sent
    send_dict = {
      "cyc_O0" : O0_cyc,
      "cyc_O3" : O3_cyc,
      "make_accel_calls" : make_accel_calls
    }
    as_json = json.dumps(send_dict, indent = 2)
    # Note that domake() passes cwd=EO_rundir to subprocess.Popen()
    # causing all the tmp files to go there, but this JSON file will
    # be in the original directory (in which this Python program is run)
    json_name = "from_worker_" + JSON_TAG + ".json"
    with open(json_name, "w") as outfile:
      outfile.write(as_json)
    mac_sent = make_accel_calls
    # print(" mainloop: wrote mac==%d to JSON" % (make_accel_calls))
    # End of def sendJson


def getHWCycles(env_config):
    global O0_cyc
    global O3_cyc
    global make_accel_calls
    # If we are running for the first time, we do two extra measurements
    # to discover the performance from opt with no options (-O0) and with
    # the -O3 option. We have to write these out to a JSON file so the
    # main process can find out what these numbers are.
    if O0_cyc == -1000:
      cyc, done = hwcyc("", 0, "O0")
      O0_cyc = cyc
      print(" baseline: set O0_cyc to " + str(O0_cyc))
      cyc, done = hwcyc("", 3, "O3")
      O3_cyc = cyc
      print(" baseline: set O3_cyc to " + str(O3_cyc))

    ga_seq = map((lambda x: env_config["opt_passes"][x]), env_config["actions"])
    ga_seq_str = " ".join(ga_seq)
    cyc, done = hwcyc(ga_seq_str, 0, None)
    if make_accel_calls > mac_sent:
      sendJson()
    print("%s gHC ret %d" % (d14(), int(cyc)))
    return int(cyc), done
    # end of get.HWCycles

def get_reward(is_done, init, env_config):  # called by step()
    global O0_cyc
    return_rew= {}
    reward = 0 
    if (init or is_done):          
        # print("O0_cyc is " + str(O0_cyc))
        # print (str(hwsinc) + "   get.reward(" + str(is_done)
        #                    + "," + str(init) + ",...)")
        cycle, done = getHWCycles(env_config)
        if (init):
            env_config["O0_cycles"] = cycle
            print("grew: O0_cycles = %d" % cycle)
        if cycle >= 10000000:
            cycle = 2 * env_config["O0_cycles"]
        if (cycle < env_config["min_cycles"]):
            env_config["min_cycles"] = cycle
            print("grew: new min_cycles = %d" % cycle)
        if not init:
            reward = env_config["O0_cycles"] - cycle
        env_config["prev_cycles"] = cycle # not currently used anywhere
        print("grew-returning %d" % reward)
    env_config["reward"] = reward
    return reward

class myEnv(gym.Env):

    def __init__(self):  # called by rlib
        print("myEnv::__init__()")
        self.env_config= configEnv_variables()
        self.action_space = get_actionSpace(self.env_config)
        self.observation_space = get_observationSpace(self.env_config)
        # self.seed(seed=23) # does nothing
        self.viewer = None
        self.state = self.env_config["state"]
        print("%s init: max_ep_steps %d" % (d14(),
                                       self.env_config["max_episode_steps"]))
        print("%s init: action_spc %s" % (d14(), str(self.action_space)))

    def seed(self, seed=None):  # called by rlib
        print("myEnv::seed(%d)" % (0 if seed is None else seed))
        self.np_random, seed = seeding.np_random(seed)
        print("%s self.np_random seeded with %s" % (d14(), str(seed)))
        return [seed]

    def step(self, action):  # called by rlib
        print("myEnv::step(%s)" % (str(action) if type(action) is not list else " ".join(str(x) for x in action)))
        state = get_state(self.env_config, action)
        is_done = check_done(self.env_config)  
        # if is_done:
        #   print (str(hwsinc) + " step(self,action)")
        reward = get_reward(is_done, False, self.env_config)
        return state, reward, is_done, {}
    
    def reset(self):  # called by rlib
        print("myEnv::reset()")
        self.env_config["actions"] = []
        if ACT_HISTORY:
          self.env_config["n_acts"] = 0
          self.env_config["state"] = [0]*(self.env_config["max_episode_steps"])
        else:
          self.env_config["state"] = [0]*len(self.env_config["opt_passes"])
        # self.env_config["prev_cycles"] = self.env_config["O0_cycles"] 
        return np.array(self.state)

    def close(self):  # called by rlib
        print("myEnv::close()")
        if os.path.isdir(self.env_config["self_run_dir"]):
            shutil.rmtree(self.env_config["self_run_dir"])
        if self.viewer:
            self.viewer.close()
            self.viewer = None



def custom_log_creator(custom_path, custom_str):
    timestr = datetime.today().strftime("%Y-%m-%d_%H-%M-%S")
    # logdir_prefix = "{}_{}".format(custom_str, timestr)
    logdir_prefix = custom_str + str(os.getpid()) + '_';
    def logger_creator(config):
        if not os.path.exists(custom_path):
            os.makedirs(custom_path)
        logdir = tempfile.mkdtemp(prefix=logdir_prefix, dir=custom_path)
        return UnifiedLogger(config, logdir, loggers=None)
    return logger_creator

class ExperimentTerminationReporter(CLIReporter):
    def should_report(self, trials=False, done=False):
        return done

def get_O_measurements():
  """
  Get information that the worker (actor) has hopefully written to a local
  file (We have to go through a file because the agents cannot access our
  globals or visa-versa)
  """
  global O0_cyc
  global O3_cyc
  global make_accel_calls
  as_json = {}
  json_name = "from_worker_" + JSON_TAG + ".json"
  with open(json_name, 'r') as infile:
    as_json = json.load(infile)

  # We test for existence of each field because a race condition between
  # multiple workers can cause the file to be empty or corrupted/invaid
  if "cyc_O0" in as_json:
    print("got O0_cyc==" + str(as_json["cyc_O0"]) + " from worker via JSON")
    O0_cyc = as_json["cyc_O0"]
  else:
    print("did not get a 'cyc_O0'")

  if "cyc_O3" in as_json:
    print("got O3_cyc==" + str(as_json["cyc_O3"]) + " from worker via JSON")
    O3_cyc = as_json["cyc_O3"]
  else:
    print("did not get a 'cyc_O3'")

  if "make_accel_calls" in as_json:
    print("got make_accel_calls=='%s' from worker via JSON"
                                      % (str(as_json["make_accel_calls"])) )
    make_accel_calls = as_json["make_accel_calls"]
    make_accel_calls = int(make_accel_calls)
  else:
    print("did not get a 'make_accel_calls'")

  # If the worker measured the same number of cycles for -O0 and -O3, then
  # we need to fudge our values to avoid a division by zero error in the
  # rescale() function
  if O3_cyc == O0_cyc:
    O3_cyc = int(O0_cyc/2)
    if O3_cyc == O0_cyc:
      O0_cyc = O0_cyc + 1

#   raw values     rewards    scaled
# worst    74085   -32000     -3.318
# -O0      42085      0        0.000
# -O3      33244     8841      1.000
# best      9644    32441      3.364
# dream on     0    42085      4.364
def rscale(O0, O3, val):
  return (val/(O0-O3))

ret = ray.init(ignore_reinit_error=True)
CHECKPOINT_ROOT = "." 
if AGENT_TYPE == 'dqn':
  config = dqn.DEFAULT_CONFIG.copy()
elif AGENT_TYPE == 'a3c':
  config = a3c.DEFAULT_CONFIG.copy()
else:
  config = ppo.DEFAULT_CONFIG.copy()

config["log_level"] = "ERROR"
config["num_workers"] = NUM_WORKERS
config["seed"] = INIT_SEED # 3->1004, 7->1008, 27->1028
if USE_LSTM:
  config["model"]["use_lstm"] = True

register_env("NewEnv", lambda config: myEnv())

if AGENT_TYPE == 'dqn':
    agent = dqn.DQNTrainer(config, env="NewEnv",
          logger_creator=
               custom_log_creator(os.path.expanduser("."), '../tmp/e'))
elif AGENT_TYPE == 'a3c':
    config["horizon"] = eplen
    agent = a3c.A3CTrainer(config, env="NewEnv",
          logger_creator=
               custom_log_creator(os.path.expanduser("."), '../tmp/e'))
else:
  agent = ppo.PPOTrainer(config, env="NewEnv",
          logger_creator=
               custom_log_creator(os.path.expanduser("."), '../tmp/e'))

s1 = "{:3d} reward {:6.1f}/{:6.1f}/{:6.1f} len {:6.2f}"
s2 = "{:3d} scaled {:5.3f}/{:5.3f}/{:5.3f} len {:4.1f}"

print("%s init: cwd %s" % (d14(), os.getcwd()))
print("%s init: program %s" % (d14(), EO_NAME))
print("%s init: agent %s" % (d14(), AGENT_TYPE))
print("%s init: use_lstm %s" % (d14(), str(USE_LSTM)))
print("%s init: seed %d" % (d14(), config["seed"]))
print("%s init: val_sem %s" % (d14(), str(VAL_SEMANTIC)))
print("%s init: SCALE_FMAX %s" % (d14(), SCALE_FMAX))
print("%s init: N_ITER %d" % (d14(), N_ITER))
print("%s init: act_hist %s" % (d14(), str(ACT_HISTORY)))
print("%s init: batchsz %d" % (d14(), config['train_batch_size']))
print("%s init: workers %d" % (d14(), config["num_workers"]))
print("%s init: MAX_GETHWC %d" % (d14(), MAX_GETHWC))
t = config["horizon"]
print("%s init: horizon %s" % (d14(), 'None' if t is None else str(t)))

n = 0
still_going = True
while (n<N_ITER) and (still_going):
  result = agent.train()
  n = n + 1

  # Agents have had a chance to measure -O0 cycles, etc.
  get_O_measurements()

  # original style output line
  print(d14() + " "
    + s1.format(
      n,
      result["episode_reward_min"],
      result["episode_reward_mean"],
      result["episode_reward_max"],
      result["episode_len_mean"]
   ))

  # Properly scaled results (0.0 = -O0, 1.0 = =O1)
  print(d14() + " "
    + s2.format(
      n,
      rscale(O0_cyc, O3_cyc, result["episode_reward_min"]),
      rscale(O0_cyc, O3_cyc, result["episode_reward_mean"]),
      rscale(O0_cyc, O3_cyc, result["episode_reward_max"]),
      result["episode_len_mean"]
   ))

  total_ghc = config["num_workers"] * make_accel_calls
  print ("%s maxhwc: est %d calls to 'make accel' so far" % (d14(), total_ghc))
  if total_ghc > MAX_GETHWC:
    still_going = False
    print("%s mainloop: ending now because total_ghc > MAX_GETHWC" % (d14()))

print(datetime.today().strftime('%Y%m%d.%H:%M:%S')
    + " mainloop: Finished " + str(n) + " uninterrupted iters of agent.train\n")
