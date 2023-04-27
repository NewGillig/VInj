import os
import pickle
pairs = []
real_world = os.listdir('./reuse_train/') # your dataset path

for file in real_world:
    if '.pkl' in file:
        f = open('./reuse_train/'+file, 'rb')
        parsed_data = pickle.load(f)
        for i,pair in enumerate(parsed_data):
            print(i)
            pairs.append(parsed_data[pair][0])
f = open('reuse_train_final.pkl','wb')
pickle.dump(pairs,f)
f.close()
