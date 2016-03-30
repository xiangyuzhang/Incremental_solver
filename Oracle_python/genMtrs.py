#!/usr/bin/python

import re
import os
import time
from testforbid import CBconstrain
from completeset import completeset




'''
The difference between genFirstMtr and genScndMtr is that\
in genScndMtr, PI input verctors together with programming\
bits are assigned, while genFirstMtr only requires assignmets\
for programming bits.
The genThdMtr only has PI input vectors assigned, the p-bits\
are unknown. The goal is to find p-bit pair assignments that\
current PI vectors cannot distinguish.
'''

def is_sequence(arg):
    '''This function judges whether arg is a list or str. List has no .strip() attribute.\
Str has no '__iter__' attribute.'''
    return (not hasattr(arg, "strip") and
        hasattr(arg, "__getitem__") or
        hasattr(arg, "__iter__"))


#################################################
#################################################
#################################################
#################################################




    #4. Miter module:
    #4.1 Build a miter for programming bit pair to make sure they are different
    """
    pbitXORlines = ''
    pbitORlines = '\n.names '
    pbitNum = len(topPbits[0])
    for i in range(pbitNum):
        pbitXORlines += '\n.names '+topPbits[0][i]+' '+topPbits[1][i]+' '
        pORin = 'pxor'+str(i)
        pbitXORlines += pORin+'\n10 1' + '\n01 1'
        pbitORlines += pORin + ' '
    pbitORlines += 'pb_mtrOut\n' #the output of the miter to distinguish Si w/ Sj
    for i in range(len(lev1pbits)):
        pbitORlines += i*'-'+'1'+(len(lev1pbits)-i-1)*'-'+' 1\n'
    subMtrDistPbits = pbitXORlines+pbitORlines
    """
    """
    #4.2 Rule out the illegal p-bit assignments (11 currently):
    nandRuleLines = ''
    nand2outerAND = [] #part of inputs to the final AND gate
    for i in range(len(topPbits)):
        SI = topPbits[i]
        singlePBITset = len(SI)
        andCnt = 1
        for n in xrange(0, singlePBITset, 2): #choose stepsize=2 b/c MUX has 2 p-bits
            tmpPlist = SI[n:n+2]
            tmpNAND = 'topNAND'+str(i)+'_'+str(andCnt)
            nand2outerAND.append(tmpNAND)
            nandRuleLines += '.names '+tmpPlist[0]+' '+tmpPlist[1]+' '+tmpNAND
            nandRuleLines += '\n0- 1\n-0 1\n'
            andCnt += 1
    """
    '''
    #4.3 build a miter that generate a PI vector that distinguishes new found pbit pair:
    miterMods = []
    miterPOs = []
    for num in range(2):
        subMiterPO = []
        miterSubckt = '.subckt '+lev1model[0]+' '
        for i in range(len(lev1pinputs)):
            miterSubckt += lev1pinputs[i]+'='+distingPI[i]+' '
        for j in range(len(lev1pbits)):
            miterSubckt += lev1pbits[j]+'='+topPbits[num][j]+' '
        for k in range(len(lev1outputs)):
            miterSubckt += lev1outputs[k]+'=miter'+str(num)+'_'+str(k)+' '
            subMiterPO.append('miter'+str(num)+'_'+str(k))
        miterMods.append(miterSubckt)
        miterPOs.append(subMiterPO)
    genPImtrXORs = ''
    genPImtrORin = []
    #XORs:
    for i in range(len(lev1outputs)):
        subORin = 'mtrORin_'+str(i)
        genPImtrORin.append(subORin)
        subMtrXOR = '.names '+miterPOs[0][i]+' '+miterPOs[1][i]+' '+subORin
        subMtrXOR += '\n10 1\n01 1\n'
        genPImtrXORs += subMtrXOR
    #ORs:
    genPImtrOR ='.names '+(' ').join(genPImtrORin)+' mtrOut\n'
    for i in range(len(lev1outputs)):
        genPImtrOR += i*'-'+'1'+(len(lev1outputs)-i-1)*'-'+' 1\n'
    #roll the lines together:
    genPImtr = ''
    for subc in miterMods:
        genPImtr += subc+'\n'
    genPImtr += genPImtrXORs
    genPImtr += genPImtrOR

    #4.4 Build the final AND miter:
    #signals ANDed together: nand2outerAND, pb_mtrOut, out of miter which gens PI.
    #finalAND = '.names '
    #4.4.1 add NANDs which rule out the illegal pbit assignments:
    #finalAND += (' ').join(nand2outerAND)
    #4.4.2 add sub miter which rules out same Pbit assignments:
    #finalAND += ' pb_mtrOut '
    #4.4.3 add genPImtrOut:
    #finalAND += ' genPImtrOut finalMtrOut\n'
    #finalAND += '1'*(len(nand2outerAND)+2)+' 1\n'
    #finalAND += '1'*(len(nand2outerAND)+1)+' 1\n'

    #5. complete top level
    #topModel = topModName+topModIn+topModOut+topscs+subMtrDistPbits+nandRuleLines+genPImtr+finalAND+'.end\n'
    topModel = topModName+topModIn+topModOut+topscs+genPImtr+'.end\n'
    #######################################
    # Combine all levels of models together
    #######################################
    outxt = topModel +'\n'+ level2model +'\n'+ lev1

    with open(compCktPath, 'w') as outfile:
        outfile.write(outxt)
        #print "\nTop miter is generated and stored in '", outputfilePath, "'"
    return None

'''

