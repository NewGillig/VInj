import AST
import Pattern
import const
import pickle
import os
import multiprocessing
#from tqdm import tqdm
import time
import signal

def getSemanticMatchString(pattern, train_subtree, test_subtree):
    hole_dict_train = {}
    hole_dict_test = {}
    pattern.matchAst(train_subtree, hole_dict_train)
    pattern.matchAst(test_subtree, hole_dict_test)
    string_list = []
    for hole in hole_dict_train:
        train_subtree_hole_string = ""
        train_hole_subtrees = hole_dict_train[hole]
        for hole_subtree in train_hole_subtrees:
            train_subtree_hole_string = train_subtree_hole_string + hole_subtree.getCode()
        test_subtree_hole_string = ""
        test_hole_subtrees = hole_dict_test[hole]
        for hole_subtree in test_hole_subtrees:
            test_subtree_hole_string = test_subtree_hole_string + hole_subtree.getCode()
        string_list.append([train_subtree_hole_string, test_subtree_hole_string])
    return string_list

def applyGroupSemantic(pattern_src_trees):
    patterns = []
    new_asts = []
    subtree_root_idss = []
    applied_subtree_root_idss = []
    pattern_subtree_semantic_list = []
    for pattern_src_tree in pattern_src_trees:
        pattern_prevalence_score = pattern_src_tree[0]
        pattern = pattern_prevalence_score[0]
        patterns.append(pattern)
        success_train_pairs = pattern_prevalence_score[2]
        success_subtree_root_ids = pattern_prevalence_score[3]
        
        success_subtrees = []
        for i, success_train_pair in enumerate(success_train_pairs):
            success_subtree = success_train_pair.getBeforeAst().getSubtree(success_subtree_root_ids[i])
            success_subtrees.append(success_subtree)
        
        test_subtrees = []
        src_tree = pattern_src_tree[1]
        src_tree_name = pattern_src_tree[2]
        #try:
        subtree_root_ids = pattern.matchAst(src_tree)
        subtree_root_idss.append(subtree_root_ids)
        for root_id in subtree_root_ids:
            test_subtree = src_tree.getSubtree(root_id)
            test_subtrees.append(test_subtree)
        
        subtree_semantic_list = []
        for test_subtree in test_subtrees:
            semantic_list = []
            for success_subtree in success_subtrees:
                semantic_list.append(getSemanticMatchString(pattern, success_subtree, test_subtree))
            subtree_semantic_list.append(semantic_list)
        pattern_subtree_semantic_list.append(subtree_semantic_list)

    f = open('pattern_subtree_semantic_list/'+src_tree_name+'.pkl', 'wb')
    pickle.dump(pattern_subtree_semantic_list, f)
    f.close()
    #import pdb
    #pdb.set_trace()
    os.system('python3 getSimilarity.py '+src_tree_name)
    
    f = open('pattern_subtree_ids/'+src_tree_name+'.pkl', 'rb')
    pattern_subtree_ids = pickle.load(f)
    f.close()

    for i, subtree_ids in enumerate(pattern_subtree_ids):
        new_asts_for_this_pattern = []
        applied_subtree_root_ids_for_this_pattern = []
        for idx in subtree_ids:
            new_asts_for_this_pattern.extend(patterns[i].applyPattern(src_tree, subtree_root_idss[i][idx]))
            applied_subtree_root_ids_for_this_pattern.append(subtree_root_idss[i][idx])
        new_asts.append(new_asts_for_this_pattern)
        applied_subtree_root_idss.append(applied_subtree_root_ids_for_this_pattern)
    #import pdb
    #pdb.set_trace()


    '''
    semantic_match_subtree_ids = p.readlines()
        for line in semantic_match_subtree_ids:
            idx = int(line[:-1])
            patterns.append(pattern)
            new_asts.append(pattern.applyPattern(src_tree, subtree_root_ids[idx]))
            subtree_root_idss.append([subtree_root_ids[idx]])
    '''

        #except:
        #    pass
    return patterns, new_asts, applied_subtree_root_idss

def applyGnnSemantic(pattern_src_trees):
    patterns = []
    new_asts = []
    subtree_root_idss = []
    applied_subtree_root_idss = []
    pattern_subtree_semantic_list = []
    for pattern_src_tree in pattern_src_trees:
        pattern_prevalence_score = pattern_src_tree[0]
        pattern = pattern_prevalence_score[0]
        patterns.append(pattern)

        test_subtrees = []
        src_tree = pattern_src_tree[1]
        src_tree_name = pattern_src_tree[2]
        #try:
        subtree_root_ids = pattern.matchAst(src_tree)
        subtree_root_idss.append(subtree_root_ids)
        for root_id in subtree_root_ids:
            test_subtree = src_tree.getSubtree(root_id)
            test_subtrees.append(test_subtree.getHoppityAst())
        pattern_subtree_semantic_list.append([pattern.getBeforePattern().getHoppityAst(), test_subtrees])

    f = open('./pattern_subtree_gnn_list/'+src_tree_name+'.pkl', 'wb')
    pickle.dump(pattern_subtree_semantic_list, f)
    f.close()

    #os.system('python3 ./hoppity_gnn/getSimilarity.py '+src_tree_name + " > /dev/null 2> /dev/null")
    os.system('python3 ./hoppity_gnn/getLocation.py '+src_tree_name + " > /dev/null 2> /dev/null")


    f = open('pattern_subtree_gnn_ids/'+src_tree_name+'.pkl', 'rb')
    pattern_subtree_ids = pickle.load(f)
    f.close()



    for i, subtree_ids in enumerate(pattern_subtree_ids):
        new_asts_for_this_pattern = []
        applied_subtree_root_ids_for_this_pattern = []
        for idx in subtree_ids:
            new_asts_for_this_pattern.extend(patterns[i].applyPattern(src_tree, subtree_root_idss[i][idx]))
            applied_subtree_root_ids_for_this_pattern.append(subtree_root_idss[i][idx])
        new_asts.append(new_asts_for_this_pattern)
        applied_subtree_root_idss.append(applied_subtree_root_ids_for_this_pattern)

    return patterns, new_asts, applied_subtree_root_idss

