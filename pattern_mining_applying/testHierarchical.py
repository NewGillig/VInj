import Hierarchical
import Pattern
import AST
import os
import pickle

#bigvul = os.listdir('VulDatasets/bigvul/')
#devign = os.listdir('VulDatasets/devign/')
#reveal = os.listdir('VulDatasets/reveal/')
#patchdb = os.listdir('VulDatasets/patchdb/')

real_world = os.listdir('reuse_train/')

patterns = []
'''
for file in bigvul:
    if '.pkl' in file:
        f = open('VulDatasets/bigvul/'+file, 'rb')
        parsed_data = pickle.load(f)
        for pair in parsed_data:
            patterns.extend(parsed_data[pair][1])
for file in devign:
    if '.pkl' in file:
        f = open('VulDatasets/devign/'+file, 'rb')
        parsed_data = pickle.load(f)
        for pair in parsed_data:
            patterns.extend(parsed_data[pair][1])
'''

for file in real_world:
    if '.pkl' in file:
        f = open('./reuse_train/'+file, 'rb')
        parsed_data = pickle.load(f)
        for pair in parsed_data:
            patterns.extend(parsed_data[pair][1])
'''
for file in patchdb:
    if '.pkl' in file:
        f = open('VulDatasets/patchdb/'+file, 'rb')
        parsed_data = pickle.load(f)
        for pair in parsed_data:
            patterns.extend(parsed_data[pair][1])
'''
cluster = Hierarchical.HierarchicalCluster(patterns)

#f2 = open('real_world_train_cluster.pkl', 'wb')
#pickle.dump(cluster, f2)

#import pdb
#pdb.set_trace()