#################################################
#################################################
#################################################
#################################################



def gateType(line):
    '''determines gate type according to the behavioral description in Verilog'''
    '''Just applys to INV, BUF, AND2, OR2, XOR2.'''


    return



'''
def transINV( lineIn, lineOut ):
    inV = str(lineIn[0])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV+' '+outV+' 0\n')
    cnfLines.append('-'+inV+' -'+outV+' 0\n')
    return cnfLines

def transAND( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append('-'+inV1+' -'+inV2+' '+outV+' 0\n')
    cnfLines.append(inV1+' -'+outV+' 0\n')
    cnfLines.append(inV2+' -'+outV+' 0\n')
    return cnfLines

def transOR( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV1+' '+inV2+' -'+outV+' 0\n')
    cnfLines.append('-'+inV1+' '+outV+' 0\n')
    cnfLines.append('-'+inV2+' '+outV+' 0\n')
    return cnfLines

def transXOR( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append('-'+inV1+' -'+inV2+' -'+outV+' 0\n')
    cnfLines.append(inV1+' '+inV2+' -'+outV+' 0\n')
    cnfLines.append(inV1+' -'+inV2+' '+outV+' 0\n')
    cnfLines.append('-'+inV1+' '+inV2+' '+outV+' 0\n')
    return cnfLines

def transNOR( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV1+' '+inV2+' '+outV+' 0\n')
    cnfLines.append('-'+inV1+' -'+outV+' 0\n')
    cnfLines.append('-'+inV2+' -'+outV+' 0\n')
    return cnfLines

def transNAND( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append('-'+inV1+' -'+inV2+' -'+outV+' 0\n')
    cnfLines.append(inV1+' '+outV+' 0\n')
    cnfLines.append(inV2+' '+outV+' 0\n')
    return cnfLines

def transBUF( lineIn, lineOut ):
    inV = str(lineIn[0])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV+' -'+outV+' 0\n')
    cnfLines.append('-'+inV+' '+outV+' 0\n')
    return cnfLines

def transZERO( lineIn, lineOut ):
    cnfLines = []
    cnfLines.append('-'+str(lineOut)+' 0\n')
    return cnfLines

def transONE( lineIn, lineOut ):
    cnfLines = []
    cnfLines.append(str(lineOut)+' 0\n')
    return cnfLines
'''


def transINV( lineIn, lineOut ):
    inV = str(lineIn[0])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV+' '+outV+' 0\n')
    cnfLines.append('-'+inV+' -'+outV+' 0\n')
    return cnfLines

def transAND( lineIn, lineOut ):
    cnfLines = []
    outV = str(lineOut)
    firstLine = ''
    for inV in lineIn:
        inVstr = str(inV)
        firstLine += '-'+inVstr+' '
        cnfLines.append(inVstr+' -'+outV+' 0\n')
    firstLine += outV+' 0\n'
    cnfLines.insert(0, firstLine)
    return cnfLines

def transOR( lineIn, lineOut ):
    cnfLines = []
    outV = str(lineOut)
    firstLine = ''
    for inV in lineIn:
        inVstr = str(inV)
        firstLine += inVstr+' '
        cnfLines.append('-'+inVstr+' '+outV+' 0\n')
    firstLine += '-'+outV+' 0\n'
    cnfLines.insert(0, firstLine)
    return cnfLines

