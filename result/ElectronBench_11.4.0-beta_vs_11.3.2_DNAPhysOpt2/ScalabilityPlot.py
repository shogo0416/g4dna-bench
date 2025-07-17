#!/bin/env python
"""
================================================================================
  BSD 2-Clause License

  Copyright (c) 2021-2025 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduction the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
================================================================================
"""
import argparse
import os
import glob
import re
import json
import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.ticker as ptick
import numpy as np
from matplotlib.backends.backend_pdf import PdfPages

DATASET = {
    '11.3.2': 'Geant4 11.3.2 (DNAPhysOpt2 + IRT)',
    '11.4.0-beta': 'Geant4 11.4.0-beta (DNAPhysOpt2 + IRT)',
    '11.3-ref6': 'Geant4 11.3-ref6 (DNAPhysOpt2 + IRT)'
}

CPUINFO = 'AMD Ryzen 9 9950X (16c/32t)'
OUTPUT_FILENAME= 'scalability_plot.pdf'

FIGSIZE_X = 6
FIGSIZE_Y = 4
DPI = 200
FONTSIZE = 14
FONTSIZE_LEGEND = 8
LINEWIDTH = 3.0

XLABEL = 'Thread Number'
YLABEL = 'Throughput (#Events/min)'

LINESTYLES = ["dashed", "dotted", "dashdot", "solid"]

#-------------------------------------------------------------------------------
def load_data(dataset):

    pattern = rf'{dataset}/sim_(\d+)mt'
    def extract_number(paths):
        match = re.search(pattern, paths)
        return int(match.group(1)) if match else float('inf')

    scores = {
        'thread_number' : [],
        'throughput'    : []
    }

    paths = sorted(glob.glob(f'{dataset}/sim_*mt'), key=extract_number)
    for path in paths:
        filepaths = glob.glob(f"{path}/benchmark*.json")
        counter = 0
        thread_number = 0
        throughputs = np.array([])
        for file in filepaths:
            counter += 1
            with open(file, "r") as fin:
                js = json.load(fin)
                if counter == 1:
                    thread_number = js['summary']['thread_number']
                throughputs = np.append(throughputs, js['summary']['throughput'])
        scores['thread_number'].append(thread_number)
        scores['throughput'].append(np.mean(throughputs))

    return scores

#-------------------------------------------------------------------------------
def makeplot(scores, pdf):
    fig, ax = plt.subplots(figsize=(FIGSIZE_X, FIGSIZE_Y), dpi=DPI)
    counter = 0
    for key in scores.keys():
        data = scores[key]
        ax.plot(data['thread_number'], data['throughput'], label=DATASET[key],
                linewidth=LINEWIDTH, linestyle=LINESTYLES[counter%4])
        counter += 1

    ax.set_title(CPUINFO, fontsize=FONTSIZE)
    ax.set_xlabel(XLABEL, fontsize=FONTSIZE)
    ax.set_ylabel(YLABEL, fontsize=FONTSIZE)

    ax.set_xlim(0, 35)
    ax.set_ylim(0, 100000)

    ax.xaxis.set_minor_locator(mpl.ticker.AutoMinorLocator())
    ax.yaxis.set_minor_locator(mpl.ticker.AutoMinorLocator())
    ax.yaxis.set_major_formatter(ptick.ScalarFormatter(useMathText=True))
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    ax.tick_params(axis='both', which='major', labelsize=FONTSIZE,
                   top=True, right=True, direction='in')
    ax.tick_params(axis='both', which='minor',
                   top=True, right=True, direction='in')
    ax.xaxis.grid(True, which = "major", linestyle = "dotted", color='black')
    ax.yaxis.grid(True, which = "major", linestyle = "dotted", color='black')

    ax.legend(fontsize=FONTSIZE_LEGEND, loc='upper right')

    pos = ax.get_position()
    ax.set_position([0.15, 0.15, pos.width * 0.95, pos.height * 0.95])

    pdf.savefig(fig)

#-------------------------------------------------------------------------------
# Main Function
#-------------------------------------------------------------------------------
def main(args):
    scores = {}
    for key in DATASET.keys():
        scores[key] = load_data(key)

    with PdfPages(OUTPUT_FILENAME) as pdf:
        makeplot(scores, pdf)

    if args.pdf:
        os.system(f'evince {OUTPUT_FILENAME}')

#===============================================================================
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Make scalability plot")
    parser.add_argument('-p', '--pdf',  action='store_true',
                        help='Launch PDF viewer')
    main(parser.parse_args())
