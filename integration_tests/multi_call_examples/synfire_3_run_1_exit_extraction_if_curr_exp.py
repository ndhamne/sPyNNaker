"""
Synfirechain-like example
"""
import unittest

import spynnaker.plot_utils as plot_utils
import spynnaker.spike_checker as spike_checker

import synfire_run_thrice as synfire_run_thrice


class Synfire3Run1ExitExtractionIfCurrExp(unittest.TestCase):

    def test_run(self):
        nNeurons = 200  # number of neurons in each population
        results = synfire_run_thrice.do_run(nNeurons)
        (v1, gsyn1, spikes1, v2, gsyn2, spikes2, v3, gsyn3, spikes3) = results
        print len(spikes1)
        print len(spikes2)
        print len(spikes3)
        plot_utils.plot_spikes(spikes1, spikes2, spikes3)
        plot_utils.heat_plot(v1, title="v1")
        plot_utils.heat_plot(gsyn1, title="gysn1")
        plot_utils.heat_plot(v2, title="v2")
        plot_utils.heat_plot(gsyn2, title="gysn2")
        plot_utils.heat_plot(v3, title="v3")
        plot_utils.heat_plot(gsyn3, title="gysn3")
        #self.assertEquals(53, len(spikes1))
        #self.assertEquals(156, len(spikes2))
        spike_checker.synfire_spike_checker(spikes1, nNeurons)
        spike_checker.synfire_multiple_lines_spike_checker(spikes2, nNeurons, 2)
        spike_checker.synfire_multiple_lines_spike_checker(spikes3, nNeurons, 3)



if __name__ == '__main__':
    pass