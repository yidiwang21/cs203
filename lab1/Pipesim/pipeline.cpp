#include "pipeline.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>


Register registerFile[16];

Register::Register(void) {

	dataValue = 0;
	registerNumber = -1;
	registerName = "";

}

Instruction::Instruction(void) {

	type = NOP;
	dest = -1;
	src1 = -1;
	src2 = -1;
	stage = NONE;
}

Instruction::Instruction(std::string newInst) {

	std::string buf; 
    	std::stringstream ss(newInst); 
	std::vector<std::string> tokens;
	
    	while (ss >> buf){
		tokens.push_back(buf);
	}

	if(tokens[0] == "ADD")
		type = ADD;
	else if(tokens[0] == "SUB")
		type = SUB;
	else if(tokens[0] == "MULT")
		type = MULT;
	else if(tokens[0] == "DIV")
		type = DIV;
	else if(tokens[0] == "LW")
		type = LW;
	else if(tokens[0] == "SW")
		type = SW;
	else if(tokens[0] == "BNEZ")
		type = BNEZ;
	else
		type = NOP;

	dest = -1;
	src1 = -1;
	src2 = -1;

	if(tokens.size() > 1) {
		dest = atoi(tokens[1].erase(0,1).c_str());
	}
	if(tokens.size() > 2) {
		src1 = atoi(tokens[2].erase(0,1).c_str());
	}
	if(tokens.size() > 3) {
		src2 = atoi(tokens[3].erase(0,1).c_str());
	}
	// Store and BNEZ has 2 source operands and no destination operand
	if (type == SW || type == BNEZ) {
		src2 = src1;
		src1 = dest;
		dest = -1;
	}

	stage = NONE;
}

Application::Application(void) {

	PC = 0;

}

void Application::loadApplication(std::string fileName) {

	std::string sLine = "";
	Instruction *newInstruction;
	std::ifstream infile;
	infile.open(fileName.c_str(), std::ifstream::in);
	
	if ( !infile ) {
		std::cout << "Failed to open file " << fileName << std::endl;
		return;
	}	

	while (!infile.eof())
	{
		getline(infile, sLine);
		if(sLine.empty())
			break;
		newInstruction = new Instruction(sLine);
		instructions.push_back(newInstruction);
	}

	infile.close();
	std::cout << "Read file completed!!" << std::endl;
	
	printApplication();

}

void Application::printApplication(void) {

	std::cout << "Printing Application: " << std::endl;
	std::vector<Instruction*>::iterator it;
	for(it=instructions.begin(); it < instructions.end(); it++) {
	
		(*it)->printInstruction();
		std::cout << std::endl;
	}

}

Instruction* Application::getNextInstruction() {

	Instruction *nextInst = NULL;

	if( PC < instructions.size() ){
		nextInst = instructions[PC];
		PC += 1;
	}
	
	if( nextInst == NULL )
		nextInst = new Instruction();
	
	return nextInst;
}

PipelineStage::PipelineStage(void) {
	inst = new Instruction();
	stageType = NONE;	
}

void PipelineStage::clear() {
	
	inst = NULL;

}

void PipelineStage::addInstruction(Instruction *newInst) {

	inst = newInst;
	inst->stage = stageType;
}

Pipeline::Pipeline(Application *app) {

	pipeline[FETCH].stageType = FETCH;
	pipeline[DECODE].stageType = DECODE;
	pipeline[EXEC].stageType = EXEC;
	pipeline[MEM].stageType = MEM;
	pipeline[WB].stageType = WB;
	cycleTime = 0;

	printPipeline();

	application = app;

	forwarding = false;

}

bool Pipeline::hasDependency(void) {

	if(pipeline[DECODE].inst->type == NOP)
		return false;

	// Checks if dependency exist between Decode stage and Exec, Mem, WB stage
	for(int i = EXEC; i <= WB; i++) {

		if( pipeline[i].inst == NULL )
			continue;		

		if( pipeline[i].inst->type == NOP )
			continue;

		if( (pipeline[i].inst->dest != -1) && 
		    (pipeline[i].inst->dest == pipeline[DECODE].inst->src1 ||		// RAW
		     pipeline[i].inst->dest == pipeline[DECODE].inst->src2) ) {
			/* FORWARDING CONDITIONS START HERE */
			if (width == 0) 
			{
				std::cout<<"333333333333"<<std::endl;
				return true;
			}
			else if (width == 1) {	// EXEC/MEM -> EXEC
				if (i > MEM) {
					std::cout<<"44444444444"<<std::endl;
					return true;
				}
				else{
					std::cout<<"1111111111"<<std::endl;
					return false;
				}
			}
			else if (width == 2) {
				std::cout<<"2222222222222"<<std::endl;
				return false;
			}
			/* FORWARDING CONDITIONS END HERE */
			else return true;	
		}

		if (width == 1) {
			
		}

	}

	return false;

}

void Pipeline::cycle(void) {

	cycleTime += 1;


	// Writeback
	pipeline[WB].clear();

	// Mem -> WB
	pipeline[WB].addInstruction(pipeline[MEM].inst);	

	// Mem
	pipeline[MEM].clear();
	
	// Exec -> Mem
	pipeline[MEM].addInstruction(pipeline[EXEC].inst);	
	
	// Exec
	pipeline[EXEC].clear();

	// Check for data hazards
	if(hasDependency()){
		// If dependency detected, stall by inserting NOP instruction
		pipeline[EXEC].addInstruction(new Instruction());
		return;
	}
	
	// Decode -> Exec
	pipeline[EXEC].addInstruction(pipeline[DECODE].inst);	
	
	// Decode 
	pipeline[DECODE].clear();
	
	// Fetch -> Decode
	pipeline[DECODE].addInstruction(pipeline[FETCH].inst);	
	
	// Fetch
	pipeline[FETCH].clear();
	pipeline[FETCH].addInstruction(application->getNextInstruction());
}

bool Pipeline::done() {

	for(int i = 0; i < 5; i++) {

		if(pipeline[i].inst->type != NOP)
			return false;

	}


	return true;

}

void Pipeline::printPipeline(void) {

	if(cycleTime == 0)
		std::cout << "Cycle" << "\tIF" << "\t\tID" << "\t\tEXEC" << "\t\tMEM" << "\t\tWB" << std::endl;
	std:: cout << cycleTime; 
	for(int i = 0; i < 5; i++) {
		
		pipeline[i].printStage();

	}
	std::cout << std::endl;
}

void PipelineStage::printStage(void) {

	std::cout << "\t";
	inst->printInstruction();

}

void Instruction::printInstruction(void) {
	if(type == NOP)
		std::cout << instructionNames[type] << "         ";
	else if(type == SW || type == BNEZ)
		std::cout << instructionNames[type] << " r" << src1 << " r" << src2;
	else if(type == LW)
		std::cout << instructionNames[type] << " r" << dest << " r" << src1;
	else 
		std::cout << instructionNames[type] << " r" << dest << " r" << src1 << " r" << src2;
}
