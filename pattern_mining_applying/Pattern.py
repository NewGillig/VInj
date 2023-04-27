import const
import AST


class TreePattern:
    def __init__(self, ast, source_asts):
        self.__ast = ast
        self.__source_asts = source_asts      # record the asts that can match the pattern

    def mergePattern(self,pattern):
        pass

class EditPattern:
    def __init__(self, before_tree_pattern, after_tree_pattern, source_pairs, node_map, reversed_node_map, inserted_trees, deleted_trees, updated_nodes, modified_trees):
        self.__before_tree_pattern = before_tree_pattern
        self.__after_tree_pattern = after_tree_pattern
        self.__source_pairs = source_pairs
        
        self.__node_map = node_map
        self.__reversed_node_map = reversed_node_map
        self.__inserted_trees = inserted_trees
        self.__deleted_trees = deleted_trees
        self.__updated_nodes = updated_nodes
        self.__modified_trees = modified_trees

        self.__stripped_before_tree_pattern = AST.GumTreeAst([])
        self.__stripped_after_tree_pattern = AST.GumTreeAst([])

        
        self.__stripped_before_node_map = {}       # "0: 1" means the node 0 in the stripped before tree matches the node 1 in original before tree
        self.__stripped_after_node_map = {}
        self.__reversed_stripped_before_node_map = {}       # "0: 1" means the node 0 in the original before tree matches the node 1 in stripped before tree
        self.__reversed_stripped_after_node_map = {}
        


        self.__stripped_before_tree_map = {}       # "15: [[previous_trees],[following_trees]]" means the previous_trees are originally previous the node 15 as a sibling. 
        self.__stripped_after_tree_map = {}
        self.__all_stripped_before_root_map = {}
        self.__all_stripped_after_root_map = {}

        self.__stripped_after_parents_ids = set()       
        # self.__stripped_before_parents_ids = set()
        self.__stripped_node_map = {}
        self.__stripped_reversed_node_map = {}
        self.__stripped_inserted_trees = set()
        self.__stripped_deleted_trees = set()
        self.__stripped_updated_nodes = set()
        self.__stripped_modified_trees = set()

        self.__getStrippedBeforeTree()
        self.__getStrippedAfterTree()

        for stripped_before_node_id in self.__stripped_before_node_map:
            original_before_node_id = self.__stripped_before_node_map[stripped_before_node_id]
            #if original_before_node_id==18:
            #    import pdb
            #    pdb.set_trace()
            #if len(self.__stripped_before_tree_pattern.getNodeDict()[stripped_before_node_id].children) < len(self.__before_tree_pattern.getNodeDict()[original_before_node_id].children):
            #    self.__stripped_before_parents_ids.add(stripped_before_node_id)
            if original_before_node_id in self.__node_map:
                self.__stripped_node_map[stripped_before_node_id] = self.__reversed_stripped_after_node_map[self.__node_map[original_before_node_id]]
                self.__stripped_reversed_node_map[self.__reversed_stripped_after_node_map[self.__node_map[original_before_node_id]]] = stripped_before_node_id
            if original_before_node_id in self.__deleted_trees:
                self.__stripped_deleted_trees.add(stripped_before_node_id)
            if original_before_node_id in self.__updated_nodes:
                self.__stripped_updated_nodes.add(stripped_before_node_id)
            if original_before_node_id in self.__modified_trees:
                self.__stripped_modified_trees.add(stripped_before_node_id)

        for stripped_after_node_id in self.__stripped_after_node_map:
            original_after_node_id = self.__stripped_after_node_map[stripped_after_node_id]
            if len(self.__stripped_after_tree_pattern.getNodeDict()[stripped_after_node_id].children) < len(self.__after_tree_pattern.getNodeDict()[original_after_node_id].children):
                self.__stripped_after_parents_ids.add(stripped_after_node_id)
            if original_after_node_id in self.__inserted_trees:
                self.__stripped_inserted_trees.add(stripped_after_node_id)
        
        self.__has_unbound_holes = False
        self.__before_holes_set = set()
        self.__after_holes_set = set()
        for node_id in self.__before_tree_pattern.getNodeDict():
            if "!@#$hole" in self.__before_tree_pattern.getNodeDict()[node_id].value:
                self.__before_holes_set.add(self.__before_tree_pattern.getNodeDict()[node_id].value)

        for node_id in self.__after_tree_pattern.getNodeDict():
            if "!@#$hole" in self.__after_tree_pattern.getNodeDict()[node_id].value:
                self.__after_holes_set.add(self.__after_tree_pattern.getNodeDict()[node_id].value)

        for holes in self.__after_holes_set:
            if not holes in self.__before_holes_set:  
                self.__has_unbound_holes = True
        
    def __len__(self):
        return len(self.__before_tree_pattern) + len(self.__after_tree_pattern)

    def __eq__(self, other):
        return self.__before_tree_pattern == other.__before_tree_pattern and self.__after_tree_pattern == other.__after_tree_pattern
    
    def hasUnboundHoles(self):
        return self.__has_unbound_holes

    def isAmbiguous(self):
        root_node = self.__before_tree_pattern.getRootNode()
        if root_node.type.type_name == '?':
            return True
        if len(root_node.children) > 0:
            is_ambiguous = True
            for child in root_node.children:
                if child.type.type_name != '?':
                    is_ambiguous = False
            if is_ambiguous:
                return True
            for child in root_node.children:
                if len(child.children) > 0:
                    if self.__isAmbiguousHelper(child) == True:
                        return True
        return False


    def __isAmbiguousHelper(self, root_node):
        is_ambiguous = True
        for child in root_node.children:
            if child.type.type_name != '?':
                is_ambiguous = False
        if is_ambiguous:
            return True
        for child in root_node.children:
            if len(child.children) > 0:
                if self.__isAmbiguousHelper(child):
                    return True
        return False
                    

    def getBeforeHolesSet(self):
        return self.__before_holes_set

    def getAfterHolesSet(self):
        return self.__after_holes_set

    def __str__(self):
        before_tree_pattern = self.__before_tree_pattern
        after_tree_pattern = self.__after_tree_pattern
        string = self.__before_tree_pattern.__str__() + "\n----------------------------------------\n" + self.__after_tree_pattern.__str__()
        return string

    def getCode(self):
        before_tree_pattern = self.__before_tree_pattern
        after_tree_pattern = self.__after_tree_pattern
        string = self.__before_tree_pattern.getCode() + "\n----------------------------------------\n" + self.__after_tree_pattern.getCode()
        return string

    def getBeforePattern(self):
        return self.__before_tree_pattern

    def getAfterPattern(self):
        return self.__after_tree_pattern

    def getSourcePairs(self):
        return self.__source_pairs
    
    def getNodeMap(self):
        return self.__node_map

    def getReversedNodeMap(self):
        return self.__reversed_node_map

    def getInsertedTrees(self):
        return self.__inserted_trees

    def getDeletedTrees(self):
        return self.__deleted_trees

    def getUpdatedNodes(self):
        return self.__updated_nodes

    def getModifiedTrees(self):
        return self.__modified_trees

    def getStrippedBeforeTree(self):
        return self.__stripped_before_tree_pattern

    def getStrippedAfterTree(self):
        return self.__stripped_after_tree_pattern

    def getStrippedBeforeNodeMap(self):
        return self.__stripped_before_node_map

    def getStrippedAfterNodeMap(self):
        return self.__stripped_after_node_map

    def getStrippedBeforeTreeMap(self):
        return self.__stripped_before_tree_map

    def getStrippedAfterTreeMap(self):
        return self.__stripped_after_tree_map

    def getAllStrippedBeforeRootMap(self):
        return self.__all_stripped_before_root_map

    def getAllStrippedAfterRootMap(self):
        return self.__all_stripped_after_root_map

    def getStrippedNodeMap(self):
        return self.__stripped_node_map

    def getStrippedReversedNodeMap(self):
        return self.__stripped_reversed_node_map

    def getStrippedInsertedTrees(self):
        return self.__stripped_inserted_trees

    def getStrippedDeletedTrees(self):
        return self.__stripped_deleted_trees

    def getStrippedUpdatedNodes(self):
        return self.__stripped_updated_nodes

    def getStrippedModifiedTrees(self):
        return self.__stripped_modified_trees

    def __getStrippedBeforeTree(self):
        root_node = self.__before_tree_pattern.getRootNode()
        if root_node == None:
            return 
        self.__stripped_before_tree_pattern.addRootNode(root_node.type, root_node.value)
        new_root_node = self.__stripped_before_tree_pattern.getRootNode()
        self.__stripped_before_node_map[new_root_node.getIdInAst()] = root_node.getIdInAst()
        self.__reversed_stripped_before_node_map[root_node.getIdInAst()] = new_root_node.getIdInAst()
        
        stripped_trees = []
        last_modified_node_id = -1
        all_stripped = True
        for i, child in enumerate(root_node.children):
            if not child.getIdInAst() in self.__node_map or child.getIdInAst() in self.__modified_trees:
                all_stripped = False
                new_child_id = self.__addStrippedBeforeTree(new_root_node, child)
                new_root_node = self.__stripped_before_tree_pattern.getRootNode()
                self.__stripped_before_tree_map[new_child_id] = [stripped_trees, []]
                if not last_modified_node_id == -1:
                    self.__stripped_before_tree_map[last_modified_node_id][1] = stripped_trees
                stripped_trees = []
                last_modified_node_id = new_child_id

            else:
                stripped_trees.append(self.__before_tree_pattern.getSubtree(child.getIdInAst()))
                # if len(new_root_node.children) == 0:
                    # self.__stripped_before_tree_map[-(new_root_node.getIdInAst())] = self.__before_tree_pattern.getSubtree(child.getIdInAst())
                # else:
                    # self.__stripped_before_tree_map[new_root_node.children[-1].getIdInAst()] = self.__before_tree_pattern.getSubtree(child.getIdInAst())
        if all_stripped:
            self.__all_stripped_before_root_map[new_root_node.getIdInAst()] = stripped_trees

        if not last_modified_node_id == -1:
            self.__stripped_before_tree_map[last_modified_node_id][1] = stripped_trees
    
    def __addStrippedBeforeTree(self, new_root_node, child):
        new_node_id = self.__stripped_before_tree_pattern.addSingleNode(child.type, child.value, new_root_node.getIdInAst(), len(new_root_node.children))
        #if len(new_root_node.children)>0:
        #    import pdb
        #    pdb.set_trace()
        new_node = self.__stripped_before_tree_pattern.getNodeDict()[new_node_id]
        self.__stripped_before_node_map[new_node_id] = child.getIdInAst()
        self.__reversed_stripped_before_node_map[child.getIdInAst()] = new_node_id

        stripped_trees = []
        last_modified_node_id = -1
        all_stripped = True        
        for i, childx in enumerate(child.children):
            if not childx.getIdInAst() in self.__node_map or childx.getIdInAst() in self.__modified_trees:
                all_stripped = False
                new_childx_id = self.__addStrippedBeforeTree(new_node, childx)
                new_node = self.__stripped_before_tree_pattern.getNodeDict()[new_node_id]
                self.__stripped_before_tree_map[new_childx_id] = [stripped_trees, []]
                if not last_modified_node_id == -1:
                    self.__stripped_before_tree_map[last_modified_node_id][1] = stripped_trees
                stripped_trees = []
                last_modified_node_id = new_childx_id

            else:
                stripped_trees.append(self.__before_tree_pattern.getSubtree(childx.getIdInAst()))
                # if len(new_node.children) == 0:
                    # self.__stripped_before_tree_map[-(new_node.getIdInAst())] = self.__before_tree_pattern.getSubtree(childx.getIdInAst())
                # else:
                    # self.__stripped_before_tree_map[new_node.children[-1].getIdInAst()] = self.__before_tree_pattern.getSubtree(childx.getIdInAst())
        if all_stripped:
            self.__all_stripped_before_root_map[new_node_id] = stripped_trees
        if not last_modified_node_id == -1:
            self.__stripped_before_tree_map[last_modified_node_id][1] = stripped_trees
        
        return new_node_id

    def __getStrippedAfterTree(self):
        root_node = self.__after_tree_pattern.getRootNode()
        if root_node == None:
            return
        self.__stripped_after_tree_pattern.addRootNode(root_node.type, root_node.value)
        new_root_node = self.__stripped_after_tree_pattern.getRootNode()
        self.__stripped_after_node_map[new_root_node.getIdInAst()] = root_node.getIdInAst()
        self.__reversed_stripped_after_node_map[root_node.getIdInAst()] = new_root_node.getIdInAst()

        stripped_trees = []
        last_modified_node_id = -1
        all_stripped = True
        for i, child in enumerate(root_node.children):
            if not child.getIdInAst() in self.__reversed_node_map or (child.getIdInAst() in self.__reversed_node_map and self.__reversed_node_map[child.getIdInAst()] in self.__modified_trees):
                all_stripped = False
                new_child_id = self.__addStrippedAfterTree(new_root_node, child)
                new_root_node = self.__stripped_after_tree_pattern.getRootNode()
                self.__stripped_after_tree_map[new_child_id] = [stripped_trees, []]
                if not last_modified_node_id == -1:
                    self.__stripped_after_tree_map[last_modified_node_id][1] = stripped_trees
                stripped_trees = []
                last_modified_node_id = new_child_id
            else:
                stripped_trees.append(self.__after_tree_pattern.getSubtree(child.getIdInAst()))
                # if len(new_root_node.children) == 0:
                    # self.__stripped_after_tree_map[-(new_root_node.getIdInAst())] = self.__after_tree_pattern.getSubtree(child.getIdInAst())
                # else:
                    # self.__stripped_after_tree_map[new_root_node.children[-1].getIdInAst()] = self.__after_tree_pattern.getSubtree(child.getIdInAst())
        if all_stripped:
            self.__all_stripped_after_root_map[new_root_node.getIdInAst()] = stripped_trees
        if not last_modified_node_id == -1:
            self.__stripped_after_tree_map[last_modified_node_id][1] = stripped_trees
    
    def __addStrippedAfterTree(self, new_root_node, child):
        new_node_id = self.__stripped_after_tree_pattern.addSingleNode(child.type, child.value, new_root_node.getIdInAst(), len(new_root_node.children))
        new_node = self.__stripped_after_tree_pattern.getNodeDict()[new_node_id]
        self.__stripped_after_node_map[new_node_id] = child.getIdInAst()
        self.__reversed_stripped_after_node_map[child.getIdInAst()] = new_node_id

        stripped_trees = []
        last_modified_node_id = -1
        
        all_stripped = True
        for i, childx in enumerate(child.children):
            if not childx.getIdInAst() in self.__reversed_node_map or (childx.getIdInAst() in self.__reversed_node_map and self.__reversed_node_map[childx.getIdInAst()] in self.__modified_trees):
                all_stripped = False
                new_childx_id = self.__addStrippedAfterTree(new_node, childx)
                new_node = self.__stripped_after_tree_pattern.getNodeDict()[new_node_id]
                self.__stripped_after_tree_map[new_childx_id] = [stripped_trees, []]
                if not last_modified_node_id == -1:
                    self.__stripped_after_tree_map[last_modified_node_id][1] = stripped_trees
                stripped_trees = []
                last_modified_node_id = new_childx_id
            else:
                stripped_trees.append(self.__after_tree_pattern.getSubtree(childx.getIdInAst()))
                # if len(new_node.children) == 0:
                    # self.__stripped_after_tree_map[-(new_node.getIdInAst())] = self.__after_tree_pattern.getSubtree(childx.getIdInAst())
                # else:
                    # self.__stripped_after_tree_map[new_node.children[-1].getIdInAst()] = self.__after_tree_pattern.getSubtree(childx.getIdInAst())
        if all_stripped:
            self.__all_stripped_after_root_map[new_node_id] = stripped_trees
        if not last_modified_node_id == -1:
            self.__stripped_after_tree_map[last_modified_node_id][1] = stripped_trees    
        return new_node_id


    def mergeEditPattern(self, other):
        stripped_before_1 = self.__stripped_before_tree_pattern
        stripped_before_2 = other.__stripped_before_tree_pattern
        stripped_after_1 = self.__stripped_after_tree_pattern
        stripped_after_2 = other.__stripped_after_tree_pattern
        
        

        before_g_node_map = {}
        after_g_node_map = {}
        
        holes_dict = {}

        before_g = mergeTreePattern(stripped_before_1, stripped_before_2, holes_dict, before_g_node_map)
        after_g = mergeTreePattern(stripped_after_1, stripped_after_2, holes_dict, after_g_node_map)
        #import pdb
        #pdb.set_trace()

        # get the map information
        g_node_map = {}
        g_reversed_node_map = {}
        g_inserted_trees = set()
        g_deleted_trees = set()
        g_updated_nodes = set()
        g_modified_trees = set()
        g_stripped_after_parents_ids = set()
        g_all_stripped_before_root_ids = set()

        g_before_tree_map = {}          # 15: [[tree_1's stripped previous trees, tree_1's stripped followed trees], [tree_2's stripped previous trees, tree2_'s stripped followed trees]]
        g_after_tree_map = {}
        
        for before_g_node_id in before_g_node_map:
            for before_self_stripped_node_id in before_g_node_map[before_g_node_id][0]:
                if not before_self_stripped_node_id in self.__stripped_node_map:
                    continue
                after_self_stripped_node_id = self.__stripped_node_map[before_self_stripped_node_id]
                flag = False
                for after_g_node_id in after_g_node_map:
                    if after_self_stripped_node_id in after_g_node_map[after_g_node_id][0]:
                        g_node_map[before_g_node_id] = after_g_node_id
                        g_reversed_node_map[after_g_node_id] = before_g_node_id
                        flag = True
                        break
                if flag:
                    break
            
        for before_g_node_id in before_g_node_map:
            if before_g_node_map[before_g_node_id][0][0] in self.__stripped_deleted_trees or before_g_node_map[before_g_node_id][1][0] in other.__stripped_deleted_trees:
                g_deleted_trees.add(before_g_node_id)
            if before_g_node_map[before_g_node_id][0][0] in self.__stripped_modified_trees or before_g_node_map[before_g_node_id][1][0] in other.__stripped_modified_trees:
                g_modified_trees.add(before_g_node_id)
            if before_g_node_map[before_g_node_id][0][0] in self.__stripped_updated_nodes or before_g_node_map[before_g_node_id][1][0] in other.__stripped_updated_nodes:
                g_updated_nodes.add(before_g_node_id)
            if before_g_node_map[before_g_node_id][0][0] in self.__stripped_before_tree_map and before_g_node_map[before_g_node_id][1][0] in other.__stripped_before_tree_map:
                g_before_tree_map[before_g_node_id] = [self.__stripped_before_tree_map[before_g_node_map[before_g_node_id][0][0]], other.__stripped_before_tree_map[before_g_node_map[before_g_node_id][1][0]]]
            #if before_g_node_map



        for after_g_node_id in after_g_node_map:
            if after_g_node_map[after_g_node_id][0][0] in self.__stripped_inserted_trees or after_g_node_map[after_g_node_id][1][0] in other.__stripped_inserted_trees:
                g_inserted_trees.add(after_g_node_id)
            if after_g_node_map[after_g_node_id][0][0] in self.__stripped_after_parents_ids or after_g_node_map[after_g_node_id][1][0] in other.__stripped_after_parents_ids:   
                g_stripped_after_parents_ids.add(after_g_node_id)
            if after_g_node_map[after_g_node_id][0][0] in self.__stripped_after_tree_map and after_g_node_map[after_g_node_id][1][0] in other.__stripped_after_tree_map:
                g_after_tree_map[after_g_node_id] = [self.__stripped_after_tree_map[after_g_node_map[after_g_node_id][0][0]], other.__stripped_after_tree_map[after_g_node_map[after_g_node_id][1][0]]]



        # check the stripped subtrees on tree_1 and tree_2, only restore the common subtree
        before_g2 = AST.GumTreeAst([])
        after_g2 = AST.GumTreeAst([])

        before_g_root_node = before_g.getRootNode()
        after_g_root_node = after_g.getRootNode()

        before_g2.addRootNode(before_g_root_node.type, before_g_root_node.value)
        after_g2.addRootNode(after_g_root_node.type, after_g_root_node.value)

        before_g2_root_node = before_g2.getRootNode()
        after_g2_root_node = after_g2.getRootNode()

        g2_node_map = {}
        before_g2_node_map = {}
        after_g2_node_map = {}

        before_g2_node_map[0] = 0
        after_g2_node_map[0] = 0

        before_restored_node_ids = set()
        after_restored_node_ids = set()

        holes_dict_multi = {}
        
        if self.__after_tree_pattern.__str__()=='' and other.__after_tree_pattern.__str__()=='' :
            try:
                source_pairs = []
                source_pairs.extend(self.__source_pairs)
                source_pairs.extend(other.__source_pairs)
                #= self.__source_pairs.extend(other.__source_pairs)
            except:
                source_pairs = []
            return EditPattern(before_g, self.__after_tree_pattern, source_pairs, g_node_map, g_reversed_node_map, g_inserted_trees, g_deleted_trees, g_updated_nodes, g_modified_trees)
        #import pdb
        #pdb.set_trace()
        before_empty_parents = []
        after_empty_parents = []
        self.__all_stripped_before_g_root_ids = set()
        #self.__all_stripped_after_root_map
        self.__before_g2 =before_g2
        self.__before_g2_node_map = before_g2_node_map
        self.__before_restored_node_ids = before_restored_node_ids
        self.__restoreStrippedTrees(after_g_root_node, after_g2_root_node, after_g2, after_g2_node_map, g_after_tree_map, holes_dict, after_restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, False, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids)
        self.__restoreStrippedTrees(before_g_root_node, before_g2_root_node, before_g2, before_g2_node_map, g_before_tree_map, holes_dict, before_restored_node_ids, holes_dict_multi, before_empty_parents,after_empty_parents, True, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids)
        
        after_g2x = AST.GumTreeAst([])
        after_g_root_node = after_g.getRootNode()
        after_g2x.addRootNode(after_g_root_node.type, after_g_root_node.value)
        after_g2x_root_node = after_g2x.getRootNode()
        after_g2x_node_map = {}
        after_g2x_node_map[0] = 0
        after_restored_node_idsx = set()

        self.__restoreStrippedTrees(after_g_root_node, after_g2x_root_node, after_g2x, after_g2x_node_map, g_after_tree_map, holes_dict, after_restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, False, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2x, after_g2x_node_map, after_restored_node_idsx)

        g2_node_map = {}
        g2_reversed_node_map = {}
        g2_inserted_trees = set()
        g2_deleted_trees = set()
        g2_updated_nodes = set()
        g2_modified_trees = set()

        for before_restored_node_id in before_restored_node_ids:
            g2_node_map[before_restored_node_id] = -1

        for after_restored_node_id in after_restored_node_ids:
            g2_reversed_node_map[after_restored_node_id] = -1
        
        for before_g2_node_id in before_g2_node_map:
            before_g_node_id = before_g2_node_map[before_g2_node_id]
            if not before_g_node_id in g_node_map:
                continue
            after_g_node_id = g_node_map[before_g_node_id]
            for after_g2_node_id in after_g2_node_map:
                if after_g2_node_map[after_g2_node_id] == after_g_node_id:
                    g2_node_map[before_g2_node_id] = after_g2_node_id
                    g2_reversed_node_map[after_g2_node_id] = before_g2_node_id
                    break

        for before_g2_node_id in before_g2_node_map:
            if before_g2_node_map[before_g2_node_id] in g_deleted_trees:
                g2_deleted_trees.add(before_g2_node_id)
            if before_g2_node_map[before_g2_node_id] in g_modified_trees:
                g2_modified_trees.add(before_g2_node_id)
            if before_g2_node_map[before_g2_node_id] in g_updated_nodes:
                g2_updated_nodes.add(before_g2_node_id)

        for after_g2_node_id in after_g2_node_map:
            if after_g2_node_map[after_g2_node_id] in g_inserted_trees:
                g2_inserted_trees.add(after_g2_node_id)
        try:
            source_pairs = []
            source_pairs.extend(self.__source_pairs)
            source_pairs.extend(other.__source_pairs)
            #self.__source_pairs.extend(other.__source_pairs)
        except:
            source_pairs = []
        return EditPattern(before_g2, after_g2, source_pairs, g2_node_map, g2_reversed_node_map, g2_inserted_trees, g2_deleted_trees, g2_updated_nodes, g2_modified_trees)

        

    def __restoreStrippedTrees(self, g_root_node, g2_root_node, g2, g2_node_map, g_tree_map, holes_dict, restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, is_before_part, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids):
        if len(g_root_node.children) == 0 and is_before_part == False:
            after_empty_parents.append(g2_root_node)
        if len(g_root_node.children) == 0 and is_before_part == True:
            before_empty_parents.append(g2_root_node)
            self.__all_stripped_before_g_root_ids.add(g_root_node.getIdInAst())

        if "!@#$hole" in g_root_node.value and is_before_part == True:
            if g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                for empty_parent in after_empty_parents:
                    if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                        empty_parent.value = "!@#$hole"+str(len(holes_dict))
                        holes_dict["!@#$hole"+str(len(holes_dict))] = [AST.GumTreeAstNode(g2, -1), AST.GumTreeAstNode(g2, -1)]

        if "!@#$hole" in g_root_node.value and is_before_part == False:
            if g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                for empty_parent in before_empty_parents:
                    if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                        empty_parent.value = "!@#$hole"+str(len(holes_dict))
                        holes_dict["!@#$hole"+str(len(holes_dict))] = [AST.GumTreeAstNode(g2, -1), AST.GumTreeAstNode(g2, -1)]


        
        for i in range(len(g_root_node.children)):       # in the merging algorithm ,the num of children must be the same
            g_child = g_root_node.children[i]
            g_child_id = g_root_node.children[i].getIdInAst()

            g_child_previous_trees_1 = g_tree_map[g_child_id][0][0]
            g_child_previous_trees_2 = g_tree_map[g_child_id][1][0]
            g_child_followed_trees_1 = g_tree_map[g_child_id][0][1]
            g_child_followed_trees_2 = g_tree_map[g_child_id][1][1]

            if len(g_child_previous_trees_1) == 0 and len(g_child_previous_trees_2) == 0:
                # add the child directly
                new_child_id = g2.addSingleNode(g_child.type, g_child.value, g2_root_node.getIdInAst(), len(g2_root_node.children))
                g2_node_map[new_child_id] = g_child.getIdInAst()
                new_child = g2.getNodeDict()[new_child_id]
                self.__restoreStrippedTrees(g_child, new_child, g2, g2_node_map, g_tree_map, holes_dict, restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, is_before_part, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids)
            
            elif len(g_child_previous_trees_1) == len(g_child_previous_trees_2):
                # merge each previous trees and then add the child
                for j in range(len(g_child_previous_trees_1)):
                    merged_tree = mergeTreePattern(g_child_previous_trees_1[j], g_child_previous_trees_2[j], holes_dict)
                    node_mapx = {}
                    merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                    
                    if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                        for empty_parent in after_empty_parents:
                            if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                node_mapxx = {}
                                merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                for copied_id in node_mapxx:
                                    after_restored_node_ids.add(copied_id)
                    if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids: 
                        for empty_parent in before_empty_parents:
                            if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                node_mapxx = {}
                                merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                for copied_id in node_mapxx:
                                    self.__before_restored_node_ids.add(copied_id)

                    for copied_id in node_mapx:
                         restored_node_ids.add(copied_id)
                new_child_id = g2.addSingleNode(g_child.type, g_child.value, g2_root_node.getIdInAst(), len(g2_root_node.children))
                g2_node_map[new_child_id] = g_child.getIdInAst()
                new_child = g2.getNodeDict()[new_child_id]
                self.__restoreStrippedTrees(g_child, new_child, g2, g2_node_map, g_tree_map, holes_dict, restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, is_before_part, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids)

            else:
                flag = False
                if len(g_child_previous_trees_1) > len(g_child_previous_trees_2):
                    g_cut_previous_trees_1 =  g_child_previous_trees_1[:len(g_child_previous_trees_1)-len(g_child_previous_trees_2)]
                    g_cut_previous_trees_2 = []
                else:
                    g_cut_previous_trees_2 =  g_child_previous_trees_2[:len(g_child_previous_trees_2)-len(g_child_previous_trees_1)]
                    g_cut_previous_trees_1 = []
                for holes in holes_dict_multi:
                    if g_cut_previous_trees_1 == holes_dict_multi[holes][0] and g_cut_previous_trees_2 == holes_dict_multi[holes][1]:
                        hole_id = g2.addSingleNode(AST.GumTreeAstType('?'), holes, g2_root_node.getIdInAst(), len(g2_root_node.children))
                        restored_node_ids.add(hole_id)
                        flag = True
                        break
                if not flag:
                    hole_id = g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), g2_root_node.getIdInAst(), len(g2_root_node.children))
                    
                    if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                        for empty_parent in after_empty_parents:
                            if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                hole_idx = after_g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), empty_parent.getIdInAst(), len(empty_parent.children))
                                after_restored_node_ids.add(hole_idx)
                    if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                        for empty_parent in before_empty_parents:
                            if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                node_mapxx = {}
                                hole_idx = before_g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), empty_parent.getIdInAst(), len(empty_parent.children))
                                self.__before_restored_node_ids.add(hole_idx)
                    #if str(len(holes_dict_multi)+1000) == "1002":
                        #import pdb
                        #pdb.set_trace()
                    holes_dict_multi["!@#$holes"+str(len(holes_dict_multi)+1000)] = [g_cut_previous_trees_1, g_cut_previous_trees_2]
                    restored_node_ids.add(hole_id)
                    
                if len(g_child_previous_trees_1) > len(g_child_previous_trees_2):
                    len_diff = len(g_child_previous_trees_1) - len(g_child_previous_trees_2)
                    node_mapx = {}
                    for j in range(len(g_child_previous_trees_2)):
                        merged_tree = mergeTreePattern(g_child_previous_trees_1[j+len_diff], g_child_previous_trees_2[j], holes_dict)
                        node_mapx = {}
                        merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                        
                        if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                            for empty_parent in after_empty_parents:
                                if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        after_restored_node_ids.add(copied_id)
                        if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                            for empty_parent in before_empty_parents:
                                if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        self.__before_restored_node_ids.add(copied_id)

                        for copied_id in node_mapx:
                            restored_node_ids.add(copied_id)
                if len(g_child_previous_trees_2) > len(g_child_previous_trees_1):
                    len_diff = len(g_child_previous_trees_2) - len(g_child_previous_trees_1)
                    node_mapx = {}
                    for j in range(len(g_child_previous_trees_1)):
                        merged_tree = mergeTreePattern(g_child_previous_trees_1[j], g_child_previous_trees_2[j+len_diff], holes_dict)
                        node_mapx = {}
                        merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                        
                        if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                            for empty_parent in after_empty_parents:
                                if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        after_restored_node_ids.add(copied_id)
                        if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                            for empty_parent in before_empty_parents:
                                if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        self.__before_restored_node_ids.add(copied_id)
                        for copied_id in node_mapx:
                            restored_node_ids.add(copied_id)
                
                new_child_id = g2.addSingleNode(g_child.type, g_child.value, g2_root_node.getIdInAst(), len(g2_root_node.children))
                g2_node_map[new_child_id] = g_child.getIdInAst()
                new_child = g2.getNodeDict()[new_child_id]
                self.__restoreStrippedTrees(g_child, new_child, g2, g2_node_map, g_tree_map, holes_dict, restored_node_ids, holes_dict_multi, before_empty_parents, after_empty_parents, is_before_part, g_stripped_after_parents_ids, g_node_map, g_reversed_node_map, after_g2, after_g2_node_map, after_restored_node_ids)

            if i == len(g_root_node.children) - 1:
                if len(g_child_followed_trees_1) == 0 and len(g_child_followed_trees_2) == 0:
                    # do nothing
                    pass

                elif len(g_child_followed_trees_1) == len(g_child_followed_trees_2):
                    # merge each followed trees
                    for j in range(len(g_child_followed_trees_1)):
                        merged_tree = mergeTreePattern(g_child_followed_trees_1[j], g_child_followed_trees_2[j], holes_dict)
                        node_mapx = {}
                        merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                        
                        if is_before_part == True and g_root_node.getIdInAst() in g_node_map and  g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                            for empty_parent in after_empty_parents:
                                if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        after_restored_node_ids.add(copied_id)
                        if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                            for empty_parent in before_empty_parents:
                                if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                    for copied_id in node_mapxx:
                                        self.__before_restored_node_ids.add(copied_id)

                        for copied_id in node_mapx:
                            restored_node_ids.add(copied_id)
                else:
                    if len(g_child_followed_trees_1) > len(g_child_followed_trees_2):
                        g_cut_followed_trees_1 =  g_child_followed_trees_1[-(len(g_child_followed_trees_1)-len(g_child_followed_trees_2)):]
                        g_cut_followed_trees_2 = []
                    else:
                        g_cut_followed_trees_2 =  g_child_followed_trees_2[-(len(g_child_followed_trees_2)-len(g_child_followed_trees_1)):]
                        g_cut_followed_trees_1 = []
                    
                    
                    if len(g_child_followed_trees_1) > len(g_child_followed_trees_2):
                        len_diff = len(g_child_followed_trees_1) - len(g_child_followed_trees_2)
                        node_mapx = {}
                        for j in range(len(g_child_followed_trees_2)):
                            merged_tree = mergeTreePattern(g_child_followed_trees_1[j], g_child_followed_trees_2[j], holes_dict)
                            node_mapx = {}
                            merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                            
                            if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                                for empty_parent in after_empty_parents:
                                    if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                        node_mapxx = {}
                                        merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                        for copied_id in node_mapxx:
                                            after_restored_node_ids.add(copied_id)
                            if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                                for empty_parent in before_empty_parents:
                                    if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                        node_mapxx = {}
                                        merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                        for copied_id in node_mapxx:
                                            self.__before_restored_node_ids.add(copied_id)
                            
                            for copied_id in node_mapx:
                                restored_node_ids.add(copied_id)
                    if len(g_child_followed_trees_2) > len(g_child_followed_trees_1):
                        len_diff = len(g_child_followed_trees_2) - len(g_child_followed_trees_1)
                        node_mapx = {}
                        for j in range(len(g_child_followed_trees_1)):
                            merged_tree = mergeTreePattern(g_child_followed_trees_1[j], g_child_followed_trees_2[j], holes_dict)
                            node_mapx = {}
                            merged_tree.copySubtree(merged_tree.getRootNode(), g2, g2_root_node.getIdInAst(), len(g2_root_node.children), node_mapx)
                            
                            if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                                for empty_parent in after_empty_parents:
                                    if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                        node_mapxx = {}
                                        merged_tree.copySubtree(merged_tree.getRootNode(), after_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                        for copied_id in node_mapxx:
                                            after_restored_node_ids.add(copied_id)
                            if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                                for empty_parent in before_empty_parents:
                                    if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                        node_mapxx = {}
                                        merged_tree.copySubtree(merged_tree.getRootNode(), self.__before_g2, empty_parent.getIdInAst(), len(empty_parent.children), node_mapxx)
                                        for copied_id in node_mapxx:
                                            self.__before_restored_node_ids.add(copied_id)
                            for copied_id in node_mapx:
                                restored_node_ids.add(copied_id)
                    
                    flag = False
                    for holes in holes_dict_multi:
                        if g_cut_followed_trees_1 == holes_dict_multi[holes][0] and g_cut_followed_trees_2 == holes_dict_multi[holes][1]:
                            hole_id = g2.addSingleNode(AST.GumTreeAstType('?'), holes, g2_root_node.getIdInAst(), len(g2_root_node.children))
                            restored_node_ids.add(hole_id)
                            flag = True
                            break
                    if not flag:
                        hole_id = g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), g2_root_node.getIdInAst(), len(g2_root_node.children))
                        
                        if is_before_part == True and g_root_node.getIdInAst() in g_node_map and g_node_map[g_root_node.getIdInAst()] in g_stripped_after_parents_ids:
                            for empty_parent in after_empty_parents:
                                if after_g2_node_map[empty_parent.getIdInAst()] == g_node_map[g_root_node.getIdInAst()]:
                                    hole_idx = after_g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), empty_parent.getIdInAst(), len(empty_parent.children))
                                    after_restored_node_ids.add(hole_idx)
                        if is_before_part == False and len(self.__all_stripped_before_g_root_ids)!=0and g_root_node.getIdInAst() in g_reversed_node_map and g_reversed_node_map[g_root_node.getIdInAst()] in self.__all_stripped_before_g_root_ids:
                            for empty_parent in before_empty_parents:
                                if self.__before_g2_node_map[empty_parent.getIdInAst()] == g_reversed_node_map[g_root_node.getIdInAst()]:
                                    node_mapxx = {}
                                    hole_idx = self.__before_g2.addSingleNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict_multi)+1000), empty_parent.getIdInAst(), len(empty_parent.children))
                                    self.__before_restored_node_ids.add(hole_idx)
                        #if str(len(holes_dict_multi)+1000) == "1002":
                            #import pdb
                            #pdb.set_trace()
                        holes_dict_multi["!@#$holes"+str(len(holes_dict_multi)+1000)] = [g_cut_followed_trees_1, g_cut_followed_trees_2]
                        restored_node_ids.add(hole_id)

                    
    def matchAst(self, ast, hole_dict = {}):
        before_tree_pattern = self.__before_tree_pattern
        ast_root_node = ast.getRootNode()
        ast_node_dict = ast.getNodeDict()
        #hole_dict = {}
        self.__tmp_ast = ast
        node_ids = []
        for node_id in ast_node_dict:
            if self.__matchAstHelper(ast_node_dict[node_id], before_tree_pattern.getRootNode(), hole_dict):
                node_ids.append(node_id)
        return node_ids
        #return None

    def __matchAstHelper(self, ast_root_node, pattern_root_node, hole_dict):
        if not "!@#$hole" in pattern_root_node.value:
            if ast_root_node.value == pattern_root_node.value and ast_root_node.type == pattern_root_node.type:
                if self.__childrenMatchHelper(ast_root_node.children, pattern_root_node.children, hole_dict):
                    return True
            else:
                return False
        else:
            if "!@#$holes" in pattern_root_node.value:
                return True
            else:
                if pattern_root_node.type.type_name == '?':
                    return True
                elif pattern_root_node.type == ast_root_node.type:
                    return True
                else:
                    return False
    def __childrenMatchHelper(self, ast_children, pattern_children, hole_dict):
        if len(pattern_children) == 0:
            if len(ast_children) == 0:
                return True
            else:
                return False
        
        if "!@#$holes" in pattern_children[0].value:
            ast_ch_idx = 0
            flag = True
            for i in range(len(pattern_children)):
                if not "!@#$holes" in pattern_children[i].value:
                    flag = False
                    break
            if flag:
                for i in range(len(pattern_children)):
                    hole_dict[pattern_children[i].value] = []
                    if i==0:
                        for child in ast_children:
                            hole_dict[pattern_children[0].value].append(self.__tmp_ast.getSubtree(child.getIdInAst()))
                return True
            while ast_ch_idx < len(ast_children):
                if self.__childrenMatchHelper(ast_children[ast_ch_idx:], pattern_children[1:], hole_dict):
                    hole_dict[pattern_children[0].value] = []
                    for idx in range(ast_ch_idx):
                        hole_dict[pattern_children[0].value].append(self.__tmp_ast.getSubtree(ast_children[idx].getIdInAst()))
                    return True
                else:
                    ast_ch_idx += 1
            return False
        elif "!@#$hole" in pattern_children[0].value:
            if len(ast_children) == 0:
                return False
            if pattern_children[0].type.type_name == "?":
                #if len(pattern_children) <= 1 and len(ast_children) <= 1:
                #    hole_dict[pattern_children[0].value] = [ast_children[0].getIdInAst()]
                #    return True
                #elif len(pattern_children) <= 1 and not len(ast_children) <= 1:
                #    return False
                #if pattern_children[0].value == '!@#$hole14':
                if self.__childrenMatchHelper(ast_children[1:], pattern_children[1:], hole_dict):
                    hole_dict[pattern_children[0].value] = [self.__tmp_ast.getSubtree(ast_children[0].getIdInAst())]
                    return True
                else:
                    return False
            else:
                if pattern_children[0].type == ast_children[0].type:
                    #if len(pattern_children) <= 1 and len(ast_children) <= 1:
                    #    hole_dict[pattern_children[0].value] = [ast_children[0].getIdInAst()]
                    #    return True
                    #elif len(pattern_children) <= 1 and not len(ast_children) <= 1:
                    #    return False
                    if self.__childrenMatchHelper(ast_children[1:], pattern_children[1:], hole_dict):
                        hole_dict[pattern_children[0].value] = [self.__tmp_ast.getSubtree(ast_children[0].getIdInAst())]
                        return True
                    else:
                        return False
                else:
                    return False
        else:
            if len(ast_children) == 0:
                return False
            if self.__matchAstHelper(ast_children[0], pattern_children[0], hole_dict):
                if self.__childrenMatchHelper(ast_children[1:], pattern_children[1:], hole_dict):
                    return True
                else:
                    return False
            else:
                return False
    
    def applyPattern(self, ast, root_id = -1):
        hole_dict = {}
        matched_root_ids = self.matchAst(ast, hole_dict)

        if len(matched_root_ids) == 0:
            return []
        new_asts = []
        for matched_root_id in matched_root_ids:
            if root_id > 0 and root_id != matched_root_id:
                continue
            ast_root_node = ast.getRootNode()
            ast_node_dict = ast.getNodeDict()
            if self.__has_unbound_holes:
                return []
            new_ast = AST.GumTreeAst([])
            if matched_root_id == 0:
                after_root_node = self.__after_tree_pattern.getRootNode()
                if after_root_node == None:
                    new_asts.append(None)
                    continue
                if "!@#$hole" in after_root_node.value:
                    subtrees_to_be_added = hole_dict[after_root_node.value]
                    #for subtree in subtrees_to_be_added:
                    new_ast = subtrees_to_be_added[0].getSubtree(0)
                else:
                    new_ast.addRootNode(after_root_node.type, after_root_node.value)
                    for i, child in enumerate(after_root_node.children):
                        self.__applyPatternHelper(child, new_ast, 0, i, hole_dict, matched_root_id, True)

            else:
                new_ast.addRootNode(ast_root_node.type, ast_root_node.value)
                for i, child in enumerate(ast_root_node.children):
                    self.__applyPatternHelper(child, new_ast, 0, i, hole_dict, matched_root_id, False)
            new_asts.append(new_ast)
        return new_asts

        
    def __applyPatternHelper(self, src_subtree_root_node, tgt_tree, tgt_subtree_parent_node_id, tgt_subtree_children_list_location, hole_dict, matched_root_id, is_after_pattern):
        if not is_after_pattern:
            if matched_root_id == src_subtree_root_node.getIdInAst():
                after_root_node = self.__after_tree_pattern.getRootNode()
                if after_root_node == None:
                    return
                if "!@#$hole" in after_root_node.value:
                    subtrees_to_be_added = hole_dict[after_root_node.value]
                    subtrees_to_be_added[0].copySubtree(0, tgt_tree, tgt_subtree_parent_node_id, len(tgt_tree.getNodeDict()[tgt_subtree_parent_node_id].children))
                    #new_ast = subtree.getSubtree(0)
                else:
                    new_ast_node_id = tgt_tree.addSingleNode(after_root_node.type, after_root_node.value, tgt_subtree_parent_node_id, len(tgt_tree.getNodeDict()[tgt_subtree_parent_node_id].children))
                    for i, child in enumerate(after_root_node.children):
                        self.__applyPatternHelper(child, tgt_tree, new_ast_node_id, i, hole_dict, matched_root_id, True)
            else:
                new_node_id = tgt_tree.addSingleNode(src_subtree_root_node.type, src_subtree_root_node.value, tgt_subtree_parent_node_id, tgt_subtree_children_list_location)
                for i, child in enumerate(src_subtree_root_node.children):
                    self.__applyPatternHelper(child, tgt_tree, new_node_id, i, hole_dict, matched_root_id, False)
        else:
            after_subtree_root_node = src_subtree_root_node
            if "!@#$hole" in after_subtree_root_node.value:
                subtrees_to_be_added = hole_dict[after_subtree_root_node.value]
                for subtree in subtrees_to_be_added:
                    subtree.copySubtree(subtree.getRootNode(), tgt_tree, tgt_subtree_parent_node_id, len(tgt_tree.getNodeDict()[tgt_subtree_parent_node_id].children))
            else:
                new_node_id = tgt_tree.addSingleNode(after_subtree_root_node.type, after_subtree_root_node.value, tgt_subtree_parent_node_id, len(tgt_tree.getNodeDict()[tgt_subtree_parent_node_id].children))
                for i, child in enumerate(after_subtree_root_node.children):
                    self.__applyPatternHelper(child, tgt_tree, new_node_id, i, hole_dict, matched_root_id, True)
        



                
            
        


