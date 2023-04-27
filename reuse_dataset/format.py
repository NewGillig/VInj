import os
pairs=os.listdir('./functions_tokenized_space')
for pair in pairs:
    files=os.listdir('./functions_tokenized_space/'+pair)
    for file in files:
        if not os.path.exists('./functions_formatted/'+pair):
            os.mkdir('./functions_formatted/'+pair)
        os.popen('clang-format -style=file ./functions_tokenized_space/'+pair+'/'+file+' > '+'./functions_formatted/'+pair+'/'+file)