def applyGnnLocation2(pattern_src_trees):
    patterns = []
    for pattern_src_tree in pattern_src_trees:
        pattern_prevalence_score = pattern_src_tree[0]
        pattern = pattern_prevalence_score[0]
        patterns.append(pattern)

        src_tree = pattern_src_tree[1]
        src_tree_hoppity = src_tree.getHoppityAst()
        src_tree_name = pattern_src_tree[2]
        
    f = open('./ast_token_list/'+src_tree_name+'.pkl', 'wb')
    pickle.dump(src_tree_hoppity, f)
    f.close()

    os.system('python3 ./hoppity_gnn/getLocation2.py '+src_tree_name + " > /dev/null 2> /dev/null")

    f = open('subtree_gnn_ids/'+src_tree_name+'.pkl', 'rb')
    subtree_id = pickle.load(f)[0]
    f.close()
    
    
    for pattern in patterns:
        new_asts = pattern.applyPattern(src_tree, subtree_id)
        if len(new_asts) > 0:
            return [pattern],[new_asts],[subtree_id]
    return [], [], []
            
    '''
    for i, subtree_ids in enumerate(pattern_subtree_ids):
        new_asts_for_this_pattern = []
        applied_subtree_root_ids_for_this_pattern = []
        for idx in subtree_ids:
            new_asts_for_this_pattern.extend(patterns[i].applyPattern(src_tree, subtree_root_idss[i][idx]))
            applied_subtree_root_ids_for_this_pattern.append(subtree_root_idss[i][idx])
        new_asts.append(new_asts_for_this_pattern)
        applied_subtree_root_idss.append(applied_subtree_root_ids_for_this_pattern)
    '''
    #return patterns, new_asts, applied_subtree_root_idss

def applyT5Location(pattern_src_trees, T5_output):
    patterns = []
    for pattern_src_tree in pattern_src_trees:
        pattern_prevalence_score = pattern_src_tree[0]
        pattern = pattern_prevalence_score[0]
        patterns.append(pattern)

        src_tree = pattern_src_tree[1]
        src_tree_name = pattern_src_tree[2]

    T5_pred = ""
    patternsx = []
    new_astss = []
    subtree_ids = []
    for i in range(len(T5_output['names'])):
        if T5_output['names'][i] == src_tree_name:
            T5_pred = T5_output['raw_predictions'][i]
            #if T5_output['is_correct'][i] == 1:
            #    import pdb
            #    pdb.set_trace()
            T5_pred = T5_pred.replace('<S2SV_blank>', '')
            T5_predx = ""
            for t in T5_pred:
                if t == ',':
                    T5_predx = T5_predx + ' ,'
                elif t == '!':
                    T5_predx = T5_predx + ' !'
                else:
                    T5_predx = T5_predx + t
            T5_predx = T5_predx + " "
            T5_pred = T5_predx
            subtree_id = -1
            for j in range(len(src_tree.getNodeDict())):
                subtree_code = src_tree.getSubtree(j).getCode()
                if len(subtree_code.split(' '))>=3 and (";" in subtree_code or "if (" in subtree_code) and subtree_code in T5_pred:
                    subtree_id = j
                    break
                elif "if (" in subtree_code:
                    subtree_codex = subtree_code.replace("{ ", "")
                    subtree_codex = subtree_codex.replace(" }", "")
                    if len(subtree_codex.split(' '))>=3 and (";" in subtree_codex or "if (" in subtree_codex) and subtree_codex in T5_pred:
                        subtree_id = j
                        break
            for pattern in patterns:
                new_asts = pattern.applyPattern(src_tree, subtree_id)
                if len(new_asts) > 0:
                    patternsx.append(pattern)
                    new_astss.append(new_asts)
                    subtree_ids.append(subtree_id)
                    break
    return patternsx,new_astss,subtree_ids
    #return [], [], []

def applyGnnSemantic2(pattern_src_trees):
    patterns = []
    new_asts = []
    subtree_root_idss = []
    applied_subtree_root_idss = []
    pattern_subtree_semantic_list = []
    for pattern_src_tree in pattern_src_trees:
        pattern_prevalence_score = pattern_src_tree[0]
        pattern = pattern_prevalence_score[0]
        patterns.append(pattern)
        success_train_pairs = pattern_prevalence_score[2]
        success_subtree_root_ids = pattern_prevalence_score[3]

        success_subtrees = []
        for i, success_train_pair in enumerate(success_train_pairs):
            success_subtree = success_train_pair.getBeforeAst().getSubtree(success_subtree_root_ids[i])
            success_subtrees.append(success_subtree.getHoppityAst())

        test_subtrees = []
        src_tree = pattern_src_tree[1]
        src_tree_name = pattern_src_tree[2]
        #try:
        subtree_root_ids = pattern.matchAst(src_tree)
        subtree_root_idss.append(subtree_root_ids)
        for root_id in subtree_root_ids:
            test_subtree = src_tree.getSubtree(root_id)
            test_subtrees.append(test_subtree.getHoppityAst())
        pattern_subtree_semantic_list.append([success_subtrees, test_subtrees])

    f = open('./pattern_subtree_gnn_list/'+src_tree_name+'.pkl', 'wb')
    pickle.dump(pattern_subtree_semantic_list, f)
    f.close()

    os.system('python3 ./hoppity_gnn/getSimilarity.py '+src_tree_name + " > /dev/null 2> /dev/null")

    f = open('pattern_subtree_gnn_ids/'+src_tree_name+'.pkl', 'rb')
    pattern_subtree_ids = pickle.load(f)
    f.close()



    for i, subtree_ids in enumerate(pattern_subtree_ids):
        new_asts_for_this_pattern = []
        applied_subtree_root_ids_for_this_pattern = []
        for idx in subtree_ids:
            new_asts_for_this_pattern.extend(patterns[i].applyPattern(src_tree, subtree_root_idss[i][idx]))
            applied_subtree_root_ids_for_this_pattern.append(subtree_root_idss[i][idx])
        new_asts.append(new_asts_for_this_pattern)
        applied_subtree_root_idss.append(applied_subtree_root_ids_for_this_pattern)

    return patterns, new_asts, applied_subtree_root_idss



def applyGroup(pattern_src_trees):
    patterns = []
    new_asts = []
    subtree_root_ids = []
    for pattern_src_tree in pattern_src_trees:
        pattern = pattern_src_tree[0]
        src_tree = pattern_src_tree[1]
        try:
            patterns.append(pattern)
            new_asts.append(pattern.applyPattern(src_tree))
            subtree_root_ids.append(pattern.matchAst(src_tree))
            #return new_ast, subtree_root_id
        except:
            pass
    return patterns, new_asts, subtree_root_ids
