
import graphviz
class DrawCall:
    def __init__(self) -> None:
        self.classGraphs = dict()
        self.nodes = set()
        self.visitedTable = set()
        self.edges = []

    def createClassCluster(self, name):
        clusterName = "cluster_" + name
        cluster = graphviz.Digraph(name=clusterName)
        cluster.graph_attr["label"] = name
        return cluster

    def scanCall(self, node, callMap):
        if node in self.visitedTable:
            return
        self.visitedTable.add(node)
        callList = callMap.get(node) or []

        for caller in callList:
            [callerName, callerSelector] = caller.split("/", 1)
            classGraph = self.classGraphs.get(callerName)
            if not classGraph:
                classGraph = self.createClassCluster(callerName)
                self.classGraphs[callerName] = classGraph

            if caller not in self.nodes:
                classGraph.node(caller,callerSelector)
                self.nodes.add(caller)
            # classGraph.edge(caller, node)
            # 他大爷的，这里graphviz有点bug，需要前置声明
            self.edges.append((caller,node))
            self.scanCall(caller, callMap)
        return

    def generateGraph(self, diffs, callerMap):
        g = graphviz.Digraph(comment='The Call Graph')
        g.graph_attr["bgcolor"] = "#FFFFDD"
        g.node_attr["shape"] = "box"
        for diff in diffs:
            [name, sel] = diff.split("/", 1)
            graph = self.classGraphs.get(name)
            if not graph:
                graph = self.createClassCluster(name)
                self.classGraphs[name] = graph
            graph.node(diff, sel, color='red')
            self.nodes.add(diff)
        for diff in diffs:
            self.scanCall(diff, callerMap)
        for cls in self.classGraphs.values():
            g.subgraph(cls)
        for edge in self.edges:
            (s, e) = edge
            g.edge(s, e)
        return g




