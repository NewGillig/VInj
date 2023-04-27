import pdb
from AST import GumTreeAst
from AST import GumTreeAstPair
import pattern

t = GumTreeAstPair('/home/ynong/VulGeneration/VulDatasets/real_world_final/train/bigvul_Android_CVE-2017-0397_CWE-200_Android_CVE-2017-0397_CWE-200')   # your pair folder
before_astx = t.getBeforeAst()
print(before_astx)
before_ast = t.getBeforeAst().getNodeDict()
after_ast = t.getAfterAst().getNodeDict()
node_map = t.getNodeMap()
inserted_trees = t.getInsertedTrees()
deleted_trees = t.getDeletedTrees()
updated_nodes = t.getUpdatedNodes()

#subtree = t.getBeforeAst().getSubtree(808)

patterns = pattern.getEditPatterns(t)

print(patterns[4])

pdb.set_trace()
#f=open('./vrend_decode/vrend_decode.c_vul.ast')
#lines=f.readlines()
#t=GumTreeAst(lines)
#pdb.set_trace()
