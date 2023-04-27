import copy
import const
import os
import re
import Pattern
import json

class GumTreeAstPair:
    def __init__(self, pair_folder):
        self.idx = pair_folder.split('/')[-1]
        self.__before_ast = None
        self.__after_ast = None
        self.__diff_nodes_info = []
        self.__node_map = {}        # e.g., 5:9 means node 5 in before_ast matches node 9 in after_ast
        self.__reversed_node_map = {} # after_node to before_node
        self.__deleted_trees = set()    # root node id in before_ast
        self.__updated_nodes = set()   # id in before_ast. Note that updated nodes can find node_map
        self.__inserted_trees = set()   # root node id in after_ast
        self.__modified_before_trees = set()
        #self.__modified_after_trees = set()
        self.__parsePair(pair_folder)

    def __parsePair(self, pair_folder):
        files = os.listdir(pair_folder)
        before_file = ""
        after_file = ""
        for file in files:
            before_match = re.match(".*" + const.before_postfix, file)
            if before_match != None and before_match.span()[1] == len(file):     # match the postfix
                before_file = pair_folder + "/" + file
                os.system("gumtree parse " + pair_folder + "/" + file + " > " + pair_folder + "/" + file + ".ast")
                f_before_ast = open(pair_folder + "/" + file + ".ast")
                before_ast_lines = f_before_ast.readlines()
                self.__before_ast = GumTreeAst(before_ast_lines)
            after_match = re.match(".*" + const.after_postfix, file)
            if after_match != None and after_match.span()[1] == len(file):     # match the postfix
                after_file = pair_folder + "/" + file
                os.system("gumtree parse " + pair_folder + "/" + file + " > " + pair_folder + "/" + file + ".ast")
                f_after_ast = open(pair_folder + "/" + file + ".ast")
                after_ast_lines = f_after_ast.readlines()
                self.__after_ast = GumTreeAst(after_ast_lines)
        assert before_file != ""
        assert after_file != ""
        diff_file = before_file.replace(const.before_postfix, "") + ".diff"
        os.system("gumtree textdiff " + before_file + " " + after_file + " > " + diff_file)
        f_diff = open(diff_file)
        if len(f_diff.readlines())>30000:
            raise TimeoutError("too big pair")
        f_diff.close()
        f_diff = open(diff_file)
        self.__diff_nodes_info = f_diff.read().split("===")
        i = 1
        info = self.__diff_nodes_info
        while i < len(self.__diff_nodes_info):
            diff_info = self.__diff_nodes_info[i]
            diff_info_lines = diff_info.split('\n')
            if diff_info_lines[1] == "match":
                before_node_info = diff_info_lines[3].split(' ')
                after_node_info = diff_info_lines[4] .split(' ')
                if len(before_node_info) >= 3:
                    before_node_location = before_node_info[0][:-1] + " " + before_node_info[-1]     # remove :
                else:
                    before_node_location = before_node_info[0] + " " + before_node_info[1]
                if len(after_node_info) >= 3:
                    after_node_location = after_node_info[0][:-1] + " " + after_node_info[-1]     # remove :
                else:
                    after_node_location = after_node_info[0] + " " + after_node_info[1]
                before_node_id = self.__before_ast.getNodeByLocation(before_node_location).getIdInAst()
                after_node_id = self.__after_ast.getNodeByLocation(after_node_location).getIdInAst()
                self.__node_map[before_node_id] = after_node_id
                self.__reversed_node_map[after_node_id] = before_node_id
            i += 1
        i = 1
        while i < len(self.__diff_nodes_info):
            diff_info = self.__diff_nodes_info[i]
            diff_info_lines = diff_info.split('\n')
            if "delete" in diff_info_lines[1]:
                before_node_info = diff_info_lines[3].split(' ')
                if len(before_node_info) >= 3:
                    before_node_location = before_node_info[0][:-1] + " " + before_node_info[-1]     # remove :
                else:
                    before_node_location = before_node_info[0] + " " + before_node_info[1]
                before_node = self.__before_ast.getNodeByLocation(before_node_location)
                before_node_id = before_node.getIdInAst()
                self.__deleted_trees.add(before_node_id)
                self.__modified_before_trees.add(before_node_id)
                cur_node = before_node
                while cur_node.parent != None:
                    self.__modified_before_trees.add(cur_node.parent.getIdInAst())
                    cur_node = cur_node.parent
            if "insert" in diff_info_lines[1]:
                after_node_info = diff_info_lines[3].split(' ')
                if len(after_node_info) >= 3:
                    after_node_location = after_node_info[0][:-1] + " " + after_node_info[-1]     # remove :
                else:
                    after_node_location = after_node_info[0] + " " + after_node_info[1]
                after_node = self.__after_ast.getNodeByLocation(after_node_location)
                after_node_id = after_node.getIdInAst()
                self.__inserted_trees.add(after_node_id)
                cur_node = after_node
                while cur_node.parent != None:
                    if cur_node.parent.getIdInAst() in self.__reversed_node_map:
                        self.__modified_before_trees.add(self.__reversed_node_map[cur_node.parent.getIdInAst()])
                    cur_node = cur_node.parent


            if "update" in diff_info_lines[1]:
                before_node_info = diff_info_lines[3].split(' ')
                if len(before_node_info) >= 3:
                    before_node_location = before_node_info[0][:-1] + " " + before_node_info[-1]     # remove :
                else:
                    before_node_location = before_node_info[0] + " " + before_node_info[1]
                before_node = self.__before_ast.getNodeByLocation(before_node_location)
                before_node_id = self.__before_ast.getNodeByLocation(before_node_location).getIdInAst()
                self.__updated_nodes.add(before_node_id)
                self.__modified_before_trees.add(before_node_id)
                cur_node = before_node
                while cur_node.parent != None:
                    self.__modified_before_trees.add(cur_node.parent.getIdInAst())
                    cur_node = cur_node.parent
            if "move" in diff_info_lines[1]:
                before_node_info = diff_info_lines[3].split(' ')
                if len(before_node_info) >= 3:
                    before_node_location = before_node_info[0][:-1] + " " + before_node_info[-1]     # remove :
                else:
                    before_node_location = before_node_info[0] + " " + before_node_info[1]
                
                before_node = self.__before_ast.getNodeByLocation(before_node_location)
                def markUpdatedNode(before_node):
                    before_node_id = before_node.getIdInAst()
                    self.__deleted_trees.add(before_node_id)
                    self.__modified_before_trees.add(before_node_id)
                    after_node_id = self.__node_map[before_node_id]
                    self.__inserted_trees.add(after_node_id)
                    cur_node = before_node
                    while cur_node.parent != None:
                        self.__modified_before_trees.add(cur_node.parent.getIdInAst())
                        cur_node = cur_node.parent
                    for child in before_node.children:
                        markUpdatedNode(child)
                markUpdatedNode(before_node)
            i += 1             

    def getBeforeAst(self):
        return self.__before_ast
        #return_ast = copy.deepcopy(self.__before_ast)
        #return return_ast

    def getAfterAst(self):
        return self.__after_ast
        #return_ast = copy.deepcopy(self.__after_ast)
        #return return_ast

    def getNodeMap(self):
        return_map = copy.deepcopy(self.__node_map)
        return return_map

    def getReversedNodeMap(self):
        return_map = copy.deepcopy(self.__reversed_node_map)
        return return_map


    def getInsertedTrees(self):
        return_set = copy.deepcopy(self.__inserted_trees)
        return return_set

    def getDeletedTrees(self):
        return_set = copy.deepcopy(self.__deleted_trees)
        return return_set

    def getUpdatedNodes(self):
        return_set = copy.deepcopy(self.__updated_nodes)
        return return_set
    
    def getModifiedBeforeTrees(self):
        return_set = copy.deepcopy(self.__modified_before_trees)
        return return_set
    '''
    def isSubtreeModified2(self, subtree_root_node_id):
        subtree_root_node = self.__before_ast.getNodeDict()[subtree_root_node_id]
        if subtree_root_node.getIdInAst() in self.__updated_nodes or subtree_root_node.getIdInAst() in self.__deleted_trees:
            return True
        for child in subtree_root_node.children:
            if self.isSubtreeModified2(child.getIdInAst()):
                return True
        return False
    
    def isSubtreeModified(self, subtree_root_node_id):
        subtree_root_node = self.__before_ast.getNodeDict()[subtree_root_node_id]
        if subtree_root_node.getIdInAst() in self.__updated_nodes or subtree_root_node.getIdInAst() in self.__deleted_trees:
            return True
        for child in subtree_root_node.children:
            if self.isSubtreeModified2(child.getIdInAst()):
                return True
        if not subtree_root_node_id in self.__node_map:
            return True
        after_subtree_root_node_id = self.__node_map[subtree_root_node_id]
        if self.isAfterSubtreeInserted(after_subtree_root_node_id):
            return True
        return False

    def isAfterSubtreeInserted(self, subtree_root_node_id):
        subtree_root_node = self.__after_ast.getNodeDict()[subtree_root_node_id]
        if subtree_root_node.getIdInAst() in self.__inserted_trees:
            return True
        for child in subtree_root_node.children:
            if self.isAfterSubtreeInserted(child.getIdInAst()):
                return True
        return False
    '''