def transXOR( lineIn, lineOut ):
    inV1 = str(lineIn[0])
    inV2 = str(lineIn[1])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append('-'+inV1+' -'+inV2+' -'+outV+' 0\n')
    cnfLines.append(inV1+' '+inV2+' -'+outV+' 0\n')
    cnfLines.append(inV1+' -'+inV2+' '+outV+' 0\n')
    cnfLines.append('-'+inV1+' '+inV2+' '+outV+' 0\n')
    return cnfLines

def transNOR( lineIn, lineOut ):
    cnfLines = []
    outV = str(lineOut)
    firstLine = ''
    for inV in lineIn:
        inVstr = str(inV)
        firstLine += inVstr+' '
        cnfLines.append('-'+inVstr+' -'+outV+' 0\n')
    firstLine += outV+' 0\n'
    cnfLines.insert(0, firstLine)
    return cnfLines

def transNAND( lineIn, lineOut ):
    cnfLines = []
    outV = str(lineOut)
    firstLine = ''
    for inV in lineIn:
        inVstr = str(inV)
        firstLine += '-'+inVstr+' '
        cnfLines.append(inVstr+' '+outV+' 0\n')
    firstLine += '-'+outV+' 0\n'
    cnfLines.insert(0, firstLine)
    return cnfLines

def transBUF( lineIn, lineOut ):
    inV = str(lineIn[0])
    outV = str(lineOut)
    cnfLines = []
    cnfLines.append(inV+' -'+outV+' 0\n')
    cnfLines.append('-'+inV+' '+outV+' 0\n')
    return cnfLines

def transZERO( lineIn, lineOut ):
    cnfLines = []
    cnfLines.append('-'+str(lineOut)+' 0\n')
    return cnfLines

def transONE( lineIn, lineOut ):
    cnfLines = []
    cnfLines.append(str(lineOut)+' 0\n')
    return cnfLines

#################################################
#################################################
#################################################
#################################################


def oracV2cnf(OracInV, piNum):
    '''Convert oracle input Verilog file to cnf file.'''
    '''The number of variables and the number of clauses are increased to allow assignments to PI vector.'''

    cnfLines = []
    gateTypeDict = {'and':1, 'or':2, 'xor':3, 'inv':4, 'buf':5, 'nand':6, 'nor':7, 'one':8, 'zero':9}
    options = {1:transAND, 2:transOR, 3:transXOR, 4:transINV, 5:transBUF, 6:transNAND, 7:transNOR, 8:transONE, 9:transZERO}

    with open(OracInV, 'r') as infile:
        inV = infile.read()
        Vlines = inV.split(';\n')

    #1.1 Convert the original circuit to CNF format:
    inputs = [] # stores 2 sets of inputs: PIs and p-bits
    varIndexDict = {}
    intVarDict = {}
    varIndex = 1
    cnFile = []
    #pisIndex = [] #store integer indexes of pis to return
    #piVars = []
    posIndex = []	#store integer indexes of pos to return
    poVars = []
    gateCnt = 0
    # order of integers: PI vars, programming bit vars, PO vars, internal wire vars;
    for line in Vlines:
        line = line.replace('\n', '')
        if 'input' in line and not '//' in line:
            #print line
            PIs=re.search(r'(?<=input )(.*)(?=$)', line).group().replace(' ','').split(',')
            tmpPis = []
            for pi in PIs:
                pi = pi.replace('\\','').replace('[','').replace(']','')
                varIndexDict[pi] = varIndex
                #intVarDict[varIndex] = pi
                #pisIndex.append(varIndex)
                tmpPis.append(varIndex)
                varIndex += 1
            inputs.append(tmpPis)
        elif 'output' in line and not '//' in line:
            POs=re.search(r'(?<=output )(.*)(?=$)', line).group().replace(' ','').split(',')
            for po in POs:
                po = po.replace('\\','').replace('[','').replace(']','')
                varIndexDict[po] = varIndex
                #intVarDict[varIndex] = po
                posIndex.append(varIndex)
                #poVars.append(po)
                varIndex += 1
        elif 'wire' in line and not '//' in line:
            wires=re.search(r'(?<=wire )(.*)(?=$)', line).group().replace(' ','').split(',')
            for w in wires:
                varIndexDict[w] = varIndex
                #intVarDict[varIndex] = w
                varIndex += 1
        elif line!='' and line[0]!='/' and not 'module' in line:
            line = line.replace(' ','')
            if '.' in line and '(' in line: #means it is a mapped Verilog
                gate = re.search(r'^(.*)(?=g\S+\(\.)', line).group().strip('1234567890')
            else: #means it is a behavioral Verilog
                gate = gateType(line)
            #convert vars to standard form:
            buf_split = [term.replace('\\','').replace('[','').replace(']','') for term in line.split('.')]
            buf_processed = []
            for i in range(1,len(buf_split)):
                buf_processed.append(re.search(r'\((.*)\)', buf_split[i]).group().strip('( )'))
            #convert standard vars to integer indexes in cnf file:
            lineOut = varIndexDict[buf_processed[-1]]
            lineIn = []
            for i in range(len(buf_processed)-1):
                lineIn.append(varIndexDict[buf_processed[i]])
            #convert logic gate to CNF format:
            caseNo = gateTypeDict[gate]
            cnfLines = options[caseNo](lineIn, lineOut)
            for line in cnfLines:
                cnFile.append(line)
            gateCnt += 1

    timeGend = time.asctime( time.localtime(time.time()) )
    varNum = varIndex-1
    clauseNum = len(cnFile)
    cmmtLine1 = 'c This file is generated by oracV2cnf\n'
    cmmtLine2 = 'c Generated on '+str(timeGend)+'\n'
    clauseNum += piNum #for later expansion with PI assignments
    firstLine = 'p cnf '+str(varNum)+' '+str(clauseNum)+'\n'
    cnFile.insert(0, firstLine)
    cnFile.insert(0, cmmtLine2)
    cnFile.insert(0, cmmtLine1)

    outxt = ''.join(cnFile)
    with open("Orac_cnf", 'w') as outfile:
         outfile.write(outxt)

    return (inputs[0], posIndex, cnFile)

