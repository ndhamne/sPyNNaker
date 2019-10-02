# Copyright (c) 2017-2019 The University of Manchester
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from spynnaker.pyNN.models.neuron import AbstractPyNNNeuronModelStandard
from spynnaker.pyNN.models.defaults import default_initial_values
from spynnaker.pyNN.models.neuron.neuron_models import (
    NeuronModelEProp)
from spynnaker.pyNN.models.neuron.synapse_types import (
    SynapseTypeEPropAdaptive)
from spynnaker.pyNN.models.neuron.input_types import InputTypeCurrent
from spynnaker.pyNN.models.neuron.threshold_types import ThresholdTypeAdaptive

class EPropAdaptive(AbstractPyNNNeuronModelStandard):
    """ Adaptive threshold neuron with eprop support
    """

    @default_initial_values({"v", "isyn_exc", "isyn_exc2", "isyn_inh", 
                             "isyn_inh2", "psi", "target_rate", "tau_err",
                             "B", "small_b"})
    def __init__(
            self,
            # neuron model params
            tau_m=20.0, cm=1.0, v_rest=-65.0, v_reset=-65.0,
            tau_refrac=5, i_offset=0.0, v=-65.0,  psi=0.0,

            #synapse type params
            tau_syn_E=5.0, tau_syn_E2=5.0, tau_syn_I=5.0, tau_syn_I2=5.0,
            isyn_exc=0.0, isyn_exc2=0.0, isyn_inh=0.0, isyn_inh2=0.0,
            
            # Regularisation params
            target_rate=10, tau_err=1000,  #  fits with 1 ms timestep
            
            # Threshold parameters
            B=10, small_b=0, small_b_0=10, tau_a=500, beta=1.8
            
            ):
        # pylint: disable=too-many-arguments, too-many-locals
        neuron_model = NeuronModelEProp(
            v, v_rest, tau_m, cm, i_offset, v_reset, tau_refrac, psi, 
            target_rate, tau_err)
        
        synapse_type = SynapseTypeEPropAdaptive(
            tau_syn_E, tau_syn_E2, tau_syn_I, tau_syn_I2, 
            isyn_exc, isyn_exc2, isyn_inh, isyn_inh2)
        
        input_type = InputTypeCurrent()
        
        threshold_type = ThresholdTypeAdaptive(B,
                                               small_b,
                                               small_b_0,
                                               tau_a,
                                               beta)

        super(EPropAdaptive, self).__init__(
            model_name="eprop_adaptive", binary="eprop_adaptive.aplx",
            neuron_model=neuron_model, input_type=input_type,
            synapse_type=synapse_type, threshold_type=threshold_type)