class GumTreeAstType:
    def __init__(self, type_name):
        self.type_name = type_name
        # self.children_types = []          # May implement the ASDL tree later
        # self.parent_type = None
    def __eq__(self, other):
        if type(self) != type(other):
            return False
        return self.type_name == other.type_name

class GumTreeAstNode:
    def __init__(self, AST = None, id_in_ast = -1):
        # Each node must bind with an AST and its id in the AST is unique. Once set, they cannot change
        self.__AST = AST
        self.__id_in_ast = id_in_ast
        
        self.type = None 
        self.value = ''
        self.children = []
        self.parent = None
        self.location = ''  # hard to locate after modifying the AST, will be only used as in the initial AST

    def getAst(self):
        return self.__AST

    def getIdInAst(self):
        return self.__id_in_ast

    def __str__(self):
        string = "id: " + str(self.__id_in_ast) + "\ntype: "+ self.type.type_name + "\nvalue: " + self.value + "\n children:" + "[ "
        for child in self.children:
            string = string + str(child.getIdInAst()) +' '
        string = string + ']\nparent: '
        if self.parent != None:
            string = string + str(self.parent.getIdInAst()) 
        string = string + '\nlocation: ' + self.location
        return string
    
    def __eq__(self, other):
        if type(other) != type(self):
            return False
        if self.type != other.type or self.value != other.value:
            return False
        if len(self.children) != len(other.children):
            return False
        for i in range(len(self.children)):
            if self.children[i] != other.children[i]:
                return False
        return True

