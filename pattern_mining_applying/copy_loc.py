import os
import pandas as pd
import pickle
os.system('cp /root/VulGen/reuse/localization/data/raw_predictions/CodeT5/VulRepair_raw_preds_final_beam1.csv ./')
df=pd.read_csv('VulRepair_raw_preds_final_beam1.csv')
obj=df.to_dict()
f=open('VulRepair_raw_preds_final_beam1.pkl','wb')
pickle.dump(obj,f)
f.close()
