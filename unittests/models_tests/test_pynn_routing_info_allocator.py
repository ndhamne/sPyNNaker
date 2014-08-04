import unittest
from pacman.model.subgraph.subgraph import Subgraph
from pacman.model.subgraph.subedge import Subedge
from pacman.model.subgraph.subvertex import Subvertex

from pacman.model.graph.vertex import Vertex

from pacman.model.graph_subgraph_mapper.graph_subgraph_mapper \
    import GraphSubgraphMapper
from spynnaker.pyNN.overrided_pacman_functions.pynn_routing_info_allocator \
    import PyNNRoutingInfoAllocator
from pacman.model.placements.placement import Placement
from pacman.model.placements.placements import Placements


class TestPyNNRoutingInfoAllocator(unittest.TestCase):
    def test_key_mask_combo(self):
        ria = PyNNRoutingInfoAllocator(None)
        self.assertEqual(ria.get_key_mask_combo(0xf1f2,0x00ff),0xf2)
        self.assertEqual(ria.get_key_mask_combo(0xf1f2,0x0f0f),0x0102)
        self.assertEqual(ria.get_key_mask_combo(0xf1f2,0xf00f),0xf002)
        self.assertEqual(ria.get_key_mask_combo(0xf1f2,0xf0f0),0xf0f0)
        self.assertEqual(ria.get_key_mask_combo(0xf1f2,0xffff),0xf1f2)

    def test_add_subgraph_and_placement(self):
        gsm = GraphSubgraphMapper()
        ria = PyNNRoutingInfoAllocator(gsm)
        subvertices = list()
        subedges = list()
        for i in range(10):
            subvertices.append(Subvertex(i*10,(i+1)*10 - 1))
        for i in range(5):
            subedges.append(Subedge(subvertices[0],subvertices[(i+1)]))
        for i in range(5,10):
            subedges.append(Subedge(subvertices[5],subvertices[(i+1)%10]))
        subgraph = Subgraph(None, subvertices, subedges)

        subv = Subvertex(0,100)
        pl = Placement(subv,0,0,1)
        pls = Placements([pl])
        ria.allocate_routing_info(subgraph, pls)

    def test_add_subgraph_and_placement_with_proper_graph_subgraph_mapper(self):

        subvertices = list()
        subedges = list()
        for i in range(10):
            subvertices.append(Subvertex(i*10,(i+1)*10 - 1))
        for i in range(5):
            subedges.append(Subedge(subvertices[0],subvertices[(i+1)]))
        for i in range(5,10):
            subedges.append(Subedge(subvertices[5],subvertices[(i+1)%10]))
        subgraph = Subgraph(None, subvertices, subedges)
        gsm = GraphSubgraphMapper()
        gsm.add_subvertices(subvertices[0:3],Vertex(30,"First vertex"))
        gsm.add_subvertices(subvertices[3:6],Vertex(60,"Second vertex"))
        gsm.add_subvertices(subvertices[6:10],Vertex(100,"Third vertex"))
        ria = PyNNRoutingInfoAllocator(gsm)
        subv = Subvertex(0,100)
        pl = Placement(subv,0,0,1)
        pls = Placements([pl])
        ria.allocate_routing_info(subgraph, pls)

    def test_check_masks(self):
        pass




if __name__ == '__main__':
    unittest.main()
