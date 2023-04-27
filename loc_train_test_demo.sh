cd reuse_dataset
#python proc_loc.py
#./tokenize loc_train
#./tokenize loc_test
#cp -r loc_train ../localization/M1_VulRepair_PL-NL/
#cp -r loc_test ../localization/M1_VulRepair_PL-NL/
cd ../localization/M1_VulRepair_PL-NL/
bash train.sh
bash test.sh
