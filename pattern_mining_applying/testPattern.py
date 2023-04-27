import pdb
from AST import GumTreeAst
from AST import GumTreeAstPair
from Pattern import mergeTreePattern
from Pattern import getEditPatterns

t1 = GumTreeAstPair('e1')   # your pair folder
t2 = GumTreeAstPair('e2')

patterns1 = getEditPatterns(t1)
patterns2 = getEditPatterns(t2)

#before_ast = t.getBeforeAst()
#after_ast = t.getAfterAst()
merged_edit_pattern = patterns1[1].mergeEditPattern(patterns2[2])


pdb.set_trace()
#f=open('./vrend_decode/vrend_decode.c_vul.ast')
#lines=f.readlines()
#t=GumTreeAst(lines)
#pdb.set_trace()
