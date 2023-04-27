import Hierarchical
import Pattern
import AST
import os
import pickle
import multiprocessing
import difflib
import random
random.seed(3000)

def testPairs(pairs):
    for i, pair in enumerate(pairs):
        #if os.path.exists('./generated/'+pair.idx+'_vul.c'):   
        #    continue
        before_ast = pair.getBeforeAst()
        after_ast = pair.getAfterAst()
        f=open('./generated/'+pair.idx+'_nonvul.c','w')
        f.write(before_ast.getCode())
        f.close()
        f=open('./generated/'+pair.idx+'_vul.c','w')
        f.write(after_ast.getCode())
        f.close()
        #import pdb
        #pdb.set_trace()
        rets = hierarchical_cluster.applyPattern(before_ast, pair.idx)
        #rets = [[],[],[]]
        new_asts = rets[1]
        subtree_root_ids = rets[2]
        patterns = rets[0]
        flag = False
        before_nodes = before_ast.__str__().split('\n')
        after_nodes = after_ast.__str__().split('\n')
        diff = difflib.Differ()
        differences = list(diff.compare(before_nodes, after_nodes))
        del_flag = "del: false\n"
        add_flag = "add: false\n"
        for d in differences:
            if d[0] == '-':
                del_flag = "del: true\n"
            if d[0] == '+':
                add_flag = "add: true\n"

        #import pdb
        #pdb.set_trace()
        '''
        test_asts = []
        for i, pattern in enumerate(patterns):
            for new_ast in new_asts[i]:
                test_asts.append(new_ast)
        #import pdb
        #pdb.set_trace()
        if not len(test_asts)-1 < 0:
            j = random.randint(0, len(test_asts)-1)
            test_ast = test_asts[j]
            if test_ast.getCode() == after_ast.getCode():
                flag = True
        '''
        breakflag = False
        #for i, pattern in enumerate(patterns):
        for ppp in range(1):
            if not len(new_asts)-1 < 0:
                i = random.randint(0, len(new_asts)-1)
                pattern = patterns[i]
                if not len(new_asts[i])-1 < 0:
                    j = random.randint(0, len(new_asts[i])-1)
                
                #print(i,len(new_asts[i]))
                #for j, new_ast in enumerate(new_asts[i]):
                    print(i,len(new_asts[i]),j)
                    new_ast = new_asts[i][j]
                    f=open('./generated/'+pair.idx+'_gen.c','w')
                    f.write(new_ast.getCode())
                    f.close()
                    if new_ast.getCode() == after_ast.getCode():
                        flag = True
                        print("    ", j)
                        #breakflag = True
                        #break
                    else:
                        flag = False
                        #breakflag = True
                        #break
                #if breakflag:
                #    break
        if flag:
            print(pair.idx + "\nsuccess\n" + del_flag + add_flag)
        else:
            print(pair.idx + "\nfail\n" + del_flag + add_flag)


pairs = []
#real_world = os.listdir('VulDatasets/real_world_final/test/')

if os.path.exists('vulgen_test_final.pkl'):
    f = open('vulgen_test_final.pkl','rb')
    pairs = pickle.load(f)
    f.close()
else:
    exit()

hierarchical_cluster = Hierarchical.HierarchicalCluster([])
clusters = os.listdir('./clusters')
for cluster in clusters:
    try:
        f = open('./clusters/'+cluster,'rb')
        hierarchical_cluster.hierarchical_nodes.extend(pickle.load(f))
        f.close()
    except:
        pass

testPairs(pairs)