'''
def time_limit(set_time):
    def wraps(func):
        def handler(signum, frame)
            raise
        def deco(*args, **kwargs):
            try:
                signal.signal(signal.SIGALRM, handler)
                signal.alarm(set_time)
                res = func(*args, **kwargs)
                signal.alarm(0)
                return res
            except:
                print("Timeout")
        return deco
    return wraps
'''
def handle_timeout(signum, frame):
    raise TimeoutError("Timeout")


class HierarchicalNode:
    def __init__(self):
        self.parent = None
        self.children = []
        self.pattern = None

class HierarchicalCluster:
    def __init__(self, working_set):
        self.working_set = working_set
        self.working_sets = {}
        #self.cluster_node_sets = {}
        #import random
        #andom.seed(2000)
        #random.shuffle(self.working_set)
        self.hierarchical_nodes = []
        self.__clustering()


    def __clustering(self):
        time = 0
        err_num = 0
        import random
        random.seed(1000)
        random.shuffle(self.working_set)
        for pattern in self.working_set:
            if not pattern.getBeforePattern().getRootNode().type.type_name in self.working_sets:
                self.working_sets[pattern.getBeforePattern().getRootNode().type.type_name] = []
                #self.cluster_node_sets[pattern.getBeforePattern().getRootNode().type.type_name] = []
            self.working_sets[pattern.getBeforePattern().getRootNode().type.type_name].append(pattern)

        for root_type in self.working_sets:
            if root_type == "unit" or root_type == "block":
                continue
            working_set = self.working_sets[root_type]
            working_set.sort(key = self.sizeofPattern, reverse = False)
            if not os.path.exists('./pkls/'+root_type+'.pkl'):
                f = open('./pkls/'+root_type+'.pkl', 'wb')
                pickle.dump(working_set, f)
                f.close()
        ps = []
        for root_type in self.working_sets:
            if root_type == "unit" or root_type == "block":
                continue
            ps.append(os.popen("python3 -u cluster.py "+root_type+" > "+'./outs/'+root_type+".out 2> "+'./outs/'+root_type+".err"))

        for p in ps:
            p.read()


            
    @staticmethod        
    def clusterSubset(root_type):
        f = open('./pkls/'+root_type+'.pkl', 'rb')
        working_set = pickle.load(f)
        f.close()
        iii = 0
        cluster_node_set = []
        #Remove duplicates
        while iii < len(working_set):
            jjj = iii+1
            flag = False
            while jjj < len(working_set):
                if len(working_set[iii]) != len(working_set[jjj]):
                    break
                if working_set[iii] == working_set[jjj]:
                    del working_set[jjj]
                else:
                    jjj += 1
            iii += 1
        stack = []
        iii = 0
        while len(working_set) > 1:
            if len(stack) == 0:
                stack.append(working_set[iii])
            pattern1 = stack[-1]
            gen_set = []
            import time
            before = time.time()
            bound_exist = False
            for i in range(len(working_set)):
                pattern2 = working_set[i]
                if pattern1 == pattern2:
                    continue
                pattern3 = pattern1.mergeEditPattern(pattern2)
                if pattern3.hasUnboundHoles() or pattern3.isAmbiguous():
                    if bound_exist:
                        continue
                else:
                    bound_exist = True
                gen_set.append([pattern1,pattern2,pattern3])
            if len(gen_set) == 0:
                gen_set.append([pattern1,pattern1,pattern1])
            if not bound_exist:
                gen_set = [gen_set[0]]
            after = time.time()
            print(after - before)

            before = time.time()
            gen_set.sort(key = HierarchicalCluster.numOfUnmodifiedMappings, reverse = True)
            gen_set.sort(key = HierarchicalCluster.numOfUnmodifiedMappingsWithLabels, reverse = True)
            gen_set.sort(key = HierarchicalCluster.sizeOfGeneralizedTrees, reverse = False)
            gen_set.sort(key = HierarchicalCluster.numOfHoles, reverse = False)
            gen_set.sort(key = HierarchicalCluster.numOfModifiedMappings, reverse = True)
            if gen_set[0][0].getAfterPattern().getRootNode() == None:
                gen_set.sort(key = HierarchicalCluster.sizeofBeforePattern, reverse = True)
            gen_set.sort(key = HierarchicalCluster.hasUnboundHoles, reverse = False)
            after = time.time()
            print(after - before)
            print(root_type, len(working_set))
            if gen_set[0][1] in stack:
                #import pdb
                #pdb.set_trace()
                e1_pattern = gen_set[0][0]
                try:
                    stack.remove(gen_set[0][0])
                except:
                    pass
                e2_pattern = gen_set[0][1]
                try:
                    stack.remove(gen_set[0][1])
                except:
                    pass
                new_pattern = e1_pattern.mergeEditPattern(e2_pattern)
                e1_node = None
                e2_node = None
                
                for hierarchical_node in cluster_node_set:
                    if hierarchical_node.pattern == e1_pattern:
                        e1_node = hierarchical_node
                        break
                for hierarchical_node in cluster_node_set:
                    if hierarchical_node.pattern == e2_pattern:
                        e2_node = hierarchical_node
                        break
                if e1_node == None:
                    e1_node = HierarchicalNode()
                    e1_node.pattern = e1_pattern
                    cluster_node_set.append(e1_node)
                if e2_node == None:
                    e2_node = HierarchicalNode()
                    e2_node.pattern = e2_pattern
                    cluster_node_set.append(e2_node)

                e3_node = HierarchicalNode()
                e3_node.pattern = new_pattern
                e3_node.children.append(e1_node)
                e3_node.children.append(e2_node)
                e1_node.parent = e3_node
                e2_node.parent = e3_node
                cluster_node_set.append(e3_node)

                working_set.append(new_pattern)
                try:
                    working_set.remove(e1_pattern)
                except:
                    pass
                try:
                    working_set.remove(e2_pattern)
                except:
                    pass
            else:
                stack.append(gen_set[0][1])
        f = open('./clusters/'+root_type + "_cluster.pkl", 'wb')
        pickle.dump(cluster_node_set, f)
        f.close()
    #del self.working_set
    #del self.working_sets

    '''
        while len(self.working_set) > 1:
            pattern1 = self.working_set[0]
            gen_set = []
            for i in range(1,len(self.working_set)):
                pattern2 = self.working_set[i]
                pattern3 = pattern1.mergeEditPattern(pattern2)
                gen_set.append([pattern1,pattern2,pattern3])
            gen_set.sort(key = self.numOfUnmodifiedMappings, reverse = True)
            gen_set.sort(key = self.numOfUnmodifiedMappingsWithLabels, reverse = True)
            gen_set.sort(key = self.sizeOfGeneralizedTrees, reverse = False)
            gen_set.sort(key = self.numOfHoles, reverse = False)
            gen_set.sort(key = self.numOfModifiedMappings, reverse = True)
            if gen_set[0][0].getAfterPattern().getRootNode() == None:
                gen_set.sort(key = self.sizeofBeforePattern, reverse = True)
            gen_set.sort(key = self.hasUnboundHoles, reverse = False)
            new_pattern = gen_set[0][2]
            print('working_set size:',len(self.working_set))
            
            new_asts = gen_set[0][0].applyPattern(gen_set[0][0].getSourcePairs()[0].getBeforeAst())
            after_ast = gen_set[0][0].getSourcePairs()[0].getAfterAst()
            before_ast = gen_set[0][0].getSourcePairs()[0].getBeforeAst()
            subtree_root_ids = gen_set[0][0].matchAst(gen_set[0][0].getSourcePairs()[0].getBeforeAst())
            success = False
            if gen_set[0][0].hasUnboundHoles():
                #print('Unbound Holes')
                success = True
            for i, new_ast in enumerate(new_asts):
                subtree_root_id = subtree_root_ids[i]
                if gen_set[0][0].getAfterPattern().getRootNode() == None and not subtree_root_id in gen_set[0][0].getSourcePairs()[0].getNodeMap():
                    #print('Delete whole tree')
                    success = True
                    break
                else:
                    try:
                        new_ast_subtree = new_ast.getSubtree(subtree_root_id)
                        before_ast_subtree = before_ast.getSubtree(subtree_root_id)
                        after_ast_subtree = after_ast.getSubtree(gen_set[0][0].getSourcePairs()[0].getNodeMap()[subtree_root_id])
                        #print(new_ast_subtree == after_ast_subtree)
                        if new_ast_subtree == after_ast_subtree:
                            success = True
                            break
                    except:
                        continue
            if not success:
                print("failed")
                err_num += 1
                print(err_num)
                #import pdb
                #pdb.set_trace()
                
            time += 1
            #if time == 3:
                #import pdb
                #pdb.set_trace()
                #gen_set[0][0].mergeEditPattern(gen_set[0][1])
            
            discard = False
            for i in range(len(gen_set[0][0].getSourcePairs())):
                matched_id = gen_set[0][2].matchAst(gen_set[0][0].getSourcePairs()[i].getBeforeAst())
                if matched_id != None:
                    print(matched_id)
                else:
                    print("cannot match")
                    discard = True
                    break
                    #import pdb
                    #pdb.set_trace()
                    #gen_set[0][2].matchAst(gen_set[0][0].getSourcePairs()[i].getBeforeAst())
                    #gen_set[0][0].mergeEditPattern(gen_set[0][1])

            for i in range(len(gen_set[0][1].getSourcePairs())):
                matched_id = gen_set[0][2].matchAst(gen_set[0][1].getSourcePairs()[i].getBeforeAst())
                if matched_id != None:
                    print(matched_id)
                else:
                    print("cannot match")
                    discard = True
                    break
                    #import pdb
                    #pdb.set_trace()
                    #gen_set[0][2].matchAst(gen_set[0][1].getSourcePairs()[i].getBeforeAst())
                    #gen_set[0][0].mergeEditPattern(gen_set[0][1])
            #if discard:
            #    self.working_set.remove(gen_set[0][0])
            #    self.working_set.remove(gen_set[0][1])
            #    continue
            #print(gen_set[0][0])
            #print(gen_set[0][1])
            #print(gen_set[0][2])
            #import pdb
            #pdb.set_trace()
            #gen_set[0][0].getSourcePairs()[0].getBeforeAst()
            #gen_set[0][0].mergeEditPattern(gen_set[0][1])
            
            
            self.working_set.sort(key = self.numOfUnmodifiedMappings, reverse = True)
            self.working_set.sort(key = self.numOfUnmodifiedMappingsWithLabels, reverse = True)
            self.working_set.sort(key = self.numOfHoles, reverse = False)
            self.working_set.sort(key = self.numOfModifiedMappings, reverse = True)
            self.working_set.sort(key = self.hasUnboundHoles, reverse = False)
            new_pattern = self.working_set[0].mergeEditPattern(self.working_set[1])
            
            e1_node = None
            e2_node = None
            for hierarchical_node in self.hierarchical_nodes:
                if hierarchical_node.pattern == gen_set[0][0]:
                    e1_node = hierarchical_node
                    break
            for hierarchical_node in self.hierarchical_nodes:
                if hierarchical_node.pattern == gen_set[0][1]:
                    e2_node = hierarchical_node
                    break
            if e1_node == None:
                e1_node = HierarchicalNode()
                e1_node.pattern = gen_set[0][0]
                self.hierarchical_nodes.append(e1_node)
            if e2_node == None:
                e2_node = HierarchicalNode()
                e2_node.pattern = gen_set[0][1]
                self.hierarchical_nodes.append(e2_node)
            
            e3_node = HierarchicalNode()
            e3_node.pattern = new_pattern
            e3_node.children.append(e1_node)
            e3_node.children.append(e2_node)
            e1_node.parent = e3_node
            e2_node.parent = e3_node
            self.hierarchical_nodes.append(e3_node)
            
            self.working_set.append(new_pattern)
            self.working_set.remove(gen_set[0][0])
            self.working_set.remove(gen_set[0][1])
    '''
    @staticmethod
    def sizeofPattern(pattern):
        return len(pattern)

    @staticmethod
    def sizeofBeforePattern(pattern):
        return len(pattern[2])

    @staticmethod
    def hasUnboundHoles(pattern):
        if pattern[2].hasUnboundHoles() or pattern[2].isAmbiguous():
            return 1
        else:
            return 0

    @staticmethod
    def numOfModifiedMappings(pattern):
        num = 0
        for node_id in pattern[2].getNodeMap():
            if node_id in pattern[2].getModifiedTrees():
                num += 1
        return num

    @staticmethod
    def numOfHoles(pattern):
        return len(pattern[2].getBeforeHolesSet().union(pattern[2].getAfterHolesSet()))
    @staticmethod
    def sizeOfGeneralizedTrees(pattern):
        return (len(pattern[0]) + len(pattern[1])) / 2 - len(pattern[2])

    @staticmethod
    def numOfUnmodifiedMappingsWithLabels(pattern):
        num = 0
        for node_id in pattern[1].getNodeMap():
            if not node_id in pattern[1].getModifiedTrees() and pattern[1].getBeforePattern().getNodeDict()[node_id].type.type_name != '?':
                num += 1
        return num
    @staticmethod
    def numOfUnmodifiedMappings(pattern):
        num = 0
        for node_id in pattern[1].getNodeMap():
            if not node_id in pattern[1].getModifiedTrees():
                num += 1
        return num

    def getGnnTraining3(self):
        try:
            if self.proned:
                pass
            else:
                raise
        except:
            if os.path.exists('prevalence.pkl'):
                f = open('prevalence.pkl', 'rb')
                obj = pickle.load(f)
                patterns = obj[0]
                self.proned_patterns = patterns
                self.pattern_prevalence_scores = obj[1]
                f.close()
            else:
                f = open('real_world_train_pairs.pkl', 'rb')
                train_pairs = pickle.load(f)
                f.close()
                patterns = []
                for node in self.hierarchical_nodes: 
                    pattern = node.pattern
                    #if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet())>10 or len(pattern)>100:
                    if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet()) > len(pattern.getBeforePattern())*0.5:
                        continue
                    else:
                        patterns.append(pattern)     
                patterns2 = []
                for i, pattern in enumerate(patterns):
                    flag = False
                    for j in range(i+1, len(patterns)):
                        if pattern == patterns[j]:
                            flag = True
                    if not flag:
                        patterns2.append(pattern)
                patterns = patterns2
                self.pattern_prevalence_scores = []
                for iii, pattern in enumerate(patterns):
                    #if pattern.getBeforePattern().getRootNode().type.type_name == 'if':
                    score = 0
                    success_train_pairs = []
                    success_subtree_root_ids = []
                    for jjj,train_pair in enumerate(train_pairs):
                        #print(jjj)
                        before_ast = train_pair.getBeforeAst()
                        after_ast = train_pair.getAfterAst()
                        new_asts = []
                        subtree_root_ids = []
                        signal.signal(signal.SIGALRM, handle_timeout)
                        signal.alarm(1)
                        try:
                            subtree_root_ids.extend(pattern.matchAst(before_ast))
                            new_asts.extend(pattern.applyPattern(before_ast))
                        except TimeoutError:
                            print('Timeout')
                        except:
                            print('err')
                        finally:
                            signal.alarm(0)
                        for kkk, new_ast in enumerate(new_asts):
                            if new_ast.getCode() == after_ast.getCode():
                                success_subtree_root_ids.append(subtree_root_ids[kkk])
                                success_train_pairs.append(train_pair)
                                score += 1
                                break
                    self.pattern_prevalence_scores.append([pattern, score, success_train_pairs, success_subtree_root_ids])
                    '''
                    if score >= 10:
                        subtree0 = success_train_pairs[0].getBeforeAst().getSubtree(success_subtree_root_ids[0])
                        subtree1 = success_train_pairs[1].getBeforeAst().getSubtree(success_subtree_root_ids[1])
                        string_list = semanticMatch(pattern, subtree0, subtree1)
                    ''' 
                    print(iii)

                self.proned_patterns = patterns
                self.proned = True
                
                patterns = self.proned_patterns
                f = open('prevalence.pkl', 'wb')
                pickle.dump([patterns, self.pattern_prevalence_scores], f)
                f.close()
        


        #print(len(src_tree_patterns))
        self.pattern_prevalence_scores.sort(key = self.getScore2, reverse = True)
        #import pdb
        #pdb.set_trace()
        #patterns.sort(key = self.getHoleNum, reverse = False)
        #patterns.sort(key = self.getPatternSize, reverse = True)
        
        #src_tree_patterns.sort(key = self.getScore, reverse = True)
        new_asts = []
        subtree_root_ids = []
        pattern_src_trees = []
        
        import random
        random.seed(1000)

        f = open('real_world_vulgen_train_pairs.pkl', 'rb')
        train_pairs = pickle.load(f)
        f.close()
        gnn_training = {}
        #gnn_training = []
        #linevul_training = []
        sample_set = set()
        #random.shuffle(self.pattern_prevalence_scores)
        
        '''
        node_type = set()
        for iii, pattern_score in enumerate(self.pattern_prevalence_scores):
            pattern = pattern_score[0]    
            node_type.add(pattern.getBeforePattern().getRootNode().type.type_name)
            if iii >= 50:
                break
        '''
        type_set=set()
        for iii, pattern_score in enumerate(self.pattern_prevalence_scores):
            pattern = pattern_score[0]
            print(iii)
            #if not pattern.getAfterPattern().__str__() == "":
                #continue
            if iii <=0 or iii > 30:
                continue
            type_set.add(pattern.getBeforePattern().getRootNode().type.type_name)
            random.shuffle(train_pairs)
            for jjj,train_pair in enumerate(train_pairs):
                #if train_pair.idx in gnn_training:
                    #continue
                print(jjj)
                before_ast = train_pair.getBeforeAst()
                after_ast = train_pair.getAfterAst()
                new_asts = []
                subtree_root_ids = []
                signal.signal(signal.SIGALRM, handle_timeout)
                signal.alarm(1)
                try:
                    subtree_root_ids.extend(pattern.matchAst(before_ast))
                    new_asts.extend(pattern.applyPattern(before_ast))
                except TimeoutError:
                    print('Timeout')
                except:
                    print('err')
                finally:
                    signal.alarm(0)
                for kkk, new_ast in enumerate(new_asts):
                    if new_ast.getCode() == after_ast.getCode():
                        ground_truth_root_id = subtree_root_ids[kkk]
                        try:
                            #linevul_training.append([before_ast.getCode(), after_ast.getCode()])
                            if not train_pair.idx in gnn_training:
                                gnn_training[train_pair.idx] = [before_ast.getHoppityAst(), [ground_truth_root_id]]
                            else:
                                gnn_training[train_pair.idx][1].append(ground_truth_root_id)
                            #gnn_training.append([before_ast.getHoppityAst(), ground_truth_root_id])
                        except:
                            pass
                        sample_set.add(train_pair)
                    
        
        f = open('gnn_training_localization_vulgen3.0.pkl', 'wb')
        pickle.dump(gnn_training, f)
        f.close()
        

        #f = open('linevul_training_vulgen.pkl', 'wb')
        #pickle.dump(linevul_training, f)
        #f.close()


    def getGnnTraining2(self):
        try:
            if self.proned:
                pass
            else:
                raise
        except:
            if os.path.exists('prevalence.pkl'):
                f = open('prevalence.pkl', 'rb')
                obj = pickle.load(f)
                patterns = obj[0]
                self.proned_patterns = patterns
                self.pattern_prevalence_scores = obj[1]
                f.close()
            else:
                f = open('real_world_train_pairs.pkl', 'rb')
                train_pairs = pickle.load(f)
                f.close()
                patterns = []
                for node in self.hierarchical_nodes: 
                    pattern = node.pattern
                    #if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet())>10 or len(pattern)>100:
                    if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet()) > len(pattern.getBeforePattern())*0.5:
                        continue
                    else:
                        patterns.append(pattern)     
                patterns2 = []
                for i, pattern in enumerate(patterns):
                    flag = False
                    for j in range(i+1, len(patterns)):
                        if pattern == patterns[j]:
                            flag = True
                    if not flag:
                        patterns2.append(pattern)
                patterns = patterns2
                self.pattern_prevalence_scores = []
                for iii, pattern in enumerate(patterns):
                    #if pattern.getBeforePattern().getRootNode().type.type_name == 'if':
                    score = 0
                    success_train_pairs = []
                    success_subtree_root_ids = []
                    for jjj,train_pair in enumerate(train_pairs):
                        #print(jjj)
                        before_ast = train_pair.getBeforeAst()
                        after_ast = train_pair.getAfterAst()
                        new_asts = []
                        subtree_root_ids = []
                        signal.signal(signal.SIGALRM, handle_timeout)
                        signal.alarm(1)
                        try:
                            subtree_root_ids.extend(pattern.matchAst(before_ast))
                            new_asts.extend(pattern.applyPattern(before_ast))
                        except TimeoutError:
                            print('Timeout')
                        except:
                            print('err')
                        finally:
                            signal.alarm(0)
                        for kkk, new_ast in enumerate(new_asts):
                            if new_ast.getCode() == after_ast.getCode():
                                success_subtree_root_ids.append(subtree_root_ids[kkk])
                                success_train_pairs.append(train_pair)
                                score += 1
                                break
                    self.pattern_prevalence_scores.append([pattern, score, success_train_pairs, success_subtree_root_ids])
                    '''
                    if score >= 10:
                        subtree0 = success_train_pairs[0].getBeforeAst().getSubtree(success_subtree_root_ids[0])
                        subtree1 = success_train_pairs[1].getBeforeAst().getSubtree(success_subtree_root_ids[1])
                        string_list = semanticMatch(pattern, subtree0, subtree1)
                    ''' 
                    print(iii)

                self.proned_patterns = patterns
                self.proned = True
                
                patterns = self.proned_patterns
                f = open('prevalence.pkl', 'wb')
                pickle.dump([patterns, self.pattern_prevalence_scores], f)
                f.close()
        


        #print(len(src_tree_patterns))
        self.pattern_prevalence_scores.sort(key = self.getScore2, reverse = True)
        #import pdb
        #pdb.set_trace()
        #patterns.sort(key = self.getHoleNum, reverse = False)
        #patterns.sort(key = self.getPatternSize, reverse = True)
        
        #src_tree_patterns.sort(key = self.getScore, reverse = True)
        new_asts = []
        subtree_root_ids = []
        pattern_src_trees = []
        
        import random
        random.seed(1000)

        f = open('real_world_train_pairs.pkl', 'rb')
        train_pairs = pickle.load(f)
        f.close()
        gnn_training_pairs = []
        #random.shuffle(self.pattern_prevalence_scores)
        for iii, pattern_score in enumerate(self.pattern_prevalence_scores):
            pattern = pattern_score[0]
            print(iii)
            if iii >= 50:
                break
            positives = 0
            negatives = 0
            
            positive_subtrees = []
            negative_subtrees = []

            random.shuffle(train_pairs)
            for jjj,train_pair in enumerate(train_pairs[:100]):
                before_ast = train_pair.getBeforeAst()
                after_ast = train_pair.getAfterAst()
                new_asts = []
                subtree_root_ids = []
                signal.signal(signal.SIGALRM, handle_timeout)
                signal.alarm(1)
                try:
                    subtree_root_ids.extend(pattern.matchAst(before_ast))
                    new_asts.extend(pattern.applyPattern(before_ast))
                except TimeoutError:
                    print('Timeout')
                except:
                    print('err')
                finally:
                    signal.alarm(0)
                
                for kkk, new_ast in enumerate(new_asts):
                    if new_ast.getCode() == after_ast.getCode():
                        positive_subtrees.append(before_ast.getSubtree(subtree_root_ids[kkk]).getHoppityAst())
                        positives += 1
                    else:
                        negative_subtrees.append(before_ast.getSubtree(subtree_root_ids[kkk]).getHoppityAst())
                        negatives += 1   
                print(positives, negatives)
            positive_pairs_num = 0
            negative_pairs_num = 0
            for jjj in range(len(positive_subtrees)):
                for kkk in range(jjj+1,len(positive_subtrees)):
                    gnn_training_pairs.append([positive_subtrees[jjj],positive_subtrees[kkk],1])
                    positive_pairs_num += 1
            for jjj in range(len(positive_subtrees)):
                for kkk in range(len(negative_subtrees)):
                    gnn_training_pairs.append([positive_subtrees[jjj],negative_subtrees[kkk],0])
                    negative_pairs_num += 1
                    if negative_pairs_num >= positive_pairs_num:
                        break
                if negative_pairs_num >= positive_pairs_num:
                    break
                    

        f = open('gnn_training_pairs2.pkl', 'wb')
        pickle.dump(gnn_training_pairs, f)


    def getGnnTraining(self):
        try:
            if self.proned:
                pass
            else:
                raise
        except:
            if os.path.exists('prevalence.pkl'):
                f = open('prevalence.pkl', 'rb')
                obj = pickle.load(f)
                patterns = obj[0]
                self.proned_patterns = patterns
                self.pattern_prevalence_scores = obj[1]
                f.close()
            else:
                f = open('real_world_train_pairs.pkl', 'rb')
                train_pairs = pickle.load(f)
                f.close()
                patterns = []
                for node in self.hierarchical_nodes: 
                    pattern = node.pattern
                    #if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet())>10 or len(pattern)>100:
                    if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet()) > len(pattern.getBeforePattern())*0.5:
                        continue
                    else:
                        patterns.append(pattern)     
                patterns2 = []
                for i, pattern in enumerate(patterns):
                    flag = False
                    for j in range(i+1, len(patterns)):
                        if pattern == patterns[j]:
                            flag = True
                    if not flag:
                        patterns2.append(pattern)
                patterns = patterns2
                self.pattern_prevalence_scores = []
                for iii, pattern in enumerate(patterns):
                    #if pattern.getBeforePattern().getRootNode().type.type_name == 'if':
                    score = 0
                    success_train_pairs = []
                    success_subtree_root_ids = []
                    for jjj,train_pair in enumerate(train_pairs):
                        #print(jjj)
                        before_ast = train_pair.getBeforeAst()
                        after_ast = train_pair.getAfterAst()
                        new_asts = []
                        subtree_root_ids = []
                        signal.signal(signal.SIGALRM, handle_timeout)
                        signal.alarm(1)
                        try:
                            subtree_root_ids.extend(pattern.matchAst(before_ast))
                            new_asts.extend(pattern.applyPattern(before_ast))
                        except TimeoutError:
                            print('Timeout')
                        except:
                            print('err')
                        finally:
                            signal.alarm(0)
                        for kkk, new_ast in enumerate(new_asts):
                            if new_ast.getCode() == after_ast.getCode():
                                success_subtree_root_ids.append(subtree_root_ids[kkk])
                                success_train_pairs.append(train_pair)
                                score += 1
                                break
                    self.pattern_prevalence_scores.append([pattern, score, success_train_pairs, success_subtree_root_ids])
                    '''
                    if score >= 10:
                        subtree0 = success_train_pairs[0].getBeforeAst().getSubtree(success_subtree_root_ids[0])
                        subtree1 = success_train_pairs[1].getBeforeAst().getSubtree(success_subtree_root_ids[1])
                        string_list = semanticMatch(pattern, subtree0, subtree1)
                    ''' 
                    print(iii)

                self.proned_patterns = patterns
                self.proned = True
                
                patterns = self.proned_patterns
                f = open('prevalence.pkl', 'wb')
                pickle.dump([patterns, self.pattern_prevalence_scores], f)
                f.close()
        


        #print(len(src_tree_patterns))
        self.pattern_prevalence_scores.sort(key = self.getScore2, reverse = True)
        #import pdb
        #pdb.set_trace()
        #patterns.sort(key = self.getHoleNum, reverse = False)
        #patterns.sort(key = self.getPatternSize, reverse = True)
        
        #src_tree_patterns.sort(key = self.getScore, reverse = True)
        new_asts = []
        subtree_root_ids = []
        pattern_src_trees = []
        
        import random
        random.seed(1000)

        f = open('real_world_train_pairs.pkl', 'rb')
        train_pairs = pickle.load(f)
        f.close()
        gnn_training_pairs = []
        random.shuffle(self.pattern_prevalence_scores)
        for iii, pattern_score in enumerate(self.pattern_prevalence_scores):
            pattern = pattern_score[0]
            print(iii)
            positives = 0
            negatives = 0
            
            random.shuffle(train_pairs)
            for jjj,train_pair in enumerate(train_pairs):
                before_ast = train_pair.getBeforeAst()
                after_ast = train_pair.getAfterAst()
                new_asts = []
                subtree_root_ids = []
                signal.signal(signal.SIGALRM, handle_timeout)
                signal.alarm(1)
                try:
                    subtree_root_ids.extend(pattern.matchAst(before_ast))
                    new_asts.extend(pattern.applyPattern(before_ast))
                except TimeoutError:
                    print('Timeout')
                except:
                    print('err')
                finally:
                    signal.alarm(0)
                
                for kkk, new_ast in enumerate(new_asts):
                    if new_ast.getCode() == after_ast.getCode():
                        gnn_training_pairs.append([pattern.getBeforePattern().getHoppityAst(), before_ast.getSubtree(subtree_root_ids[kkk]).getHoppityAst(), 1])
                        positives += 1
                    else:
                        gnn_training_pairs.append([pattern.getBeforePattern().getHoppityAst(), before_ast.getSubtree(subtree_root_ids[kkk]).getHoppityAst(), 0])
                        negatives += 1
                if positives + negatives > 1000:
                    break
                
            print(positives, negatives)
        f = open('gnn_training_pairs.pkl', 'wb')
        pickle.dump(gnn_training_pairs, f)




    def applyPattern(self, src_tree, src_tree_name):
        try:
            if self.proned:
                pass
            else:
                raise
        except:
            if os.path.exists('prevalence.pkl'):
                f = open('prevalence.pkl', 'rb')
                obj = pickle.load(f)
                patterns = obj[0]
                self.proned_patterns = patterns
                self.pattern_prevalence_scores = obj[1]
                f.close()
                self.proned = True
            else:
                f = open('real_world_train_pairs.pkl', 'rb')
                train_pairs = pickle.load(f)
                f.close()
                patterns = []
                for node in self.hierarchical_nodes: 
                    pattern = node.pattern
                    #if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet())>10 or len(pattern)>100:
                    if pattern.hasUnboundHoles() or pattern.isAmbiguous() or len(pattern.getBeforeHolesSet()) > len(pattern.getBeforePattern())*0.5:
                        continue
                    else:
                        patterns.append(pattern)     
                patterns2 = []
                for i, pattern in enumerate(patterns):
                    flag = False
                    for j in range(i+1, len(patterns)):
                        if pattern == patterns[j]:
                            flag = True
                    if not flag:
                        patterns2.append(pattern)
                patterns = patterns2
                self.pattern_prevalence_scores = []
                for iii, pattern in enumerate(patterns):
                    #if pattern.getBeforePattern().getRootNode().type.type_name == 'if':
                    score = 0
                    success_train_pairs = []
                    success_subtree_root_ids = []
                    for jjj,train_pair in enumerate(train_pairs):
                        #print(jjj)
                        before_ast = train_pair.getBeforeAst()
                        after_ast = train_pair.getAfterAst()
                        new_asts = []
                        subtree_root_ids = []
                        signal.signal(signal.SIGALRM, handle_timeout)
                        signal.alarm(1)
                        try:
                            subtree_root_ids.extend(pattern.matchAst(before_ast))
                            new_asts.extend(pattern.applyPattern(before_ast))
                        except TimeoutError:
                            print('Timeout')
                        except:
                            print('err')
                        finally:
                            signal.alarm(0)
                        for kkk, new_ast in enumerate(new_asts):
                            if new_ast.getCode() == after_ast.getCode():
                                success_subtree_root_ids.append(subtree_root_ids[kkk])
                                success_train_pairs.append(train_pair)
                                score += 1
                                break
                    self.pattern_prevalence_scores.append([pattern, score, success_train_pairs, success_subtree_root_ids])
                    '''
                    if score >= 10:
                        subtree0 = success_train_pairs[0].getBeforeAst().getSubtree(success_subtree_root_ids[0])
                        subtree1 = success_train_pairs[1].getBeforeAst().getSubtree(success_subtree_root_ids[1])
                        string_list = semanticMatch(pattern, subtree0, subtree1)
                    ''' 
                    print(iii)

                self.proned_patterns = patterns
                self.proned = True
                
                patterns = self.proned_patterns
                f = open('prevalence.pkl', 'wb')
                pickle.dump([patterns, self.pattern_prevalence_scores], f)
                f.close()
        


        #print(len(src_tree_patterns))
        self.pattern_prevalence_scores.sort(key = self.getScore2, reverse = True)
        #import pdb
        #pdb.set_trace()
        #patterns.sort(key = self.getHoleNum, reverse = False)
        #patterns.sort(key = self.getPatternSize, reverse = True)
        
        #src_tree_patterns.sort(key = self.getScore, reverse = True)
        new_asts = []
        subtree_root_ids = []
        pattern_src_trees = []
        
        
        # for semantic group applying and gnn match applying
        pattern_scores = []
        for pattern_prevalence_score in self.pattern_prevalence_scores:
            pattern = pattern_prevalence_score[0]
            prevalence_score = pattern_prevalence_score[1]
            match_num = len(pattern.matchAst(src_tree))
            if match_num == 0:
                continue
            spec_score = 1.0/match_num
            pattern_scores.append([pattern_prevalence_score, spec_score*prevalence_score])
        pattern_scores.sort(key = self.getScore2, reverse = True)
        '''
        # load the pattern scores directly for efficiency
        try:
            exist=self.pattern_scores
        except:
            f=open('pattern_scores.pkl','rb')
            self.pattern_scores=pickle.load(f)
            f.close()
        pattern_scores=self.pattern_scores
        '''
        '''
        # for old applyGroup
        pattern_scores = []
        for pattern_prevalence_score in self.pattern_prevalence_scores:
            pattern = pattern_prevalence_score[0]
            prevalence_score = pattern_prevalence_score[1]
            match_num = len(pattern.matchAst(src_tree))
            if match_num == 0:
                continue
            spec_score = 1.0/match_num
            pattern_scores.append([pattern, spec_score*prevalence_score])

        pattern_scores.sort(key = self.getScore2, reverse = True)
        '''

        for pattern_score in pattern_scores[:300]:
            pattern_src_trees.append([pattern_score[0], src_tree, src_tree_name])
        
        

        '''
        for pattern_prevalence_score in self.pattern_prevalence_scores[:300]:
            pattern_src_trees.append([pattern_prevalence_score[0], src_tree])
        '''

        #for pattern in patterns:
        #    if len(pattern.matchAst(src_tree)) > 0:
        #        pattern_src_trees.append([pattern,src_tree])
        
        #for i,pattern_src_tree in enumerate(pattern_src_trees):
            #before = time.time()
            #new_ast, subtree_root_id = applyOne(pattern_src_tree)
            #after = time.time()
            #print(after-before)
            #if after-before > 0.5:
                #import pdb
                #pdb.set_trace()
        #pool = multiprocessing.Pool(1)
        #before = time.time()
        #rets = []
        #import random
        #random.seed(1000)
        #random.shuffle(pattern_src_trees)
        #groups = []
        #group_size = len(pattern_src_trees)//5
        #i = 0
        #while i < len(pattern_src_trees):
            #if i+group_size > len(pattern_src_trees):
                #groups.append(pattern_src_trees[i:])
            #else:
                #groups.append(pattern_src_trees[i:i+group_size])
            #i += group_size
        #rets = pool.map_async(applyGroup, [pattern_src_trees]).get()
        
        #patterns, new_asts, subtree_root_ids = applyGroup(pattern_src_trees)
        #patterns, new_asts, subtree_root_ids = applyGroupSemantic(pattern_src_trees)
        #patterns, new_asts, subtree_root_ids = applyGnnSemantic2(pattern_src_trees)
        
        
        f=open('VulRepair_raw_preds_final_beam1.pkl', 'rb')
        T5_output=pickle.load(f)
        f.close()
        patterns, new_asts, subtree_root_ids = applyT5Location(pattern_src_trees, T5_output)
        

        #for pattern_src_tree in pattern_src_trees:
            #rets.append(applyOne(pattern_src_tree))
        #rets = pool.map_async(applyOne, pattern_src_trees, chunksize=len(pattern_src_trees)//40).get()
        #job_list = []
        #for job in jobs:
            #job_list.append(job)
        #for new_ast, subtree_root_id in rets:
            #try:
                #new_asts.extend(new_ast)
                #subtree_root_ids.extend(subtree_root_id)
            #except:
                #pass
        #pool.close()
        #pool.join()
        #after = time.time()
        #print(after-before)

        
        
            #break
        #pool.close()
        #pool.join()

        #patterns = [pattern]
        #print(src_tree_patterns[0][1])
        #import pdb
        #pdb.set_trace()
        return [patterns, new_asts, subtree_root_ids]

    def getHoleNum(self, pattern):
        return len(pattern.getBeforeHolesSet())

    def getPatternSize(self, pattern):
        return len(pattern.getBeforePattern())
    # self.hierarchical_nodes[-1] is the root node of the cluster
    
    def getScore2(self, pattern_score):
        return pattern_score[1]

    def getScore(self, src_tree_pattern):
        if src_tree_pattern[1].hasUnboundHoles() or src_tree_pattern[1].isAmbiguous() or len(src_tree_pattern[1].matchAst(src_tree_pattern[0])) == 0:
            return 0
        else:
            return 1
        #s_prevalence = len(set(src_tree_pattern[1].getSourcePairs()))/len(set(self.hierarchical_nodes[-1].pattern.getSourcePairs()))
        #s_specialized = len(src_tree_pattern[0])/len(src_tree_pattern[1].matchAst(src_tree_pattern[0]))
        return s_prevalence * s_specialized