#################################################
#################################################
#################################################
#################################################



def v2cnfMtr(camInV, MuxStyle):
    '''Converts plain camouflaged verilog file to a cnf miter. Variable number = 2*VarInV + SingleCktOutBitsNum + 1'''

    cnfLines = []
    gateTypeDict = {'and':1, 'or':2, 'xor':3, 'inv':4, 'buf':5, 'nand':6, 'nor':7, 'one':8, 'zero':9}
    options = {1:transAND, 2:transOR, 3:transXOR, 4:transINV, 5:transBUF, 6:transNAND, 7:transNOR, 8:transONE, 9:transZERO}

    with open(camInV, 'r') as infile:
        inV = infile.read()
        Vlines = inV.replace('\r','').split(';\n')

    #1.1 Convert the original circuit to CNF format:
    has_CONST1 = False
    has_CONST0 = False
    inputs = [] # stores 2 sets of inputs: PIs and p-bits
    varIndexDict = {}
    intVarDict = {}
    varIndex = 1
    cnFile = []
    #pisIndex = [] #store integer indexes of pis to return
    #piVars = []
    posIndex = []	#store integer indexes of pos to return
    poVars = []
    Allow = []
    gateCnt = 0
    # order of integers: PI vars, programming bit vars, PO vars, internal wire vars;
    for line in Vlines:
        line = line.replace('\n', '')
        if 'input' in line:
            if "RE__" in line:    # here, I need to assign forbidden bits!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                line = line[:line.find('/')]
            else:
                line = line.replace('\n','')
            if "CONST1" in line:    has_CONST1 = True
            if "CONST0" in line:    has_CONST0 = True
            line = line[:line.find('/')]
            PIs=re.search(r'(?<=input )(.*)(?=$)', line).group().replace(' ','').replace(';', '').split(',')
            tmpPis = []
            for pi in PIs:
                pi = pi.replace('\\','').replace('[','').replace(']','')
                varIndexDict[pi] = varIndex
                #intVarDict[varIndex] = pi
                #pisIndex.append(varIndex)
                tmpPis.append(varIndex)
                varIndex += 1
            inputs.append(tmpPis)
        elif 'output' in line:
            POs=re.search(r'(?<=output )(.*)(?=$)', line).group().replace(' ','').split(',')
            for po in POs:
                po = po.replace('\\','').replace('[','').replace(']','')
                varIndexDict[po] = varIndex
                #intVarDict[varIndex] = po
                posIndex.append(varIndex)
                #poVars.append(po)
                varIndex += 1
        elif 'wire' in line:
            wires=re.search(r'(?<=wire )(.*)(?=$)', line).group().replace(' ','').split(',')
            for w in wires:
                varIndexDict[w] = varIndex
                #intVarDict[varIndex] = w
                varIndex += 1
        elif line!='' and line[0]!='/' and not 'module'  in line:
            #print line
            line = line.replace(' ','')
            gate = re.search(r'^(.*)(?=g\S+\(\.)', line).group().strip('1234567890')
            #convert vars to standard form:
            buf_split = [term.replace('\\','').replace('[','').replace(']','') for term in line.split('.')]
            buf_processed = []
            for i in range(1,len(buf_split)):
                buf_processed.append(re.search(r'\((.*)\)', buf_split[i]).group().strip('( )'))
            #convert standard vars to integer indexes in cnf file:
            lineOut = varIndexDict[buf_processed[-1]]
            lineIn = []
            for i in range(len(buf_processed)-1):
                lineIn.append(varIndexDict[buf_processed[i]])
            #convert logic gate to CNF format:
            caseNo = gateTypeDict[gate]
            cnfLines = options[caseNo](lineIn, lineOut)
            for line in cnfLines:
                cnFile.append(line)
            gateCnt += 1
    camVarNum = varIndex-1 #total number of nodes in original ckt
    camCNFile = cnFile[:]

    #1.2 Duplicate the original circuit with different node notations:
    cnFile.append('c The second circuit:\n')
    cnFile2 = []
    for i in xrange(len(cnFile)-1):
        tmpClause = cnFile[i]
        intIndexLs = [tmpInt for tmpInt in tmpClause.split() if tmpInt!=''][:-1]
        newTmpCls = ''
        for j in range(len(intIndexLs)):
            tmpInt = intIndexLs[j]
            if '-' in tmpInt:
                newTmpCls+='-'+str(int(tmpInt.strip('-'))+camVarNum)+' '
            else:
                newTmpCls+=str(int(tmpInt)+camVarNum)+' '
        cnFile2.append(newTmpCls+'0\n')
    #dupCamCktCnf = cnFile+cnFile2
    cnFile = cnFile+cnFile2
    #print cnFile

    # 2. Add constraints:
    #2.0 if there are CONST1 or CONST0, then constrain it to 0 or 1
    if has_CONST1 == True:
        cnFile.append(str(varIndexDict["CONST1"]) + " 0\n")
    if has_CONST0 == True:
        cnFile.append("-" + str(varIndexDict["CONST0"]) + " 0\n")
    #2.1 primary inputs are the same:
    cnFile.append('c Force PIs of 2 ckts to be the same:\n')
    piVec = inputs[0]
    for i in piVec:
        piConsLine1 = str(i)+' -'+str(i+camVarNum)+' 0\n'
        cnFile.append(piConsLine1)
        piConsLine2 = '-'+str(i)+' '+str(i+camVarNum)+' 0\n'
        cnFile.append(piConsLine2)
    #2.2 rule out illegal bits
    for i in range(len(MuxStyle)):
        if MuxStyle[i] is not 'NULL':
            CBforbit1 = CBconstrain(MuxStyle[i], inputs[i+1])
            for i in CBforbit1:
                cnFile.append(i)
            print "Here is the forbidden bits for original circuit: ", CBforbit1
            intIndexLs = []
            CBforbit2 = []
            for i in range(len(CBforbit1)):
                tmpInt = CBforbit1[i]
                if tmpInt!='':
                    intIndexLs.append(tmpInt.split()[:-1])
            for j in range(len(intIndexLs)):
                newTmpCls = ''
                tmpInt = intIndexLs[j]
                for z in tmpInt:
                    if '-' in z:
                        newTmpCls+='-'+str(int(z.strip('-'))+camVarNum)+' '
                    else:
                        newTmpCls+=str(int(z)+camVarNum)+' '
                newTmpCls +="0\n"
                CBforbit2.append(newTmpCls)
            print "Here is the forbidden bits for second duplication", CBforbit2
            for i in CBforbit2:
                cnFile.append(i)



