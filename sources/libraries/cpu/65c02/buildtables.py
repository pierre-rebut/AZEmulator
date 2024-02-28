# *******************************************************************************************
# *******************************************************************************************
#
#		File:			buildtables.py
#		Date:			3rd September 2019
#		Purpose:		Creates files tables.h from the .opcodes descriptors
#						Creates disassembly include file.
#		Author:			Paul Robson (paul@robson.org.uk)
#		Formatted By: 	Jeries Abedrabbo (jabedrabbo@asaltech.com)
#
# *******************************************************************************************
# *******************************************************************************************

import re

############# CONSTANTS ##############
######################################
########### KEY CONSTANTS ############
ACTN_KEY_STR = "action"
CYCLES_KEY_STR = "cycles"
MODE_KEY_STR = "mode"
OPCODE_KEY_STR = "opcode"

######################################
########### REGEX CONSTANTS ##########
OPCODE_REGEX_STR = "^(?P<{actnCode}>\\w+)\\s+(?P<{addrMode}>\\w+)\\s+(?P<{mchnCycles}>\\d)\\s+\\$(?P<{opCode}>[0-9a-fA-F]+)$".format(
    actnCode=ACTN_KEY_STR,
    addrMode=MODE_KEY_STR,
    mchnCycles=CYCLES_KEY_STR,
    opCode=OPCODE_KEY_STR
)

#####################################
######### OPCODE CONSTANTS ##########
EMPTY_NOP_ACTN = "NOP"
EMPTY_NOP_CYCLS = "2"
EMPTY_NOP_MOD = "IMP"
TOTAL_NUMBER_OPCODES = 2 ** 8

#####################################
############# FILENAMES #############
TABLES_HEADER_FNAME = "Opcodes.h"
OPCODES_6502_FNAME = "6502.opcodes"


#####################################
#####################################


#######################################################################################################################
####################################  Load in a source file with opcode descriptors  ##################################
#######################################################################################################################
def loadSource(srcFile):
    # Read file line by line and omit committed
    fileLinesObject = open(srcFile).readlines()
    strippedOpcodeLines = [opcodeLine.strip() for opcodeLine in fileLinesObject if
                           opcodeLine.strip() != "" and not opcodeLine.startswith(";")]

    # Import Opcodes
    for opcodeLine in strippedOpcodeLines:
        matchLine = re.match(OPCODE_REGEX_STR, opcodeLine)
        assert matchLine is not None, "Format {}".format(opcodeLine)

        opcode = int(matchLine.group(OPCODE_KEY_STR), 16)
        assert opcodesList[opcode] is None, "Duplicate {0:02x}".format(opcode)

        opcodesList[opcode] = {
            MODE_KEY_STR: matchLine.group(MODE_KEY_STR),
            ACTN_KEY_STR: matchLine.group(ACTN_KEY_STR),
            CYCLES_KEY_STR: matchLine.group(CYCLES_KEY_STR),
            OPCODE_KEY_STR: opcode
        }


#######################################################################################################################
#############################################  Fill unused slots with NOPs  ###########################################
#######################################################################################################################
def fillNop():
    for i in range(0, TOTAL_NUMBER_OPCODES):
        if opcodesList[i] is None:
            opcodesList[i] = {
                MODE_KEY_STR: EMPTY_NOP_MOD,
                ACTN_KEY_STR: EMPTY_NOP_ACTN,
                CYCLES_KEY_STR: EMPTY_NOP_CYCLS,
                OPCODE_KEY_STR: i
            }


#######################################################################################################################
###################################################  Output a table  ##################################################
#######################################################################################################################
def generateTable(hFileName):
    for item in opcodesList:
        hFileName.write("#define op_{0:02x} {{\"{1}\", &Core65c02::{1}, &Core65c02::{2}, {3}}}\n".format(
            item[OPCODE_KEY_STR],
            item[ACTN_KEY_STR],
            item[MODE_KEY_STR],
            item[CYCLES_KEY_STR]
        ))


#######################################################################################################################
##################################################  Load in opcodes  ##################################################
#######################################################################################################################
if __name__ == '__main__':
    opcodesList = [None] * TOTAL_NUMBER_OPCODES
    # Load 6502 opcodes list
    loadSource(OPCODES_6502_FNAME)
    # Fill opcodes list with NOP instructions
    fillNop()

    # Create "TABLES_HEADER_FNAME" header file
    with open(TABLES_HEADER_FNAME, "w") as output_h_file:
        output_h_file.write("/* Generated by buildtables.py */\n\n")
        generateTable(output_h_file)
