# VInJ: Automatic Vulnerability Dataset Generation

## Introduction
![Alt text](https://raw.githubusercontent.com/NewGillig/VInj/master/workflow.png)

Essentially, VInJ is a well implemented tool that separate *where* and *how* to inject vulnerabilities into existing programs, to automatically generate vulnerability datasets. It can be also used for other code edit tasks that need to first locate the code and then edit.

## Installation
We provide two ways to install VInJ. The best way is to use our docker image and this is recommended:
```
docker pull g2ecb/vinj:latest
containerid=`docker run -d -it --gpus all g2ecb/vinj:latest bash`
docker exec -it $containerid bash
```
Once the docker container is set up, you can directly go to **usage** Section for the next step.

Otherwise, please follow the instructions below to set up the environments on your computer.

First, set up the environments and install dependencies.
1. Install Java>=11.
2. Install srcml for parsing code into AST: https://www.srcml.org/
3. Download and extract our specific **gumtree-lite.zip**: https://figshare.com/s/d85b715edc26ea99c5fa.
4. Set environment path for gumtree-lite:
```
export GUMTREE=YOUR_GUMTREE_PATH
export PATH=$PATH:$GUMTREE
```
5. Clone the VInj repository: 
```
git clone https://github.com/NewGillig/VInj.git
```
6. Install the dependencies of the localization model:
```
pip install transformers
pip install torch
pip install numpy
pip install tqdm
pip install pandas
pip install tokenizers
pip install datasets
pip install gdown
pip install tensorboard
pip install scikit-learn
```
7. Install pypy3 to improve efficiency: https://www.pypy.org/

## Usage
Because of the repository size limit on GitHub, we only upload a small set of training and testing samples to the repository. You can run the following script in the repository to have a quick test.
```
source loc_train_test.sh
source pattern_train_demo.sh
source pattern_test_demo.sh 
```
To do complete training and testing, please download our full training and testing datasets for localization and pattern mining/application: 
https://figshare.com/s/d85b715edc26ea99c5fa **localization_train_test_data.zip** and **vulgen_train_test_final.zip**. Then replace the respective folders and files with the demo ones.

You can also directly download our trained localization model and patterns for testing, by download them in https://figshare.com/s/d85b715edc26ea99c5fa **VInj_trained_pattern_model.zip**. Or you can directly run the script to automatically download models and inject vulnerabilities on our testing set shown in the paper:
```
source pattern_test.sh
``` 

The vulnerability-injected samples are outputted to 
```
./pattern_mining_application/generated/
```