class GumTreeAst:
    def __init__(self, file_lines):
        self.__root_node = None
        self.__node_dict = {}
        self.__node_dict_by_location = {}
        self.__new_idx = 0    # Each node has a unique idx in an AST. This number only increases.
        
        self.__readAst(file_lines)

    def __eq__(self, other):
        return self.__root_node == other.__root_node
    
    def size(self):
        return len(self.__node_dict)

    def __len__(self):
        return self.size()
    
    def addRootNode(self, type, value):      # only if root_node == None
        if self.__root_node == None:
            self.__root_node = GumTreeAstNode(self, self.__new_idx)
            self.__root_node.type = type      # remove :
            self.__root_node.value = value
            self.__node_dict[self.__new_idx] = self.__root_node
            parent_node_id = self.__new_idx
            self.__new_idx += 1

    def getRootNode(self):
        #return_node = copy.deepcopy(self.__root_node)
        return_node = self.__root_node
        return return_node

    def getNodeDict(self):
        #return_dict = copy.deepcopy(self.__node_dict)
        return_dict = self.__node_dict
        return return_dict

    def getNodeByLocation(self, location):
        dct = self.__node_dict_by_location
        #return_node = copy.deepcopy(self.__node_dict_by_location[location])
        return_node = self.__node_dict_by_location[location]
        return return_node
    
    def addSingleNode(self, type, value, parent_node_id, children_list_location, location=''):
        new_node = GumTreeAstNode(self, self.__new_idx)
        new_node.type = type
        new_node.value = value
        new_node.children = []
        new_node.parent = self.__node_dict[parent_node_id]
        new_node.location = location
        self.__node_dict[parent_node_id].children.insert(children_list_location, new_node)
        self.__node_dict[self.__new_idx] = new_node
        self.__node_dict_by_location[new_node.type.type_name + " " + new_node.location] = new_node
        self.__new_idx += 1
        return self.__new_idx - 1       # return the id of the added node

    def getSubtree(self, subtree_root_node_id, node_map = {}):
        node_map[0] = subtree_root_node_id
        ret_subtree = GumTreeAst([])
        ret_subtree.__root_node = GumTreeAstNode(ret_subtree,ret_subtree.__new_idx)
        src_subtree_root_node = self.__node_dict[subtree_root_node_id]
        ret_subtree.__root_node.type = src_subtree_root_node.type
        ret_subtree.__root_node.value = src_subtree_root_node.value
        ret_subtree.__node_dict[ret_subtree.__new_idx] = ret_subtree.__root_node
        ret_subtree.__new_idx += 1
        for i, child in enumerate(src_subtree_root_node.children):
            self.copySubtree(child, ret_subtree, ret_subtree.__root_node.getIdInAst(), i, node_map)

        return ret_subtree
        

    def copySubtree(self, src_subtree_root_node, tgt_tree, tgt_subtree_parent_node_id, tgt_subtree_children_list_location, node_map = {}):
        new_node_id = tgt_tree.addSingleNode(src_subtree_root_node.type, src_subtree_root_node.value, tgt_subtree_parent_node_id, tgt_subtree_children_list_location)
        node_map[new_node_id] = src_subtree_root_node.getIdInAst()
        for i,child in enumerate(src_subtree_root_node.children):
            self.copySubtree(child, tgt_tree, new_node_id, i, node_map)

    def __str__(self):
        if self.__root_node == None:
            return ""
        return self.printSubtree(self.__root_node, 0)
    
    def printSubtree(self, subtree_root_node, depth):
        string = " " * depth + str(subtree_root_node.getIdInAst()) + ": " + subtree_root_node.type.type_name + ": " + subtree_root_node.value + "\n"
        #temporary change the printed string for case study
        #string = " " * depth + subtree_root_node.type.type_name + ": " + subtree_root_node.value + "\n"
        for child in subtree_root_node.children:
            string = string + self.printSubtree(child, depth + 1) 
        #string = string + " "* depth + ")"
        return string

    def getHoppityAst(self):
        try:
            tmp = self.type_dict
        except:
            f = open('./type_dict.json')
            self.type_dict = json.load(f)
            f.close()

        obj = {}
        obj["type"] = "Module"
        obj["directives"] = []
        obj["items"] = []
        root_node = self.getRootNode()
        if root_node.type.type_name == 'unit':
            obj["idx"] = 0
            children = root_node.children
            for child in children:
                self.__getHoppityAstHelper(child, obj["items"], True)
        else:
            obj["idx"] = -1
            self.__getHoppityAstHelper(root_node, obj["items"], True)
        return obj

    def __getHoppityAstHelper(self, subtree_root_node, obj, is_list):
        new_obj = {}
        if not is_list:
            new_obj = obj
        new_obj["type"] = subtree_root_node.type.type_name
        new_obj["idx"] = subtree_root_node.getIdInAst()
        if new_obj["type"] in self.type_dict and len(self.type_dict[new_obj["type"]]) == 1:
            fields = self.type_dict[new_obj["type"]][0]
            for i, field in enumerate(fields):
                field_name = ""
                for j, t in enumerate(field):
                    if j == 0 :
                        field_name = field_name + t.upper()
                    elif j > 0 and field[j-1] == '_':
                        field_name = field_name + t.upper()
                    else:
                        field_name = field_name + t
                new_obj[field_name] = {}
                child = subtree_root_node.children[i]
                self.__getHoppityAstHelper(child, new_obj[field_name], False)
            if len(fields) == 0:
                new_obj["Token"] = {}
                new_obj["Token"]["type"] = "SyntaxToken"
                new_obj["Token"]["value"] = subtree_root_node.value
        #elif new_obj["type"] in self.type_dict and len(self.type_dict[new_obj["type"]]) > 1:
        else:
            new_obj[subtree_root_node.type.type_name + "List"] = []
            for child in subtree_root_node.children:
                self.__getHoppityAstHelper(child, new_obj[subtree_root_node.type.type_name + "List"], True)
            if len(subtree_root_node.children) == 0:
                new_obj[subtree_root_node.type.type_name + "List"] = {}
                new_token = new_obj[subtree_root_node.type.type_name + "List"]
                new_token["type"] = "SyntaxToken"
                new_token["value"] = subtree_root_node.value
        #import pdb
        #pdb.set_trace()
        if is_list:
            obj.append(new_obj)


    def getCode(self, mark = None):
        pattern_text = self.__str__()
        lines = pattern_text.split("\n")
        code = ""
        stack = []
        prev_node_type = ""
        ternary_flag = False
        prev_indent = 0
        for iii, line in enumerate(lines):
            if iii == mark:
                code = code + "/*target_line*/"
            indent = 0
            for i,t in enumerate(line):
                if t!=" ":
                    indent = i
                    break
            while len(stack) > 0 and indent <= stack[-1][0]:
                #pdb.set_trace()
                code = code + stack[-1][1] + " "
                stack.pop()
            split_line = line.split(":")
            node_type = ""
            if len(split_line) > 1:
                node_type = split_line[1][1:]

            if node_type == "parameter_list":
                code = code + "( "
                stack.append([indent, ")"])

            if node_type == "argument_list":
                code = code + "( "
                stack.append([indent, ")"])

            if node_type == "block":
                code = code + "{ "
                stack.append([indent, "}"])

            if node_type == "index":
                code = code + "[ "
                stack.append([indent, "]"])
            
            if node_type == "ternary":
                ternary_flag = True
            
            if node_type == "condition" and prev_indent < indent and prev_node_type != "ternary":
                code = code + "( "
                stack.append([indent, ")"])

            if node_type == "condition" and prev_indent >= indent:
                stack.append([indent, ";"])

            if node_type == "then":
                code = code + "?"

            if node_type == "control":
                code = code + "( "
                stack.append([indent, ")"])

            if node_type == "decl_stmt":
                stack.append([indent, ";"])

            if node_type == "expr_stmt":
                stack.append([indent, ";"])

            if node_type == "if":
                code = code + "if "

            if node_type == "continue":
                code = code + "continue ; "


            if node_type == "while":
                code = code + "while "

            if node_type == "switch":
                code = code + "switch "

            if node_type == "case":
                code = code + "case "
                stack.append([indent, ":"])
            
            if node_type == "break":
                code = code + "break ; "

            if node_type == "default":
                code = code + "default : "

            if node_type == "sizeof":
                code = code + "sizeof "

            if node_type == "else" and ternary_flag == False:
                code = code + "else "

            if node_type == "else" and ternary_flag == True:
                code = code + ": "
                ternary_flag = False

            if node_type == "for":
                code = code + "for "

            if node_type == "return":
                code = code + "return "
                stack.append([indent, ";"])

            if node_type == "init" and prev_indent == indent:
                code = code + "= "

            if node_type == "init" and prev_indent != indent:
                stack.append([indent, ";"])

            if node_type == "parameter" and indent <= prev_indent and indent != 0:
                code = code + ", "

            if node_type == "argument" and indent <= prev_indent and indent != 0:
                code = code + ", "

            if node_type == "decl" and indent <= prev_indent and indent != 0:
                code = code + ", "

            token = ""
            if len(split_line) > 2:
                colon_num = 0
                for i,t in enumerate(line):
                    if t == ":":
                        colon_num += 1
                    if colon_num == 2:
                        token = line[i+2:]
                        break
            if token!="":
                code = code + token + " "
            prev_node_type = node_type
            prev_indent = indent
        return code
    
    def __readAst(self, file_lines):
        i = 0
        while i < len(file_lines):
            if file_lines[i][-1]=="\n":
                file_line = file_lines[i][:-1]
            else:
                file_line = file_lines[i]
            current_depth = 0
            indent = 0
            for c in file_line:
                if c == ' ':
                    indent+=1
                else:
                    break
            line_depth = indent // 4
            if line_depth == current_depth:            # set root_node
                node_info = file_line.split(' ')
                self.__root_node = GumTreeAstNode(self, self.__new_idx)
                if len(node_info) >= 3:
                    self.__root_node.type = GumTreeAstType(node_info[0][:-1])      # remove :
                    value = ""
                    for j in range(1, len(node_info) - 1):
                        value = value + node_info[j] + " "
                    value = value[:-1]
                    self.__root_node.location = node_info[-1]
                else:
                    self.__root_node.type = GumTreeAstType(node_info[0])      
                    self.__root_node.location = node_info[1]
                self.__node_dict[self.__new_idx] = self.__root_node
                self.__node_dict_by_location[self.__root_node.type.type_name + " " + self.__root_node.location] = self.__root_node
                parent_node_id = self.__new_idx
                self.__new_idx+=1       
                i += 1
                continue

            # subtrees  
            if line_depth == current_depth + 1:
                stop_line = self.__readSubTree(parent_node_id, file_lines, i, current_depth + 1)
                i = stop_line
            i += 1

    def __readSubTree(self, parent_node_id, file_lines, cur_line, current_depth):
        i = cur_line
        while i < len(file_lines):
            if file_lines[i][-1]=="\n":
                file_line = file_lines[i][:-1]     
            else:
                file_line = file_lines[i]
            indent = 0
            for c in file_line:
                if c == ' ':
                    indent+=1
                else:
                    break
            line_depth = indent // 4
            if line_depth == current_depth:
                node_info = file_line[indent:].split(' ')
                if len(node_info) >= 3:
                    type = GumTreeAstType(node_info[0][:-1])      # remove :
                    value = ""
                    for j in range(1, len(node_info) - 1):
                        value = value + node_info[j] + " "
                    value = value[:-1]
                    location = node_info[-1]
                else:
                    type = GumTreeAstType(node_info[0]) 
                    value = ''
                    location = node_info[1]
                self.addSingleNode(type,value,parent_node_id, len(self.__node_dict[parent_node_id].children), location)
                cur_parent_node_id = self.__new_idx - 1
                i += 1
                continue
            if line_depth == current_depth + 1:
                stop_line = self.__readSubTree(cur_parent_node_id, file_lines, i, current_depth + 1)
                i = stop_line
            if line_depth < current_depth:
                return i - 1
            i += 1
        return i - 1   




