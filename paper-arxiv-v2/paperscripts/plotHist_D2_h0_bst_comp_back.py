#! /usr/bin/python
from matplotlib import pyplot as plt
from operator import add, sub
import sys, os, math
import numpy as np

datafiles = []
plotnames = []

############################## Settings ###############################

# Output filename
outfile = "D2_h0_bst_comp_back"

linestyles = [ 'solid', 'dashed','solid', 'dashed']

# Datafiles
datafiles.append("../plotdata/results_noPU/diHiggs/histo_D2_fj1_boost_C1e.dat")
datafiles.append("../plotdata/results_SK_PU80/diHiggs/histo_D2_fj1_boost_C1e.dat")
datafiles.append("../plotdata/results_noPU/background/histo_D2_fj1_boost_C1e.dat")
datafiles.append("../plotdata/results_SK_PU80/background/histo_D2_fj1_boost_C1e.dat")

# Plot labels
plotnames.append("Signal, no PU")
plotnames.append("Signal, $n_{PU}=80$ + SK")
plotnames.append("Background, no PU")
plotnames.append("Background, $n_{PU}=80$ + SK")

# Axis labels
xLabel = r"Energy Correlation Ratio $D_2^{(\beta)}$ in leading Higgs candidate"
yLabel = "a. u."

# Log axes
xLog = False
yLog = False

# Normalise histograms
normalise = True

#######################################################################

if len(datafiles)!=len(plotnames):
  print "Error: datafile and plotname arrays are different lengths!"
  exit()

colours = ['r', 'r', 'b', 'b']
icol = 0

# Setup figure
fig, ax = plt.subplots()
if xLog == True:
  ax.set_xscale('log')
if yLog == True:
  ax.set_yscale('log')

ax.set_ylabel(yLabel)
ax.set_xlabel(xLabel)

ax.set_xlim([0,1])
ax.set_ylim([0,4.0])

for idat in xrange(0,len(datafiles)):

  # Verify paths
  infilenm = datafiles[idat]
  if os.path.exists(infilenm) == False:
    print "Error: source file" + infilenm + " not found!"
    sys.exit()
  
  infile = open(infilenm, 'rb')
  datafile = open(infilenm, 'rb')

  xhi = []
  xlo = []
  yval = []

  errup = []
  errdn = []

  dataread = False
  for line in datafile:
    linesplit = line.split()

    if len(linesplit) == 1:
      continue

    if linesplit[1] == 'END':
      break

    if dataread == True:
      xlo.append(float(linesplit[0]))
      xhi.append(float(linesplit[1]))
      yval.append(float(linesplit[2]))
      errdn.append(float(linesplit[3]))
      errup.append(float(linesplit[4]))

    if linesplit[1] == 'xlow':
      dataread = True

  # Normalisations
  norm = 1.0
  if normalise == True:
    norm = 0
    for i in xrange(0,len(xhi)):
      h = xhi[i] - xlo[i]
      norm = norm + h*yval[i]
  norm = np.sum(norm) # Numpy types

  
  # Error bars
  CVup = map(add, yval/norm, errup/norm)
  CVdn = map(sub, yval/norm, errdn/norm)

  for x in xrange(0,len(xhi)):
    xvals = [xlo[x], xhi[x]]
    yvalsup = [CVup[x], CVup[x]]
    yvalsdn = [CVdn[x], CVdn[x]]

  # Insert lower x-values
  xhi.insert(0,xlo[0])
  yval.insert(0,yval[0])

  ax.plot(xhi,yval/norm,drawstyle = "steps-pre", color = colours[icol], label=plotnames[idat],linestyle=linestyles[idat],linewidth=2.4)
  icol=icol+1

# Gridlines
ax.xaxis.grid(True)
ax.yaxis.grid(True)

plt.rcParams.update({'font.size': 14})
fig.text(0.33,0.93,"Boosted category", fontsize=20)

# Legend
legend = ax.legend(loc='best')
legend.get_frame().set_alpha(0.8)

fig.savefig(outfile+'.pdf')