def getEditPatterns(ast_pair):
    edit_patterns = []
    
    # limited_before_node_ids = set()
    # limited_after_node_ids = set()
    
    holed_subtree_ids = set()

    node_map = ast_pair.getNodeMap()
    reversed_node_map = ast_pair.getReversedNodeMap()
    inserted_trees = ast_pair.getInsertedTrees()
    deleted_trees = ast_pair.getDeletedTrees()
    updated_nodes = ast_pair.getUpdatedNodes()
    modified_trees = ast_pair.getModifiedBeforeTrees()
    
    before_ast_ids = ast_pair.getBeforeAst().getNodeDict()
    after_ast_ids = ast_pair.getAfterAst().getNodeDict()
    
    # holed subtrees checking
    for before_ast_id in before_ast_ids:
        if not before_ast_id in node_map:
            continue
        flag = False
        cur_node = ast_pair.getBeforeAst().getNodeDict()[before_ast_id]
        while cur_node != None:
            cur_node_id = cur_node.getIdInAst()
            if cur_node_id in holed_subtree_ids:
                flag = True
                break
            cur_node = cur_node.parent
        if flag:
            continue

        cur_node = ast_pair.getBeforeAst().getNodeDict()[before_ast_id]
        extract_holed_subtree = False
        for i, child in enumerate(cur_node.children):
            #if '79,1217' in cur_node.location:
            #    import pdb
            #    pdb.set_trace()
            if child.getIdInAst() in deleted_trees: #or child.getIdInAst() in updated_nodes:
                #if '79,1217' in cur_node.location:
                    #import pdb
                    #pdb.set_trace()
                if i < len(cur_node.children) - 1 and cur_node.children[i+1].getIdInAst() in modified_trees: #and not cur_node.children[i+1].getIdInAst() in deleted_trees:
                    holed_subtree_ids.add(before_ast_id)
                    extract_holed_subtree = True
                if i > 0 and cur_node.children[i-1].getIdInAst() in modified_trees: #and not cur_node.children[i-1].getIdInAst() in deleted_trees:
                    holed_subtree_ids.add(before_ast_id)
                    extract_holed_subtree = True
        after_ast_id = node_map[before_ast_id]
        cur_node = ast_pair.getAfterAst().getNodeDict()[after_ast_id]
        for i, child in enumerate(cur_node.children):
            if child.getIdInAst() in inserted_trees:
                if i < len(cur_node.children) - 1 and cur_node.children[i+1].getIdInAst() in reversed_node_map and reversed_node_map[cur_node.children[i+1].getIdInAst()] in modified_trees: #and not cur_node.children[i+1].getIdInAst() in inserted_trees:
                    holed_subtree_ids.add(before_ast_id)
                    extract_holed_subtree = True
                if i > 0 and cur_node.children[i-1].getIdInAst() in reversed_node_map and reversed_node_map[cur_node.children[i-1].getIdInAst()] in modified_trees: #and not cur_node.children[i-1].getIdInAst() in inserted_trees:
                    holed_subtree_ids.add(before_ast_id)
                    extract_holed_subtree = True

        if extract_holed_subtree:
            before_node_map = {}
            after_node_map = {}
            reversed_before_node_map = {}
            reversed_after_node_map = {}

            node_map_for_pattern = {}
            reversed_node_map_for_pattern = {}
            inserted_trees_for_pattern = set()
            deleted_trees_for_pattern = set()
            updated_nodes_for_pattern = set()
            modified_trees_for_pattern = set()

            cur_node = ast_pair.getBeforeAst().getNodeDict()[before_ast_id]
            before_tree_pattern = AST.GumTreeAst([])
            before_tree_pattern.addRootNode(cur_node.type,cur_node.value)
            before_node_map[0] = cur_node.getIdInAst()

            holes_id = 0
            hole_tree = AST.GumTreeAst([])
            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
            holes_id += 1
            hole_tree.copySubtree(hole_tree.getRootNode(), before_tree_pattern, 0, len(before_tree_pattern.getRootNode().children))

            holed = True
            for i, child in enumerate(cur_node.children):
                if not child.getIdInAst() in modified_trees and holed == False:
                    hole_tree = AST.GumTreeAst([])
                    hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                    holes_id += 1
                    hole_tree.copySubtree(hole_tree.getRootNode(), before_tree_pattern, 0, len(before_tree_pattern.getRootNode().children))
                    holed = True
                if child.getIdInAst() in modified_trees:
                    ast_pair.getBeforeAst().copySubtree(child, before_tree_pattern, 0, len(before_tree_pattern.getRootNode().children), before_node_map)
                    holed = False
            if holed == False:
                hole_tree = AST.GumTreeAst([])
                hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                holes_id += 1
                hole_tree.copySubtree(hole_tree.getRootNode(), before_tree_pattern, 0, len(before_tree_pattern.getRootNode().children))
            before_holes_num = holes_id

            cur_node = ast_pair.getAfterAst().getNodeDict()[after_ast_id]
            after_tree_pattern = AST.GumTreeAst([])
            after_tree_pattern.addRootNode(cur_node.type,cur_node.value)
            after_node_map[0] = cur_node.getIdInAst()
            before_holes_id = holes_id
            holes_id = 0
            hole_tree = AST.GumTreeAst([])
            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
            holes_id += 1
            hole_tree.copySubtree(hole_tree.getRootNode(), after_tree_pattern, 0, len(after_tree_pattern.getRootNode().children))
            holed = True
            nothing_between = True
            for i, child in enumerate(cur_node.children):
                if child.getIdInAst() in reversed_node_map and not reversed_node_map[child.getIdInAst()] in modified_trees and holed == False:
                    hole_tree = AST.GumTreeAst([])
                    hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                    holes_id += 1
                    hole_tree.copySubtree(hole_tree.getRootNode(), after_tree_pattern, 0, len(after_tree_pattern.getRootNode().children))
                    holed = True
                if not child.getIdInAst() in reversed_node_map or reversed_node_map[child.getIdInAst()] in modified_trees:
                    ast_pair.getAfterAst().copySubtree(child, after_tree_pattern, 0, len(after_tree_pattern.getRootNode().children), after_node_map)
                    holed = False
                    nothing_between = False
            #if holed == False or nothing_between == True:
            for i in range(holes_id, before_holes_id):
                hole_tree = AST.GumTreeAst([])
                hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(i))
                #holes_id += 1
                hole_tree.copySubtree(hole_tree.getRootNode(), after_tree_pattern, 0, len(after_tree_pattern.getRootNode().children))
            source_pairs = [ast_pair.idx]

            for before_node_id in before_node_map:
                reversed_before_node_map[before_node_map[before_node_id]] = before_node_id
            for after_node_id in after_node_map:
                reversed_after_node_map[after_node_map[after_node_id]] = after_node_id

            # build node_map_for_pattern
            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                try:
                    original_after_node_id = node_map[original_before_node_id]
                    after_node_id = reversed_after_node_map[original_after_node_id]
                    node_map_for_pattern[before_node_id] = after_node_id
                    reversed_node_map_for_pattern[after_node_id] = before_node_id
                except:
                    pass
            
            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                if original_before_node_id in deleted_trees:
                    deleted_trees_for_pattern.add(before_node_id)
                if original_before_node_id in updated_nodes:
                    updated_nodes_for_pattern.add(before_node_id)
                if original_before_node_id in modified_trees:
                    modified_trees_for_pattern.add(before_node_id)

            for after_node_id in after_node_map:
                original_after_node_id = after_node_map[after_node_id]
                if original_after_node_id in inserted_trees:
                    inserted_trees_for_pattern.add(after_node_id)


            edit_patterns.append(EditPattern(before_tree_pattern, after_tree_pattern, source_pairs, node_map_for_pattern, reversed_node_map_for_pattern, inserted_trees_for_pattern, deleted_trees_for_pattern, updated_nodes_for_pattern, modified_trees_for_pattern)) 
            after_holes_num = holes_id

            flag = False
            if holes_id > 2:
                flag = True

            i = 0
            sub_pattern_id = 0

            cur_node = before_tree_pattern.getRootNode()
            while flag and i < len(before_tree_pattern.getRootNode().children) - 1:
                if before_holes_num != after_holes_num:
                    break
                child = before_tree_pattern.getRootNode().children[i]
                if '!@#$holes' in child.value:
                    before_node_mapx = {}
                    after_node_mapx = {}
                    reversed_before_node_mapx = {}
                    reversed_after_node_mapx = {}

                    node_map_for_patternx = {}
                    reversed_node_map_for_patternx = {}
                    inserted_trees_for_patternx = set()
                    deleted_trees_for_patternx = set()
                    updated_nodes_for_patternx = set()
                    modified_trees_for_patternx = set()

                    before_tree_patternx = None
                    after_tree_patternx = None
                    for j in range(i+1,len(before_tree_pattern.getRootNode().children)):
                        if '!@#$holes' in before_tree_pattern.getRootNode().children[j].value:
                            before_tree_patternx = AST.GumTreeAst([])
                            before_tree_patternx.addRootNode(cur_node.type,cur_node.value)
                            before_node_mapx[0] = 0

                            holes_id = 0
                            hole_tree = AST.GumTreeAst([])
                            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                            holes_id += 1
                            hole_tree.copySubtree(hole_tree.getRootNode(), before_tree_patternx, 0, len(before_tree_patternx.getRootNode().children))
                            for childx in before_tree_pattern.getRootNode().children[i+1:j]:
                                before_tree_pattern.copySubtree(childx, before_tree_patternx, 0, len(before_tree_patternx.getRootNode().children), before_node_mapx)
                            hole_tree = AST.GumTreeAst([])
                            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                            holes_id += 1
                            hole_tree.copySubtree(hole_tree.getRootNode(), before_tree_patternx, 0, len(before_tree_patternx.getRootNode().children))
                            
                            after_tree_patternx = AST.GumTreeAst([])
                            after_tree_patternx.addRootNode(cur_node.type,cur_node.value)
                            after_node_mapx[0] = 0

                            holes_id = 0
                            hole_tree = AST.GumTreeAst([])
                            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                            holes_id += 1
                            hole_tree.copySubtree(hole_tree.getRootNode(), after_tree_patternx, 0, len(after_tree_patternx.getRootNode().children))
                            sub_pattern_id_after = 0
                            m = 0
                            while m < len(after_tree_pattern.getRootNode().children):
                                flag = False
                                childx = after_tree_pattern.getRootNode().children[m]
                                if '!@#$holes' in after_tree_pattern.getRootNode().children[m].value:
                                    if sub_pattern_id_after == sub_pattern_id:
                                        for childx in after_tree_pattern.getRootNode().children[m+1:]:
                                            if '!@#$holes' in childx.value:
                                                flag = True
                                                break
                                            after_tree_pattern.copySubtree(childx, after_tree_patternx, 0, len(after_tree_patternx.getRootNode().children), after_node_mapx)
                                    else:
                                        sub_pattern_id_after += 1
                                if flag:
                                    break
                                m += 1
                            hole_tree = AST.GumTreeAst([])
                            hole_tree.addRootNode(AST.GumTreeAstType('?'), '!@#$holes'+str(holes_id))
                            holes_id += 1
                            hole_tree.copySubtree(hole_tree.getRootNode(), after_tree_patternx, 0, len(after_tree_patternx.getRootNode().children))
                            
                            sub_pattern_id += 1
                            break
                    if before_tree_patternx != None and after_tree_patternx != None:
                        source_pairs = [ast_pair.idx]
                        for before_node_idx in before_node_mapx:
                            reversed_before_node_mapx[before_node_mapx[before_node_idx]] = before_node_idx
                        for after_node_idx in after_node_mapx:
                            reversed_after_node_mapx[after_node_mapx[after_node_idx]] = after_node_idx

                        for before_node_idx in before_node_mapx:
                            original_before_node_idx = before_node_mapx[before_node_idx]
                            try:
                                original_after_node_idx = node_map_for_pattern[original_before_node_idx]
                                after_node_idx = reversed_after_node_mapx[original_after_node_idx]
                                node_map_for_patternx[before_node_idx] = after_node_idx
                                reversed_node_map_for_patternx[after_node_idx] = before_node_idx
                            except:
                                pass

                        for before_node_idx in before_node_mapx:
                            original_before_node_idx = before_node_mapx[before_node_idx]
                            if original_before_node_idx in deleted_trees_for_pattern:
                                deleted_trees_for_patternx.add(before_node_idx)
                            if original_before_node_idx in updated_nodes_for_pattern:
                                updated_nodes_for_patternx.add(before_node_idx)
                            if original_before_node_idx in modified_trees_for_pattern:
                                modified_trees_for_patternx.add(before_node_idx)

                        for after_node_idx in after_node_mapx:
                            original_after_node_idx = after_node_mapx[after_node_idx]
                            if original_after_node_idx in inserted_trees_for_pattern:
                                inserted_trees_for_patternx.add(after_node_idx)

                        edit_patterns.append(EditPattern(before_tree_patternx, after_tree_patternx, source_pairs, node_map_for_patternx, reversed_node_map_for_patternx, inserted_trees_for_patternx, deleted_trees_for_patternx, updated_nodes_for_patternx, modified_trees_for_patternx))       
                i += 1

    for before_ast_id in before_ast_ids:
        flag = False
        cur_node = ast_pair.getBeforeAst().getNodeDict()[before_ast_id].parent
        while cur_node != None:
            cur_node_id = cur_node.getIdInAst()
            if cur_node_id in holed_subtree_ids:
                flag = True
                break
            cur_node = cur_node.parent
        if flag:
            continue

        if before_ast_id in deleted_trees:  
            def checkMap(node):
                if node.getIdInAst() in node_map:
                    return False
                for child in node.children:
                    if checkMap(child) == False:
                        return False
                return True
            if not checkMap(ast_pair.getBeforeAst().getNodeDict()[before_ast_id]):
                continue
            before_node_map = {}
            after_node_map = {}
            reversed_before_node_map = {}
            reversed_after_node_map = {}

            node_map_for_pattern = {}
            reversed_node_map_for_pattern = {}
            inserted_trees_for_pattern = set()
            deleted_trees_for_pattern = set()
            updated_nodes_for_pattern = set()
            modified_trees_for_pattern = set()

            before_tree_pattern = ast_pair.getBeforeAst().getSubtree(before_ast_id, before_node_map)
            after_tree_pattern = AST.GumTreeAst([])
            source_pairs = [ast_pair.idx]

            for before_node_id in before_node_map:
                reversed_before_node_map[before_node_map[before_node_id]] = before_node_id
            for after_node_id in after_node_map:
                reversed_after_node_map[after_node_map[after_node_id]] = after_node_id

            # build node_map_for_pattern
            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                try:
                    original_after_node_id = node_map[original_before_node_id]
                    after_node_id = reversed_after_node_map[original_after_node_id]
                    node_map_for_pattern[before_node_id] = after_node_id
                    reversed_node_map_for_pattern[after_node_id] = before_node_id
                except:
                    pass

            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                if original_before_node_id in deleted_trees:
                    deleted_trees_for_pattern.add(before_node_id)
                if original_before_node_id in updated_nodes:
                    updated_nodes_for_pattern.add(before_node_id)
                if original_before_node_id in modified_trees:
                    modified_trees_for_pattern.add(before_node_id)

            for after_node_id in after_node_map:
                original_after_node_id = after_node_map[after_node_id]
                if original_after_node_id in inserted_trees:
                    inserted_trees_for_pattern.add(after_node_id)

            edit_patterns.append(EditPattern(before_tree_pattern, after_tree_pattern, source_pairs, node_map_for_pattern, reversed_node_map_for_pattern, inserted_trees_for_pattern, deleted_trees_for_pattern, updated_nodes_for_pattern, modified_trees_for_pattern))
        
        elif before_ast_id in updated_nodes:
            before_node_map = {}
            after_node_map = {}
            reversed_before_node_map = {}
            reversed_after_node_map = {}

            node_map_for_pattern = {}
            reversed_node_map_for_pattern = {}
            inserted_trees_for_pattern = set()
            deleted_trees_for_pattern = set()
            updated_nodes_for_pattern = set()
            modified_trees_for_pattern = set()
            
            try:
                if ast_pair.getBeforeAst().getNodeDict()[before_ast_id].parent.type != ast_pair.getAfterAst().getNodeDict()[node_map[before_ast_id]].parent.type:
                    continue
                if ast_pair.getBeforeAst().getNodeDict()[before_ast_id].parent.parent.type != ast_pair.getAfterAst().getNodeDict()[node_map[before_ast_id]].parent.parent.type:
                    continue
            except:
                pass


            before_tree_pattern = ast_pair.getBeforeAst().getSubtree(before_ast_id, before_node_map)
            after_ast_id = node_map[before_ast_id]
            after_tree_pattern = ast_pair.getAfterAst().getSubtree(after_ast_id, after_node_map)
            source_pairs = [ast_pair.idx]

            for before_node_id in before_node_map:
                reversed_before_node_map[before_node_map[before_node_id]] = before_node_id
            for after_node_id in after_node_map:
                reversed_after_node_map[after_node_map[after_node_id]] = after_node_id

            # build node_map_for_pattern
            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                try:
                    original_after_node_id = node_map[original_before_node_id]
                    after_node_id = reversed_after_node_map[original_after_node_id]
                    node_map_for_pattern[before_node_id] = after_node_id
                    reversed_node_map_for_pattern[after_node_id] = before_node_id
                except:
                    pass

            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                if original_before_node_id in deleted_trees:
                    deleted_trees_for_pattern.add(before_node_id)
                if original_before_node_id in updated_nodes:
                    updated_nodes_for_pattern.add(before_node_id)
                if original_before_node_id in modified_trees:
                    modified_trees_for_pattern.add(before_node_id)

            for after_node_id in after_node_map:
                original_after_node_id = after_node_map[after_node_id]
                if original_after_node_id in inserted_trees:
                    inserted_trees_for_pattern.add(after_node_id)

            edit_patterns.append(EditPattern(before_tree_pattern, after_tree_pattern, source_pairs, node_map_for_pattern, reversed_node_map_for_pattern, inserted_trees_for_pattern, deleted_trees_for_pattern, updated_nodes_for_pattern, modified_trees_for_pattern))

        elif before_ast_id in modified_trees:
            before_node_map = {}
            after_node_map = {}
            reversed_before_node_map = {}
            reversed_after_node_map = {}

            node_map_for_pattern = {}
            reversed_node_map_for_pattern = {}
            inserted_trees_for_pattern = set()
            deleted_trees_for_pattern = set()
            updated_nodes_for_pattern = set()
            modified_trees_for_pattern = set()

            try:
                if ast_pair.getBeforeAst().getNodeDict()[before_ast_id].parent.type != ast_pair.getAfterAst().getNodeDict()[node_map[before_ast_id]].parent.type:
                    continue
                if ast_pair.getBeforeAst().getNodeDict()[before_ast_id].parent.parent.type != ast_pair.getAfterAst().getNodeDict()[node_map[before_ast_id]].parent.parent.type:
                    continue
            except:
                pass
            before_tree_pattern = ast_pair.getBeforeAst().getSubtree(before_ast_id, before_node_map)
            if not before_ast_id in node_map:
                continue
            after_ast_id = node_map[before_ast_id]
            after_tree_pattern = ast_pair.getAfterAst().getSubtree(after_ast_id, after_node_map)
            source_pairs = [ast_pair.idx]

            for before_node_id in before_node_map:
                reversed_before_node_map[before_node_map[before_node_id]] = before_node_id
            for after_node_id in after_node_map:
                reversed_after_node_map[after_node_map[after_node_id]] = after_node_id

            # build node_map_for_pattern
            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                try:
                    original_after_node_id = node_map[original_before_node_id]
                    after_node_id = reversed_after_node_map[original_after_node_id]
                    node_map_for_pattern[before_node_id] = after_node_id
                    reversed_node_map_for_pattern[after_node_id] = before_node_id
                except:
                    pass

            for before_node_id in before_node_map:
                original_before_node_id = before_node_map[before_node_id]
                if original_before_node_id in deleted_trees:
                    deleted_trees_for_pattern.add(before_node_id)
                if original_before_node_id in updated_nodes:
                    updated_nodes_for_pattern.add(before_node_id)
                if original_before_node_id in modified_trees:
                    modified_trees_for_pattern.add(before_node_id)

            for after_node_id in after_node_map:
                original_after_node_id = after_node_map[after_node_id]
                if original_after_node_id in inserted_trees:
                    inserted_trees_for_pattern.add(after_node_id)

            edit_patterns.append(EditPattern(before_tree_pattern, after_tree_pattern, source_pairs, node_map_for_pattern, reversed_node_map_for_pattern, inserted_trees_for_pattern, deleted_trees_for_pattern, updated_nodes_for_pattern, modified_trees_for_pattern))
    print("patterns extracted:", len(edit_patterns))
    #import pdb
    #pdb.set_trace()
    return edit_patterns

    # for after_ast_id in after_ast_ids:
      #  if after_ast_id in inserted_trees:



