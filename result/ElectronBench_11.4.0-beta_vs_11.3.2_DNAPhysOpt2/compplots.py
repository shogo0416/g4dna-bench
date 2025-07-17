#!/usr/bin/env python3
"""
================================================================================
  BSD 2-Clause License

  Copyright (c) 2021-2023 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
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
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ptick
from matplotlib.backends.backend_pdf import PdfPages
import math
import os
import numpy as np

OUTPUT_FILENAME= "compgval.pdf"

INPUTS = [
    "./11.3.2/sim_1mt/gval.pkl",
    "./11.4.0-beta/sim_1mt/gval.pkl",
    "./11.3-ref6/sim_1mt/gval.pkl",
]

LABELS = [
    "IRT (11.3.2, DNAPhysOpt2)",
    "IRT (11.4.0-beta, DNAPhysOpt2)",
    "IRT (11.3-ref6, DNAPhysOpt2)",
]

XAXIS_LOG_SCALE = True

#-------------------------------------------------------------------------------
LINESTYLE = ["solid", "dashed", "dashdot", "dotted"]

TITLE = {
    "e_aq": r"$\mathrm{e}_{\mathrm{aq}}^{-}$",
    "H3Op": r"$\mathrm{H}_3\mathrm{O}^{+}$",
    "OH"  : r"$^{\cdot}\mathrm{OH}$",
    "OHm" : r"$\mathrm{OH}^{-}$",
    "H2"  : r"$\mathrm{H}_2$",
    "H2O2": r"$\mathrm{H}_2\mathrm{O}_2$",
    "H"   : r"$\mathrm{H}^{\cdot}$",
    "O"   : r"$\mathrm{O(3P)}$",
    "Om"  : r"$\mathrm{O}^{\cdot-}$",
    "O2"  : r"$\mathrm{O}_2$",
    "O2m" : r"$\mathrm{O}_2^{-}$",
    "O3"  : r"$\mathrm{O}_3$",
    "O3m" : r"$\mathrm{O}_3^{-}$",
    "HO2" : r"$\mathrm{HO}_2^{\cdot}$",
    "HO2m": r"$\mathrm{HO}_2^{\cdot-}$"
}

FIG_SIZE_X = 6
FIG_SIZE_Y = 4
DPI = 60
FONT_SIZE = 14
LINE_WIDTH = 3.0

XLABEL = "Time (ps)"
YLABEL = "G value (#molecules / 100 eV)"

XTICKS = []

#-------------------------------------------------------------------------------
def get_title(kind):
    if kind in TITLE:
        return TITLE[kind]
    return kind

#-------------------------------------------------------------------------------
def plot(kind, title, dfs, pdf):

    if title == 'consumption' or title == 'production':
        return

    num = len(dfs)
    ax = dfs[0].plot(x="time", y=kind, logx=XAXIS_LOG_SCALE,
                     linewidth=LINE_WIDTH, linestyle=LINESTYLE[0],
                     label=LABELS[0], figsize=(FIG_SIZE_X, FIG_SIZE_Y),
                     fontsize=FONT_SIZE)

    for x in range(1, num):
        dfs[x].plot(x="time", y=kind, ax=ax, linewidth=LINE_WIDTH,
                    linestyle=LINESTYLE[x % 4], label=LABELS[x])

    ax.set_xlabel(XLABEL, fontdict = {"fontsize": FONT_SIZE})

    if title == 'OCR2':
        ax.set_ylabel('Yields (uM)', fontdict = {"fontsize": FONT_SIZE})
        ax.set_title('OCR',          fontdict = {"fontsize": FONT_SIZE})
    elif title == 'OCR1':
        ax.set_ylabel(YLABEL, fontdict = {"fontsize": FONT_SIZE})
        ax.set_title('OCR',   fontdict = {"fontsize": FONT_SIZE})
    elif title == 'consumption':
        ax.set_ylabel(YLABEL, fontdict = {"fontsize": FONT_SIZE})
        ax.set_title(title,   fontdict = {"fontsize": FONT_SIZE})
    else:
        ymin, ymax = ax.get_ylim()
        ax.set_ylim(0.0, ymax * 1.2)
        ax.set_ylabel(YLABEL, fontdict = {"fontsize": FONT_SIZE})
        ax.set_title(title,   fontdict = {"fontsize": FONT_SIZE})

    if XAXIS_LOG_SCALE:
        ax.set_xticks(XTICKS)

    ax.set_xlim(1.0, 1.0E+06)
    ax.tick_params(axis='both', which='major', labelsize=FONT_SIZE,
                   top=True, right=True, direction='in')
    ax.tick_params(axis='both', which='minor',
                   top=True, right=True, direction='in')
    ax.xaxis.grid(True, which = "major", linestyle = "dotted", color="black")
    ax.yaxis.grid(True, which = "major", linestyle = "dotted", color="black")

    ax.yaxis.set_major_formatter(ptick.ScalarFormatter(useMathText=True))
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    ax.minorticks_on()

    pos = ax.get_position()
    ax.set_position([0.15, 0.15, pos.width * 0.95, pos.height * 0.95])

    pdf.savefig()

#===============================================================================
# main function
#===============================================================================
def main(show_pdf):
    # read pickle files
    dfs = []
    for x in INPUTS:
        dfs.append(pd.read_pickle(x))
    num = len(dfs)

    # setup for xlabel
    score_time = dfs[num - 1]["time"]
    num_node   = len(score_time)
    log10_start_time = math.log10(score_time[0])
    log10_end_time   = np.ceil(math.log10(score_time[num_node - 1]))
    for x in range(int(log10_start_time), int(log10_end_time) + 1):
        XTICKS.append(pow(10.0, x))

    # make plots
    with PdfPages(OUTPUT_FILENAME) as pdf:
        for kind in dfs[num - 1].columns:
            if kind == 'time':
                continue
            if kind in dfs[0].columns:
                if kind == 'edep':
                    continue
                plot(kind, get_title(kind), dfs, pdf)

    # show pdf file
    if show_pdf:
        os.system(f'evince {OUTPUT_FILENAME}')

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
      description="Compared with the other simulation results")
    parser.add_argument('-p', '--pdf',  action='store_true',
                        help='Launch PDF viewer')
    args = parser.parse_args()
    main(args.pdf)