#	if MuxStyle == 2:
#			#2.2 rule out  illegal assignments (11s):
#		cnFile.append('c add constraints for programming bits\n')
#		pbitSttInt = len(piVec)+1
#		pbitEndInt = pbitSttInt+len(inputs[1])
#		for i in range(pbitSttInt, pbitEndInt, 2):
#			pbitsConsLine = '-'+str(i)+' -'+str(i+1)+' 0\n'
#			cnFile.append(pbitsConsLine)
#		for i in range(pbitSttInt+camVarNum, pbitEndInt+camVarNum, 2):
#			pbitsConsLine = '-'+str(i)+' -'+str(i+1)+' 0\n'
#			cnFile.append(pbitsConsLine)


    #2.3 XOR outputs:
    cnFile.append('c XOR outputs of 2 ckts:\n')
    xorInt = camVarNum*2
    for po in posIndex:
        xorInt += 1
        sig1 = po
        sig2 = po + camVarNum
        poConsLine1='-'+str(sig1)+' -'+str(sig2)+' -'+str(xorInt)+' 0\n'
        cnFile.append(poConsLine1)
        poConsLine2=str(sig1)+' '+str(sig2)+' -'+str(xorInt)+' 0\n'
        cnFile.append(poConsLine2)
        poConsLine3=str(sig1)+' -'+str(sig2)+' '+str(xorInt)+' 0\n'
        cnFile.append(poConsLine3)
        poConsLine4='-'+str(sig1)+' '+str(sig2)+' '+str(xorInt)+' 0\n'
        cnFile.append(poConsLine4)

    #2.4 the last OR:
    cnFile.append('c The last OR gate of the miter:\n')
    #numOfXor = xorNum-1
    orIndex = xorInt+1
    orLine = ''
    for xorInt in range(camVarNum*2+1, orIndex):
        orLine += str(xorInt)+' '
    orLine += '-'+str(orIndex)+' 0\n'
    cnFile.append(orLine)
    orLine1 = ''
    for xorInt in range(camVarNum*2+1, orIndex):
        orLine1 = '-'+str(xorInt)+' '+str(orIndex)+' 0\n'
        cnFile.append(orLine1)
    cnFile.append(str(orIndex)+' 0\n')

    timeGend = time.asctime( time.localtime(time.time()) )
    varNum = orIndex
    clauseNum = len(cnFile)-4 # 5 comments (besides first 3)
    cmmtLine1 = 'c This file is generated by v2cnfMtr\n'
    cmmtLine2 = 'c Generated on '+str(timeGend)+'\n'
    firstLine = 'p cnf '+str(varNum)+' '+str(clauseNum)+'\n'
    cnFile.insert(0, firstLine)
    cnFile.insert(0, cmmtLine2)
    cnFile.insert(0, cmmtLine1)

    # cnFile: the complete miter w/o PI, PO and CB assignments;
    # inputs: [ [PIvec integer indexes], [CBvec integer indexes] ]
    # posIndex: [PO integer indexes]
    # camVarNum: total number of nodes in the original cam ckt
    # camCNFile: the CNF line list of the original cam ckt
    return (cnFile, inputs, posIndex, camVarNum, camCNFile)




