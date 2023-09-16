impalcdhls
==========

Improved Models for Policy-Agent Learning of Compiler Directives in HLS

These files accompany the 2023 paper by Robert Munafo


Dependencies
------------

Follow the instructions in "Instructions.md"

Running Experiments
-------------------

PPOexperiment.py is the Python program used to conduct experiments. An
example usage:

```
mkdir rl-exper
cd rl-exper
mkdir tmp
mkdir logs
mkdir python
cd python
cp ../../PPOexperiment.py .

RLBASE_PROG=adpcm RLBASE_AGENT_TYPE=ppo RLBASE_USE_LSTM=0 \
  RLBASE_VARIANT=14steps RLBASE_ACT_HISTORY=0 RLBASE_SEED=27 \
  RLBASE_N_ITER=300  RLBASE_WORKERS=1 RLBASE_MAX_GETHWC=20000 \
  RLBASE_SCALE_FMAX=1 RLBASE_JSON_TAG=20230701.120000 \
  python3.9 PPOexperiment.py
```

Notice that many options are controlled by parameters passed through
environment variables. The options are:

RLBASE_PROG - the basename (without extension) of the test
(benchmark) program to compile. ".c" will be added to this.

RLBASE_AGENT_TYPE - This may be "ppo", "dqn", or "a3c". It uses the
corresponding reinforcement learning trainer (for example
DQNTrainer from ray.rllib.agents.dqn)

RLBASE_USE_LSTM - Pass 0 normally, pass 1 to set the "use_lstm"
model parameter to True.

RLBASE_VARIANT - Chooses the number of optimizations to take for each
trial compilation. This is also the number of steps in an episode.
Valid values for RLBASE_VARIANT are "7steps", "10steps", "14steps",
"20steps", "30steps", and "45steps"/

RLBASE_ACT_HISTORY - Pass 0 to use a history of actions as the
environment vector, pass 0 to use a histogram

RLBASE_SEED - Pass any positive integer, this is used to seed the
random number generation.

RLBASE_N_ITER - The number of 4000-step batches to train before ending
the experiment (unless RLBASE_MAX_GETHWC is exceeded earlier).

RLBASE_MAX_GETHWC - The number of test compilations to perform before
ending the experiment (unless RLBASE_N_ITER batches finish earlier).
Note that memoization (remembering results of previous cycle count
results) will affect how many compilations are needed to complete a
batch: if the agent repeatedly recommends sequences of actions that
produce equivalent programs, the Makefile rules

RLBASE_WORKERS - The number of worker threads to use during training.
This should be an integer from 1 to 4.

RLBASE_SCALE_FMAX - Pass 1 to convert cycle counts to units of 20ns
(by taking the raw cycle count and multiplying by 50/Fmax). This
trains the agent to minimize execution time without regard for clock
frequency. To minimize cycle count alone (ignoring frequency) pass
0 for RLBASE_SCALE_FMAX.

RLBASE_JSON_TAG - This is a string used to generate a filename for a
JSON file that communicates information from the worker(s) to the main
Python thread. Use different values of this if you are running
multiple instances of PPOexperiment.py simultaneously.

Extracting Data for Plotting and Analysis
-----------------------------------------

Most of the detailed information generated in an experiment (such as
cycle counts from each trial compilation) is visible only to the
worker threads. Therefore a lot of useful information is printed by
the workers as they are running. It is intended that the
PPOexperiment.py program output be captured to a log file, e.g. by
running within a shell script, and then process the log file with a
data extraction script written in perl or sed, etc. You can change or
add print statements to get information in a format that is suitable
for your preferences.

Second-Order Training
---------------------
PairwiseExp.py is similar to PPOexperiment.py but provides small rewards
for each step in an episode, using a database of "pairwise hints" compiled
from runs of PPOexperiment.py. See the header comment in PairwiseExp.py
for instructions on how to generate the files it needs.
