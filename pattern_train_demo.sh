cd pattern_mining_applying
python parse.py reuse_train 0
python parse.py reuse_test 0
python get_dataset_pkl.py
pypy3 testHierarchical.py
bash cluster.sh