def mergeTreePattern(tree_1, tree_2, holes_dict = {}, node_map = {}):          # node_map "10: [[10,11],[12,13]]" means node 10 in the merged tree matches nodes 10,11 in tree_1 and matches nodes 12, 13 in tree_2
    root_node_1 = tree_1.getRootNode()
    root_node_2 = tree_2.getRootNode()
    merged_tree = AST.GumTreeAst([])
    
    if root_node_1 == None or root_node_2 == None:
        merged_tree.addRootNode(AST.GumTreeAstType('?'), "!@#$hole"+str(len(holes_dict)))
        holes_dict["!@#$hole"+str(len(holes_dict))] = [root_node_1, root_node_2]
        return merged_tree

    if not "!@#$hole" in root_node_1.value and not "!@#$hole" in root_node_2.value:
        if root_node_1.type == root_node_2.type and root_node_1.value == root_node_2.value and len(root_node_1.children) == len(root_node_2.children):
            merged_tree.addRootNode(root_node_1.type, root_node_1.value)
            node_map[0] = [[root_node_1.getIdInAst()],[root_node_2.getIdInAst()]]
            for i in range(len(root_node_1.children)):
                merged_subtree_node_map = {}
                tree_1_copy_subtree_node_map = {}
                tree_2_copy_subtree_node_map = {}
                merged_subtree = mergeTreePattern(tree_1.getSubtree(root_node_1.children[i].getIdInAst(), tree_1_copy_subtree_node_map), tree_2.getSubtree(root_node_2.children[i].getIdInAst(), tree_2_copy_subtree_node_map) , holes_dict, merged_subtree_node_map)

                for merged_node_id in merged_subtree_node_map:
                    matched_tree_1_nodes = merged_subtree_node_map[merged_node_id][0]
                    matched_tree_2_nodes = merged_subtree_node_map[merged_node_id][1]
                    for j, matched_tree_1_node in enumerate(matched_tree_1_nodes):
                        merged_subtree_node_map[merged_node_id][0][j] = tree_1_copy_subtree_node_map[matched_tree_1_node]

                    for j, matched_tree_2_node in enumerate(matched_tree_2_nodes):
                        merged_subtree_node_map[merged_node_id][1][j] = tree_2_copy_subtree_node_map[matched_tree_2_node]

                copy_subtree_node_map = {}
                merged_subtree.copySubtree(merged_subtree.getRootNode(), merged_tree, 0, len(merged_tree.getRootNode().children), copy_subtree_node_map)
                
                for new_node_id in copy_subtree_node_map:
                    old_node_id = copy_subtree_node_map[new_node_id]
                    node_map[new_node_id] = merged_subtree_node_map[old_node_id]

        elif root_node_1.type == root_node_2.type and (root_node_1.value != root_node_2.value or len(root_node_1.children) != len(root_node_2.children)):
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(root_node_1.type, holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(root_node_1.type, "!@#$hole"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$hole"+str(len(holes_dict))] = [root_node_1, root_node_2]
        else:
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(AST.GumTreeAstType('?'), holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(AST.GumTreeAstType('?'), "!@#$hole"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$hole"+str(len(holes_dict))] = [root_node_1, root_node_2]
    elif "!@#$holes" in root_node_1.value or "!@#$holes" in root_node_2.value:
        if root_node_1.type == root_node_2.type:
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(root_node_1.type, holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(root_node_1.type, "!@#$holes"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$holes"+str(len(holes_dict))] = [root_node_1, root_node_2]
        else:
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(AST.GumTreeAstType('?'), holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(AST.GumTreeAstType('?'), "!@#$holes"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$holes"+str(len(holes_dict))] = [root_node_1, root_node_2]
    else:
        if root_node_1.type == root_node_2.type:
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(root_node_1.type, holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(root_node_1.type, "!@#$hole"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$hole"+str(len(holes_dict))] = [root_node_1, root_node_2]
        else:
            flag = False
            for holes in holes_dict:
                if root_node_1 == holes_dict[holes][0] and root_node_2 == holes_dict[holes][1]:
                    merged_tree.addRootNode(AST.GumTreeAstType('?'), holes)
                    node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                    flag = True
                    break
            if not flag:
                merged_tree.addRootNode(AST.GumTreeAstType('?'), "!@#$hole"+str(len(holes_dict)))
                node_map[0] = [[root_node_1.getIdInAst()], [root_node_2.getIdInAst()]]
                holes_dict["!@#$hole"+str(len(holes_dict))] = [root_node_1, root_node_2]
    
    return merged_tree


