import os
import sys
from AST import GumTreeAst
from AST import GumTreeAstPair
from AST import GumTreeAstType
import pickle
import Pattern


def parse(dataset_path, start_point):
    parsed_data = {}        # key is the pair name, value is a list. The first element is GumTreeAstPair, the second element is the patterns list
    pairs=os.listdir(dataset_path)
    for i,pair in enumerate(pairs):
        if i<start_point:
            continue
        if i>start_point+1000:
            break
        print(i,pair)
        try:
            t = GumTreeAstPair(dataset_path+"/"+pair)
        except TimeoutError as e:
            print(e)
            continue
        except:
            print('err')
            continue
        try:
            patterns = Pattern.getEditPatterns(t)
        except:
            print('err')
            continue
        parsed_data[pair] = [t,patterns]
        fpatterns = open(dataset_path+"/"+pair+"/"+pair+".patterns", 'w')
        for pattern in patterns:
            #if "unit" in pattern.__str__().split("\n")[0]:
                #print(pattern.getBeforePattern().getCode())
                #import pdb
                #pdb.set_trace()
            fpatterns.write(pattern.__str__())
            fpatterns.write("=====\n")
        fpatterns.close()
    fdata = open(dataset_path+"/data_"+str(start_point)+".pkl", "wb")
    pickle.dump(parsed_data,fdata)

#parse('VulDatasets/bigvul')

start_point = int(sys.argv[2])
dataset_path = sys.argv[1]
#dataset_path = 'VulDatasets/devign'

parse(dataset_path, start_point)
#fdatax = open(dataset_path+"/data_"+str(start_point)+".pkl", "rb")
#parsed_datax = pickle.load(fdatax)
#import pdb
#pdb.set_trace()
#parse('VulDatasets/patchdb')
#parse('VulDatasets/reveal')
#parse('test2')



