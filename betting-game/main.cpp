/*
 * main.cpp
 *
 * This file is part of the Betting Game project.
 *
 * Copyright 2013 Randal S. Olson.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <string.h>
#include <vector>
#include <map>
#include <math.h>
#include <time.h>
#include <iostream>
#include <dirent.h>

#define     randDouble          ((double)rand() / (double)RAND_MAX)

//double  replacementRate             = 0.1;
double  mutationRate                = 0.1;
int     populationSize              = 16000;
int     totalGenerations            = 10000;

using namespace std;

class bettingAgent
{
public:
    double probability, fitness;
    int nrPointingAtMe, born;
    bettingAgent *ancestor;
    
    bettingAgent();
    ~bettingAgent();
    void inherit(bettingAgent *a, int generation);
};

bettingAgent::bettingAgent()
{
    probability = 0.0;
    fitness = 0.0;
    nrPointingAtMe = 0;
    born = 0;
    ancestor = NULL;
}

bettingAgent::~bettingAgent()
{
    if (ancestor != NULL)
    {
        ancestor->nrPointingAtMe -= 1;
        
        if (ancestor->nrPointingAtMe == 0)
        {
            delete ancestor;
        }
        
        ancestor = NULL;
    }
}

void bettingAgent::inherit(bettingAgent *a, int generation)
{
    born = generation;
    a->nrPointingAtMe += 1;
    ancestor = a;
    probability = a->probability;
    
    if (randDouble < mutationRate)
    {
        do
        {
            probability = randDouble;
            
        } while (probability == 0.0);
    }

}

int main(int argc, char *argv[])
{
	vector<bettingAgent*> bettingAgents, BANextGen;
	bettingAgent* bestBettingAgent = NULL;
	double bettingAgentMaxFitness = 0.0;
    string LODFileName = "";
    
    // time-based seed by default. can change with command-line parameter.
    srand((unsigned int)time(NULL));
    
    for (int i = 1; i < argc; ++i)
    {        
        // -e [LOD file name]: evolve
        if (strcmp(argv[i], "-e") == 0 && (i + 1) < argc)
        {
            ++i;
            stringstream lodfn;
            lodfn << argv[i];
            LODFileName = lodfn.str();
        }
        
        // -s [int]: set seed
        else if (strcmp(argv[i], "-s") == 0 && (i + 1) < argc)
        {
            ++i;
            srand(atoi(argv[i]));
        }
        
        // -g [int]: set generations
        else if (strcmp(argv[i], "-g") == 0 && (i + 1) < argc)
        {
            ++i;
            
            if (totalGenerations < 1)
            {
                cerr << "minimum number of generations permitted is 1." << endl;
                exit(0);
            }
        }
        
        // -p [int]: set GA population size
        else if (strcmp(argv[i], "-p") == 0 && (i + 1) < argc)
        {
            ++i;
            
            populationSize = atoi(argv[i]);
            
            if (populationSize < 1)
            {
                cerr << "minimum GA population size permitted is 1." << endl;
                exit(0);
            }
        }
    }
    
    // initial population setup
    bettingAgents.resize(populationSize);
    BANextGen.resize(populationSize);
    
    // seed the agents
	for(int i = 0; i < populationSize; ++i)
    {
		bettingAgents[i] = new bettingAgent;
        
        do
        {
            bettingAgents[i]->probability = randDouble;
            
        } while (bettingAgents[i]->probability == 0.0);
        
        bettingAgents[i]->born = 1;
    }
    
	cout << "setup complete" << endl;
    cout << "starting evolution" << endl;
    
    // main loop
	for (int update = 2; update <= totalGenerations; ++update)
    {
        // determine fitness of population
		bettingAgentMaxFitness = 0.0;
        double bettingAgentAvgFitness = 0.0;
        
		for(int i = 0; i < populationSize; ++i)
        {
            if (randDouble <= bettingAgents[i]->probability)
            {
                bettingAgents[i]->fitness = 1.0 / bettingAgents[i]->probability;
            }
            else
            {
                bettingAgents[i]->fitness = 0.0000000001;
            }
            
            bettingAgentAvgFitness += bettingAgents[i]->fitness;
            
            if(bettingAgents[i]->fitness > bettingAgentMaxFitness)
            {
                bettingAgentMaxFitness = bettingAgents[i]->fitness;
                bestBettingAgent = bettingAgents[i];
            }
		}
        
        bettingAgentAvgFitness /= (double)populationSize;
		
        if (update == 1 || update % 1000 == 0)
        {
            cout << "generation " << update << ": betting agent [" << bettingAgentAvgFitness << " : " << bettingAgentMaxFitness << "] " << bestBettingAgent->probability << endl;
        }
        
        for(int i = 0; i < populationSize; ++i)
        {
            // construct agent population for the next generation
            bettingAgent *offspring = new bettingAgent;
            int j = 0;
            
            do
            {
                j = rand() % populationSize;
            } while((j == i) || (randDouble > (bettingAgents[j]->fitness / bettingAgentMaxFitness)));
            
            offspring->inherit(bettingAgents[j], update);
            BANextGen[i] = offspring;
        }
        
        for(int i = 0; i < populationSize; ++i)
        {
            // replace the agents from the previous generation
            if(bettingAgents[i]->nrPointingAtMe == 0)
            {
                delete bettingAgents[i];
            }
            bettingAgents[i] = BANextGen[i];
        }
        
        bettingAgents = BANextGen;
	}
	
    // save best agent's LOD
    vector<bettingAgent*> saveLOD;
    
    cout << "building ancestor list" << endl;
    
    bettingAgent* curAncestor = bettingAgents[0];
    
    while (curAncestor != NULL)
    {
        // don't add the base ancestor
        if (curAncestor->ancestor != NULL)
        {
            saveLOD.insert(saveLOD.begin(), curAncestor);
        }
        
        curAncestor = curAncestor->ancestor;
    }
    
    FILE *LOD = fopen(LODFileName.c_str(), "w");

    fprintf(LOD, "generation,fitness,probability\n");
    
    cout << "analyzing ancestor list" << endl;
    
    for (vector<bettingAgent*>::iterator it = saveLOD.begin(); it != saveLOD.end(); ++it)
    {
        fprintf(LOD, "%d,%f,%f\n", (*it)->born, (*it)->fitness, (*it)->probability);
    }
    
    fclose(LOD);
    
    return 0;
}