#################################################
#################################################
#################################################
#################################################

#the first oldDupCkt is the baseMtrCnf
def dupCompCkt( camCnf, oldDupCkt, PItemp, POtemp, camVarNum, camPIndex, camCBindex, camPOindex ):
    '''Duplicate the camouflaged ckt and add the duplicated ckts\
    to base miter cnf, then evaluates the pair of new control bits\
    and a PI vector that can distinguish them. '''

    #1. duplicate the camouflaged circuit twice:
    #1.1 get the first int index for duplicated circuits:

        # camCNF is the most basic cam, directly transliation
        # self.newDupCkt is the original cam, directly translation, then it is the last dupCkt
        # Pitemp is current I (from last)
        # POtemp is current O (from last)
        # camVarNum is the number of I+O+CB+W
        # camPIndex is PI's index
        # camCBindex is CB's index
        # camPOindex is PO's index


    prevNodeNum = int(oldDupCkt[2].split()[2])
    dupCkt1 = []
    dupCkt2 = []
    correction1 = prevNodeNum
    correction2 = correction1+camVarNum
    ####1.2 create 2 duplicated circuits
    for i in range(len(camCnf)):
        tmpClause = camCnf[i]
        intIndexLs = [tmpInt for tmpInt in tmpClause.split() if tmpInt!=''][:-1]
        newTmpCls1 = ''
        newTmpCls2 = ''
        for j in range(len(intIndexLs)):
            tmpInt = intIndexLs[j]
            if '-' in tmpInt:
                newTmpCls1+='-'+str(int(tmpInt.strip('-'))+correction1)+' '
                newTmpCls2+='-'+str(int(tmpInt.strip('-'))+correction2)+' '
            else:
                newTmpCls1+=str(int(tmpInt)+correction1)+' '
                newTmpCls2+=str(int(tmpInt)+correction2)+' '
        dupCkt1.append(newTmpCls1+'0\n')
        dupCkt2.append(newTmpCls2+'0\n')
    ####1.3 connect the p-bits of new duplicated ckts with those of the miter:
    firstCBints = camCBindex[:]
    secondCBints = []
    dupCkt1CBints = []
    dupCkt2CBints = []
    for i in range(len(firstCBints)):
        baseCBInt = firstCBints[i]
        secondCBints.append(baseCBInt+camVarNum)
        dupCkt1CBints.append(baseCBInt+correction1)
        dupCkt2CBints.append(baseCBInt+correction2)
    dupCkt1CBset = []
    dupCkt2CBset = []
    for j in range(len(firstCBints)):
        #connect the first set of programming bits:
        line1=str(firstCBints[j])+' -'+str(dupCkt1CBints[j])+' 0\n'
        dupCkt1CBset.append(line1)
        line2='-'+str(firstCBints[j])+' '+str(dupCkt1CBints[j])+' 0\n'
        dupCkt1CBset.append(line2)
        #connect the second set of programming bits:
        line3=str(secondCBints[j])+' -'+str(dupCkt2CBints[j])+' 0\n'
        dupCkt2CBset.append(line3)
        line4='-'+str(secondCBints[j])+' '+str(dupCkt2CBints[j])+' 0\n'
        dupCkt2CBset.append(line4)
    ####1.4 assign new temp PI vectors to new duplicated circuits:
    dupCkt1PI = []
    dupCkt2PI = []
    for m in range(len(camPIndex)):
        basePInt = camPIndex[m]
        if PItemp[m] == '1':
            dupCkt1PI.append(str(basePInt+correction1)+' 0\n')
            dupCkt2PI.append(str(basePInt+correction2)+' 0\n')
        elif PItemp[m] == '0':
            dupCkt1PI.append('-'+str(basePInt+correction1)+' 0\n')
            dupCkt2PI.append('-'+str(basePInt+correction2)+' 0\n')
    ####1.5 assign new temp PO vectors to new duplicated circuits:
    dupCkt1PO = []
    dupCkt2PO = []
    for n in range(len(camPOindex)):
        basePOint = camPOindex[n]
        if POtemp[n] == '1':
            dupCkt1PO.append(str(basePOint+correction1)+' 0\n')
            dupCkt2PO.append(str(basePOint+correction2)+' 0\n')
        elif POtemp[n] == '0':
            dupCkt1PO.append('-'+str(basePOint+correction1)+' 0\n')
            dupCkt2PO.append('-'+str(basePOint+correction2)+' 0\n')
    ####1.6 merge all new cnf lines together:
    totDupCkt1 = dupCkt1 + dupCkt1CBset + dupCkt1PI + dupCkt1PO
    #print totDupCkt1
    totDupCkt2 = dupCkt2 + dupCkt2CBset + dupCkt2PI + dupCkt2PO
    #print totDupCkt2
    # modify the first working line in CNF:
    firstLine = oldDupCkt[2]
    firstLnLs = firstLine.split()
    firstLnLs[2] = str(prevNodeNum+2*camVarNum)
    firstLnLs[3] = str( int(firstLnLs[3])+2*len(totDupCkt1) )
    oldDupCkt[2] = (' ').join(firstLnLs)+'\n'
    # add new lines to old one:
    newDupCkt = oldDupCkt + ['c New ckt:\n'] + totDupCkt1 + ['c New ckt:\n'] + totDupCkt2

    return newDupCkt




'''

if __name__=='__main__':
    v2cnfMtr('/home/umass/Documents/Rearch-w-Prof-H/ReverseEngineeringProj/benchmarks/iscas-c7552-orac-cam-10-simple.v',10)
    #if nonMiter==False: #means this is a 1-output miter problem
    #	cnFile.append(str(varIndexDict[POs[0]])+' 0\n')

'''






















