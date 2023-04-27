import os

os.system('mkdir ./loc_test')
pairs=os.listdir('./reuse_test')
for i,pair in enumerate(pairs):
    files=os.listdir('./reuse_test/'+pair)
    for file in files:
        if '_nonvul.c' in file:
            p=os.popen('diff ./reuse_test/'+pair+'/'+file+' ./reuse_test/'+pair+'/'+file.replace('_nonvul.c', '_vul.c'))
    diff=p.readlines()
    os.system('mkdir ./loc_test/'+pair)
    for file in files:
        if '_nonvul.c' in file:
            os.popen('cp ./reuse_test/'+pair+'/'+file+' ./loc_test/'+pair+'/'+pair+'_in.c')
    f=open('./loc_test/'+pair+'/'+pair+'_out.c', 'w')

    after=""
    breakflag = False
    for diffline in diff:
        if diffline[0] == '>':
            breakflag = True
            break
        if diffline[0] == '<':
            after=after+diffline[2:]
    if 'if' in after and len(after.split('\n')) <= 2:
        f2=open('./reuse_test/'+pair+'/'+file)
        lines=f2.readlines()
        f2.close()
        try:
            diff_loc=int(diff[0].split('c')[0])-1
        except:
            diff_loc=int(diff[0].split('d')[0])-1
        if_indent=0
        for c in lines[diff_loc]:
            if not c==' ':
                break
            else:
                if_indent+=1
        for loc in range(diff_loc+1,len(lines)):
            indent=0
            line=lines[loc]
            for c in line:
                if not c==' ':
                    break
                else:
                    indent+=1
            if indent == if_indent:
                if '}' in line:
                    after=after+'\n'+line
                break
            elif indent < if_indent:
                break
            else:
                after=after+'\n'+line
    f.write(after)
    f.close()

os.system('mkdir loc_train')
pairs=os.listdir('./reuse_train')
for i,pair in enumerate(pairs):
    files=os.listdir('./reuse_train/'+pair)
    for file in files:
        if '_nonvul.c' in file:
            p=os.popen('diff ./reuse_train/'+pair+'/'+file+' ./reuse_train/'+pair+'/'+file.replace('_nonvul.c', '_vul.c'))
    diff=p.readlines()
    os.system('mkdir ./loc_train/'+pair)
    for file in files:
        if '_nonvul.c' in file:
            os.popen('cp ./reuse_train/'+pair+'/'+file+' ./loc_train/'+pair+'/'+pair+'_in.c')
    f=open('./loc_train/'+pair+'/'+pair+'_out.c', 'w')

    after=""
    breakflag = False
    for diffline in diff:
        if diffline[0] == '>':
            breakflag = True
            break
        if diffline[0] == '<':
            after=after+diffline[2:]
    if 'if' in after and len(after.split('\n')) <= 2:
        f2=open('./reuse_train/'+pair+'/'+file)
        lines=f2.readlines()
        f2.close()
        try:
            diff_loc=int(diff[0].split('c')[0])-1
        except:
            diff_loc=int(diff[0].split('d')[0])-1
        if_indent=0
        for c in lines[diff_loc]:
            if not c==' ':
                break
            else:
                if_indent+=1
        for loc in range(diff_loc+1,len(lines)):
            indent=0
            line=lines[loc]
            for c in line:
                if not c==' ':
                    break
                else:
                    indent+=1
            if indent == if_indent:
                if '}' in line:
                    after=after+'\n'+line
                break
            elif indent < if_indent:
                break
            else:
                after=after+'\n'+line
    f.write(after)
    f.close()

