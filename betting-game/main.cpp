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
double  migrationRate               = 0.0;
int     populationSize              = 16000;
int     numGroups                   = 1;
int     groupSize                   = populationSize / numGroups;
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
    void inherit(bettingAgent *a, double mr, int generation);
    void calcFitness();
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

void bettingAgent::inherit(bettingAgent *a, double mr, int generation)
{
    born = generation;
    a->nrPointingAtMe += 1;
    ancestor = a;
    probability = a->probability;
    
    if (randDouble < mr)
    {
        do
        {
            probability = randDouble;
            
        } while (probability == 0.0);
    }
}

void bettingAgent::calcFitness()
{
    if (randDouble <= probability)
    {
        fitness = 1.0 / probability;
    }
    else
    {
        fitness = 0.0000000001;
    }
}




int main(int argc, char *argv[])
{
	vector< vector<bettingAgent*> > bettingAgentPops, BANextGenPops;
	vector<bettingAgent*> bestBettingAgents;
	vector<double> bettingAgentMaxFitness, bettingAgentAvgFitness;
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
        
        // -n [int]: set number of groups in GA
        else if (strcmp(argv[i], "-n") == 0 && (i + 1) < argc)
        {
            ++i;
            
            numGroups = atoi(argv[i]);
            
            if (numGroups < 1)
            {
                cerr << "minimum number of groups permitted is 1." << endl;
                exit(0);
            }
        }
        
        // -m [double]: set GA inter-group migration rate
        else if (strcmp(argv[i], "-m") == 0 && (i + 1) < argc)
        {
            ++i;
            
            migrationRate = atof(argv[i]);
            
            if (migrationRate < 0.0)
            {
                cerr << "minimum migration rate permitted is 0.0." << endl;
                exit(0);
            }
            else if (migrationRate > 1.0)
            {
                cerr << "maximum migration rate permitted is 1.0." << endl;
                exit(0);
            }
        }
    }
    
    // determine group size
    if (populationSize % numGroups != 0)
    {
        cerr << "Population size must be evenly divisible by number of groups." << endl;
        exit(0);
    }
    
    groupSize = populationSize / numGroups;
    
    // initial population setup
    bettingAgentAvgFitness.resize(numGroups);
    bettingAgentMaxFitness.resize(numGroups);
    bestBettingAgents.resize(numGroups);
    bettingAgentPops.resize(numGroups);
    BANextGenPops.resize(numGroups);
    
    for (int i = 0; i < numGroups; ++i)
    {
        bettingAgentPops[i].resize(groupSize);
        BANextGenPops[i].resize(groupSize);
    }
    
    // seed the agents
	for(int i = 0; i < numGroups; ++i)
    {
        for (int j = 0; j < groupSize; ++j)
        {
            bettingAgentPops[i][j] = new bettingAgent;
            
            do
            {
                bettingAgentPops[i][j]->probability = randDouble;
                
            } while (bettingAgentPops[i][j]->probability == 0.0);
            
            bettingAgentPops[i][j]->born = 1;
        }
    }
    
	cout << "setup complete" << endl;
    cout << "starting evolution" << endl;
    
    // main loop
	for (int update = 1; update <= totalGenerations; ++update)
    {
        // perform fitness evaluation, selection, and reproduction separately for each group
        for (int groupNum = 0; groupNum < numGroups; ++groupNum)
        {
            // determine fitness of population
            bettingAgentMaxFitness[groupNum] = 0.0;
            bettingAgentAvgFitness[groupNum] = 0.0;
            
            for(int i = 0; i < groupSize; ++i)
            {
                bettingAgentPops[groupNum][i]->calcFitness();
                
                bettingAgentAvgFitness[groupNum] += bettingAgentPops[groupNum][i]->fitness;
                
                if(bettingAgentPops[groupNum][i]->fitness > bettingAgentMaxFitness[groupNum])
                {
                    bettingAgentMaxFitness[groupNum] = bettingAgentPops[groupNum][i]->fitness;
                    bestBettingAgents[groupNum] = bettingAgentPops[groupNum][i];
                }
            }
            
            bettingAgentAvgFitness[groupNum] /= (double)groupSize;
            
            for(int i = 0; i < groupSize; ++i)
            {
                // construct new agent population for the next generation
                bettingAgent *offspring = new bettingAgent;
                int j = 0;
                
                do
                {
                    j = rand() % groupSize;
                } while((j == i) || (randDouble > (bettingAgentPops[groupNum][j]->fitness / bettingAgentMaxFitness[groupNum])));
                
                offspring->inherit(bettingAgentPops[groupNum][j], mutationRate, update);
                BANextGenPops[groupNum][i] = offspring;
            }
            
            for(int i = 0; i < groupSize; ++i)
            {
                // replace the agents from the previous generation
                if(bettingAgentPops[groupNum][i]->nrPointingAtMe == 0)
                {
                    delete bettingAgentPops[groupNum][i];
                }
                bettingAgentPops[groupNum][i] = BANextGenPops[groupNum][i];
            }
            
            bettingAgentPops[groupNum] = BANextGenPops[groupNum];
        }
        
        // perform migration between groups
        for (int groupNum = 0; groupNum < numGroups; ++groupNum)
        {
            for(int i = 0; i < groupSize; ++i)
            {
                // if agent migrates,
                if (numGroups > 1 && randDouble <= migrationRate)
                {
                    bettingAgent *clone = new bettingAgent;
                    clone->inherit(bettingAgentPops[groupNum][i], 0.0, update);
                    
                    int newGroupNum = groupNum;
                    
                    while (newGroupNum == groupNum)
                    {
                        newGroupNum = rand() % numGroups;
                    }
                    
                    int newGroupPosition = rand() % groupSize;
                    
                    if (bettingAgentPops[newGroupNum][newGroupPosition]->nrPointingAtMe == 0)
                    {
                        delete bettingAgentPops[newGroupNum][newGroupPosition];
                        bettingAgentPops[newGroupNum][newGroupPosition] = NULL;
                    }
                    
                    bettingAgentPops[newGroupNum][newGroupPosition] = clone;
                }
            }
        }
        
        if (update == 1 || update % 1000 == 0)
        {
            cout << "generation " << update << ": betting agent [" << bettingAgentAvgFitness[0] << " : " << bettingAgentMaxFitness[0] << "] " << bestBettingAgents[0]->probability << endl;
        }
	}
    
    // calculate the final betting agent's fitness
    bettingAgentPops[0][0]->calcFitness();
	
    // save best agent's LOD
    vector<bettingAgent*> saveLOD;
    
    cout << "building ancestor list" << endl;
    
    bettingAgent* curAncestor = bettingAgentPops[0][0];
    
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
