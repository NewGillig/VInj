import os
import random
random.seed(1000)
pairs=os.listdir('./T5_vulgen_final_tokenized3')
random.shuffle(pairs)
for pair in pairs[:int(0.9*len(pairs))]:
    os.system('cp -r ./T5_vulgen_final_tokenized3/'+pair+' ./T5Train_tokenized_final3')

for pair in pairs[int(0.9*len(pairs)):]:
    os.system('cp -r ./T5_vulgen_final_tokenized3/'+pair+' ./T5Test_tokenized_final3')
    
